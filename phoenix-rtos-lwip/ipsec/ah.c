/*
 * embedded IPsec
 * Copyright (c) 2003 Niklaus Schild and Christian Scheurer, HTI Biel/Bienne
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 */

/** @file ah.c
 *  @brief RFC2402 - IP Authentication Header (AH)
 *
 *  @author  Christian Scheurer <http://www.christianscheurer.ch>
 *
 *  <B>OUTLINE:</B>
 * The AH functions are used to authenticate IPsec traffic.
 *
 *  <B>IMPLEMENTATION:</B>
 * All functions work in-place (i.g. manipulate directly the original
 * packet without copying any data). For the encapsulation routine,
 * the caller must ensure that space for the new IP and AH header are
 * available in front of the packet:
 *
 *  <pre>
 *                                  | pointer to packet header
 *     ____________________________\/_____________________________
 *    |          �       �         �                              |
 *    | Ethernet � newIP � AH, ICV �   original (inner) packet    |
 *    |__________�_______�_________�______________________________|
 *    �                            �
 *    �<-- room for new headers -->�
 *  </pre>
 *
 * This document is part of <EM>embedded IPsec<BR>
 * Copyright (c) 2003 Niklaus Schild and Christian Scheurer, HTI Biel/Bienne<BR>
 * All rights reserved.<BR>
 * This file contains code from the OpenSSL Project<BR>
 * portions Copyright (c) 1998-2003 OpenSSL (www.openssl.org)
 *</EM><HR>
 */

#include "ah.h"

#include "md5.h"
#include "sha1.h"
#include "sha256.h"
#include "debug.h"

#include "lwip/inet_chksum.h"
#include "lwip/prot/ip.h"

#include <string.h>


u32_t ipsec_ah_bitmap = 0;  /**< save session state to detect replays - must be 32 bits. 
											 *   Note: must be initialized with zero (0x00000000) when
											 *         a new SA is established! */
u32_t ipsec_ah_lastSeq = 0; /**< save session state to detect replays
											 *   Note: must be initialized with zero (0x00000000) when
											 *         a new SA is established! */

/**
 * Checks AH header and ICV (RFC 2402).
 * Mutable fields of the outer IP header are set to zero prior to the ICV calculation.
 *
 * @todo Extend function to support transport mode
 *
 * @param	outer_packet   pointer used to access the (outer) IP packet which hast to be checked
 * @param   payload_offset  pointer used to return offset of inner (original) IP packet relative to the start of the outer header
 * @param   payload_size    pointer used to return total size of the inner (original) IP packet
 * @param 	sa              pointer to security association holding the secret authentication key
 *
 * @return IPSEC_STATUS_SUCCESS	        packet could be authenticated
 * @return IPSEC_STATUS_FAILURE         packet is corrupted or ICV does not match
 * @return IPSEC_STATUS_NOT_IMPLEMENTED invalid mode (only IPSEC_TUNNEL mode is implemented)
 */
int ipsec_ah_check(struct ip_hdr *outer_packet, int *payload_offset, int *payload_size,
	sad_entry_t *sa)
{
	int ret_val = IPSEC_STATUS_NOT_INITIALIZED; /* by default, the return value is undefined */
	ipsec_ah_header *ah_header;
	int ah_len;
	int ah_offs;
	unsigned char orig_digest[IPSEC_MAX_AUTHKEY_LEN];
	unsigned char digest[IPSEC_MAX_AUTHKEY_LEN];

	IPSEC_LOG_TRC(IPSEC_TRACE_ENTER, "outer_packet=%p, *payload_offset=%d, *payload_size=%d sa=%p",
		(void *)outer_packet, *payload_offset, *payload_size, (void *)sa);

	/* The AH header is expected to be 24 bytes since we support only 96 bit authentication values */
	ah_offs = ((outer_packet->_v_hl & 0x0F) << 2);
	ah_len = (IPSEC_AH_HDR_SIZE - 4) + (((ipsec_ah_header *)((unsigned char *)outer_packet + ah_offs))->len << 2);

	/* minimal AH header + ICV */
	if (ah_len != IPSEC_AH_HDR_SIZE + IPSEC_AUTH_ICV) {
		IPSEC_LOG_DBG(IPSEC_STATUS_FAILURE, "wrong AH header size: ah_len=%d (must be 24 bytes, only 96bit authentication values allowed)", ah_len);
		IPSEC_LOG_TRC(IPSEC_TRACE_RETURN, "return = %d", IPSEC_STATUS_FAILURE);
		return IPSEC_STATUS_FAILURE;
	}

	ah_header = ((ipsec_ah_header *)((unsigned char *)outer_packet + ah_offs));

	/* preliminary anti-replay check (without updating the global sequence number window)     */
	/* This check prevents useless ICV calculation if the Sequence Number is obviously wrong  */
	ret_val = ipsec_check_replay_window(lwip_ntohl(ah_header->sequence), ipsec_ah_lastSeq, ipsec_ah_bitmap);
	if (ret_val != IPSEC_AUDIT_SUCCESS) {
		IPSEC_LOG_AUD(IPSEC_AUDIT_SEQ_MISMATCH, "packet rejected by anti-replay check (lastSeq=%08x, seq=%08x, window size=%d)", ipsec_ah_lastSeq, lwip_ntohl(ah_header->sequence), IPSEC_SEQ_MAX_WINDOW);
		return ret_val;
	}

	/* zero all mutable fields prior to ICV calculation */
	/* mutuable fields according to RFC2402, 3.3.3.1.1.1. */
	outer_packet->_tos = 0;
	outer_packet->_offset = 0;
	outer_packet->_ttl = 0;
	outer_packet->_chksum = 0;

	/* backup 96bit HMAC before setting it to 0 */
	memcpy(orig_digest, ah_header->ah_data, IPSEC_AUTH_ICV);
	memset(ah_header->ah_data, 0, IPSEC_AUTH_ICV);

	if (sa->mode != IPSEC_MODE_TUNNEL) {
		IPSEC_LOG_ERR(IPSEC_STATUS_NOT_IMPLEMENTED, "Can't handle mode %d. Only mode %d (IPSEC_TUNNEL) is implemented.", sa->mode, IPSEC_MODE_TUNNEL);
		IPSEC_LOG_TRC(IPSEC_TRACE_RETURN, "return = %d", IPSEC_STATUS_NOT_IMPLEMENTED);
		return IPSEC_STATUS_NOT_IMPLEMENTED;
	}

	switch (sa->auth_alg) {

		case SADB_AALG_MD5HMAC:
			hmac_md5((unsigned char *)outer_packet, lwip_ntohs(outer_packet->_len),
				(unsigned char *)sa->authkey, IPSEC_AUTH_MD5_KEY_LEN, (unsigned char *)&digest);
			break;
		case SADB_AALG_SHA1HMAC:
			hmac_sha1((unsigned char *)outer_packet, lwip_ntohs(outer_packet->_len),
				(unsigned char *)sa->authkey, IPSEC_AUTH_SHA1_KEY_LEN, (unsigned char *)&digest);
			break;
		case SADB_X_AALG_SHA2_256:
			hmac_sha256((unsigned char *)outer_packet, lwip_ntohs(outer_packet->_len),
				(unsigned char *)sa->authkey, IPSEC_AUTH_SHA256_KEY_LEN, (unsigned char *)&digest);
			break;
		default:
			IPSEC_LOG_ERR(IPSEC_STATUS_FAILURE, "unknown HASH algorithm for this AH");
			IPSEC_LOG_TRC(IPSEC_TRACE_RETURN, "return = %d", IPSEC_STATUS_FAILURE);
			return IPSEC_STATUS_FAILURE;
	}

	if (memcmp(orig_digest, digest, IPSEC_AUTH_ICV) != 0) {
		IPSEC_LOG_ERR(IPSEC_STATUS_FAILURE, "AH ICV does not match");
		IPSEC_LOG_TRC(IPSEC_TRACE_RETURN, "return = %d", IPSEC_STATUS_FAILURE);
		return IPSEC_STATUS_FAILURE;
	}

	/* post-ICV calculationn anti-replay check (this call will update the global sequence number window) */
	ret_val = ipsec_update_replay_window(lwip_ntohl(ah_header->sequence), (u32_t *)&ipsec_ah_lastSeq, (u32_t *)&ipsec_ah_bitmap);
	if (ret_val != IPSEC_AUDIT_SUCCESS) {
		IPSEC_LOG_AUD(IPSEC_AUDIT_SEQ_MISMATCH, "packet rejected by anti-replay update (lastSeq=%08x, seq=%08x, window size=%d)", ipsec_ah_lastSeq, lwip_ntohl(ah_header->sequence), IPSEC_SEQ_MAX_WINDOW);
		return ret_val;
	}

	*payload_offset = ah_offs + ah_len;
	*payload_size = lwip_ntohs(((struct ip_hdr *)((unsigned char *)outer_packet + ah_offs + ah_len))->_len);

	IPSEC_LOG_TRC(IPSEC_TRACE_RETURN, "return = %d", IPSEC_STATUS_SUCCESS);
	return IPSEC_STATUS_SUCCESS;
}


/**
 * Adds AH and outer IP header, calculates ICV (RFC 2402).
 *
 * @warning Attention: this function requires room (IPSEC_AH_HDR_SIZE + IPSEC_AUTH_ICV + IPSEC_MIN_IPHDR_SIZE)
 *          in front of the inner_packet pointer to add outer IP header and AH header. Depending on the
 *          TCP/IP stack implementation, additional space for the Link layer (Ethernet header) should be added).
 *
 * @todo Extend function to support transport mode
 *
 * @param	inner_packet   pointer used to access the (outer) IP packet which hast to be checked
 * @param   payload_offset  pointer used to return offset of inner (original) IP packet relative to the start of the outer header
 * @param   payload_size    pointer used to return total size of the inner (original) IP packet
 * @param   src             IP address of the local tunnel start point (external IP address)
 * @param   dst             IP address of the remote tunnel end point (external IP address)
 * @param 	sa              pointer to security association holding the secret authentication key
 * @return IPSEC_STATUS_SUCCESS	        packet could be authenticated
 * @return IPSEC_STATUS_FAILURE         packet is corrupted or ICV does not match
 * @return IPSEC_STATUS_NOT_IMPLEMENTED invalid mode (only IPSEC_TUNNEL mode is implemented)
 */
int ipsec_ah_encapsulate(struct ip_hdr *inner_packet, int *payload_offset, int *payload_size,
	sad_entry_t *sa, u32_t src, u32_t dst)
{
	struct ip_hdr *new_ip_header;
	ipsec_ah_header *new_ah_header;
	unsigned char digest[IPSEC_MAX_AUTHKEY_LEN];

	IPSEC_LOG_TRC(IPSEC_TRACE_ENTER,
		"inner_packet=%p, *payload_offset=%d, *payload_size=%d sa=%p, src=%lu, dst=%lu",
		(void *)inner_packet, *payload_offset, *payload_size, (void *)sa, src, dst);

	/* set new packet header pointers */
	new_ip_header = (struct ip_hdr *)(((char *)inner_packet) - IPSEC_AH_HDR_SIZE - IPSEC_AUTH_ICV - IPSEC_MIN_IPHDR_SIZE);
	new_ah_header = (ipsec_ah_header *)(((char *)inner_packet) - IPSEC_AUTH_ICV - IPSEC_AH_HDR_SIZE);

	/* decrement and check TTL */
	/** @todo fix TTL update and checksum calculation */
	// inner_packet->ttl--;
	// inner_packet->chksum = ip_chksum(inner_packet, sizeof(ip_header));
	if (inner_packet->_ttl == 0) {
		IPSEC_LOG_TRC(IPSEC_TRACE_RETURN, "return = %d", IPSEC_STATUS_TTL_EXPIRED);
		return IPSEC_STATUS_TTL_EXPIRED;
	}

	if (IPSEC_AUTH_ICV != 12) {
		IPSEC_LOG_TRC(IPSEC_TRACE_RETURN, "return = %d", IPSEC_STATUS_NOT_IMPLEMENTED);
		return IPSEC_STATUS_NOT_IMPLEMENTED;
	}


	/* increment Sequence Number Field by 1 for each AH packet (1st packet has squ==1) */
	sa->seqnum++;

	/* set header fields */
	new_ah_header->nexthdr = 0x04; /* IP in IP */
	new_ah_header->len = 0x04;     /* length is 4 for AH with 96bit ICV */
	new_ah_header->reserved = 0x0000;
	new_ah_header->spi = sa->spi;
	new_ah_header->sequence = lwip_htonl(sa->seqnum);
	memset(new_ah_header->ah_data, '\0', IPSEC_AUTH_ICV);

	/* setup IP header and zero all mutable fields prior to ICV calculation */
	/* mutable fields according to RFC2402, 3.3.3.1.1.1. */
	new_ip_header->_v_hl = 0x45;
	new_ip_header->_tos = 0;
	new_ip_header->_len = lwip_htons(lwip_ntohs(inner_packet->_len) + IPSEC_AH_HDR_SIZE + IPSEC_AUTH_ICV + IPSEC_MIN_IPHDR_SIZE);
	new_ip_header->_id = 1000; /**@todo id must be generated properly and incremented */
	new_ip_header->_offset = 0;
	new_ip_header->_ttl = 0;
	new_ip_header->_proto = IP_PROTO_AH;
	new_ip_header->_chksum = 0;
	new_ip_header->src.addr = src;
	new_ip_header->dest.addr = dst;

	/* calculate AH according the SA */
	switch (sa->auth_alg) {

		case SADB_AALG_MD5HMAC:
			hmac_md5((unsigned char *)new_ip_header, lwip_ntohs(new_ip_header->_len),
				(unsigned char *)sa->authkey, IPSEC_AUTH_MD5_KEY_LEN, (unsigned char *)&digest);
			break;
		case SADB_AALG_SHA1HMAC:
			hmac_sha1((unsigned char *)new_ip_header, lwip_ntohs(new_ip_header->_len),
				(unsigned char *)sa->authkey, IPSEC_AUTH_SHA1_KEY_LEN, (unsigned char *)&digest);
			break;
		case SADB_X_AALG_SHA2_256:
			hmac_sha256((unsigned char *)new_ip_header, lwip_ntohs(new_ip_header->_len),
				(unsigned char *)sa->authkey, IPSEC_AUTH_SHA256_KEY_LEN, (unsigned char *)&digest);
			break;
		default:
			IPSEC_LOG_ERR(IPSEC_STATUS_FAILURE, "unknown HASH algorithm for this AH");
			IPSEC_LOG_TRC(IPSEC_TRACE_RETURN, "return = %d", IPSEC_STATUS_FAILURE);
			return IPSEC_STATUS_FAILURE;
	}

	/* insert ICV */
	memcpy(new_ah_header->ah_data, digest, IPSEC_AUTH_ICV);

	/* update outer IP header */
	new_ip_header->_tos = inner_packet->_tos;
	new_ip_header->_ttl = 64;

	/* set checksum */
	IPH_CHKSUM_SET(new_ip_header, inet_chksum(new_ip_header, IP_HLEN));

	/* setup return values */
	*payload_size = lwip_ntohs(new_ip_header->_len);
	*payload_offset = (((char *)new_ip_header) - ((char *)inner_packet));

	IPSEC_LOG_TRC(IPSEC_TRACE_RETURN, "return = %d", IPSEC_STATUS_SUCCESS);
	return IPSEC_STATUS_SUCCESS;
}
