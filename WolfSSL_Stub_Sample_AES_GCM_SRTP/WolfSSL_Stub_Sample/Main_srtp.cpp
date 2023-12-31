/* Init libsrtp */

#include <srtp.h>
#include <crypto_kernel.h>
#include <iostream>
#include <crypto\include\cipher_types.h>
#include <crypto\cipher\aes_gcm_ossl.c>

#define KN_MAX_KEY_LEN 128
#define KN_MKI_VAL_SIZE 4

struct str_st
{
	/** Buffer pointer, which is by convention NOT null terminated. */
	char ptr[64];

	/** The length of the string. */
	int  slen;
};

//KN_TP_I_MSG_TYPE enum also needs to be updated on below enum changes.
typedef enum _MSI_I_MESSAGE_TYPE {
	MSI_I_MESSAGE_TYPE_INVALID,
	MSI_I_MESSAGE_TYPE_CSK,
	MSI_I_MESSAGE_TYPE_PCK,
	MSI_I_MESSAGE_TYPE_T_GMK,
	MSI_I_MESSAGE_TYPE_GMK,
	MSI_I_MESSAGE_TYPE_MAX
}MSI_I_MESSAGE_TYPE;

typedef struct srtp_policy_neg
{
	/** Optional key. If empty, a random key will be autogenerated. */
	str_st	key;
	/** Crypto name.   */
	str_st	name;
	/** Flags, bitmask from #pjmedia_srtp_crypto_option */
	unsigned	flags;
	/*Need this as part of ROC changes , to distinguish between Keys*/
	MSI_I_MESSAGE_TYPE eKeyType;
} srtp_policy_neg;


typedef struct srtp_crypto_info
{
	srtp_policy_neg  tx_policy;
	srtp_policy_neg  rx_policy;

	/* Temporary policy for negotiation */
	srtp_policy_neg tx_policy_neg;
	srtp_policy_neg rx_policy_neg;

	unsigned char		tx_key[KN_MAX_KEY_LEN];
	unsigned char		rx_key[KN_MAX_KEY_LEN];
	unsigned char		tx_rx_key[KN_MAX_KEY_LEN]; //Media RTP Master Key Only

	unsigned char	mki_idVal[2 * KN_MKI_VAL_SIZE];
	unsigned int	mki_idSize;
	unsigned char	csk_mki_idVal[KN_MKI_VAL_SIZE];
	unsigned int	csk_mki_idSize;

	/* libSRTP contexts */
	void* srtcp_tx_rx_ctx;	/*RTCP transmission context*/

} srtp_crypto_info;

typedef void(*crypto_method_t)(srtp_crypto_policy_t* policy);

typedef struct crypto_suite
{
	const char* name;
	srtp_cipher_type_id_t cipher_type;
	unsigned		 cipher_key_len;    /* key + salt length    */
	unsigned		 cipher_salt_len;   /* salt only length	    */
	srtp_auth_type_id_t	 auth_type;
	unsigned		 auth_key_len;
	unsigned		 srtp_auth_tag_len;
	unsigned		 srtcp_auth_tag_len;
	srtp_sec_serv_t	 service;
	/* This is an attempt to validate crypto support by libsrtp, i.e: it should
	* raise linking error if the libsrtp does not support the crypto.
	*/
	const srtp_cipher_type_t* ext_cipher_type;
	crypto_method_t      ext_crypto_method;
} crypto_suite;

extern const srtp_cipher_type_t srtp_aes_gcm_128_openssl;

/* https://www.iana.org/assignments/sdp-security-descriptions/sdp-security-descriptions.xhtml */
static crypto_suite crypto_suites[] = {
	/* plain RTP/RTCP (no cipher & no auth) */
	{ "NULL", SRTP_NULL_CIPHER, 0, SRTP_NULL_AUTH, 0, 0, 0, sec_serv_none },
	/* cipher AES_GCM, NULL auth, auth tag len = 16 octets */
	{ "AEAD_AES_128_GCM", SRTP_AES_GCM_128, 28, 12,
	SRTP_NULL_AUTH, 0, 16, 16, sec_serv_conf_and_auth,
	&srtp_aes_gcm_128_openssl },

	/* cipher AES_GCM, NULL auth, auth tag len = 16 octets */
	{ "AEAD_AES_128_GCM", SRTP_AES_GCM_128, 28, 12,
	SRTP_NULL_AUTH, 0, 16, 16, sec_serv_conf_and_auth,
	&srtp_aes_gcm_128_openssl },

	/* cipher AES_GCM, NULL auth, auth tag len = 8 octets */
	/*{ "AEAD_AES_128_GCM_8", SRTP_AES_GCM_128, 28, 12,
	SRTP_NULL_AUTH, 0, 8, 8, sec_serv_conf_and_auth,
	&srtp_aes_gcm_128_openssl },*/
};

int srtp_lib_init();

int srtp_start(srtp_crypto_info* srtp_info, unsigned int client_ssrc);

void print_buffer(const char* name, unsigned char* data, int len);

int main()
{
	//QC-CONNECT
	/*
	unsigned int client_ssrc = 1881160814;
	srtp_crypto_info srtp_cinfo = { 0 };
	srtp_crypto_info* srtp_info = &srtp_cinfo;
	char MasterKey[] = { 0x57, 0xbf, 0x35, 0xcb, 0x71, 0x13, 0x41, 0xa1, 0x18, 0x6a, 0xd6, 0xad, 0xc9, 0xec, 0x66, 0x56, 0xee, 0xb9, 0x83, 0x82, 0x1e, 0x24, 0xfb, 0x81, 0xa9, 0xd8, 0x53, 0x73 };
	char pkt[] = { 0x90, 0xcc, 0x00, 0x57, 0xe7, 0x75, 0x95, 0x38, 0xb4, 0xe7, 0x39, 0x8d, 0x9f, 0x5a, 0x39, 0x68, 0xe5, 0x5c, 0x8e, 0x4b, 0x82, 0x3d, 0xa4, 0x93, 0x8c, 0xd7, 0x44, 0x90, 0x6d, 0x25, 0xbf, 0x35, 0xdc, 0x6f, 0xde, 0x5a, 0x33, 0x63, 0xc6, 0xf1, 0x83, 0xa6, 0x33, 0xc5, 0x89, 0x38, 0xf0, 0xe2, 0xe6, 0x1b, 0xb0, 0x04, 0x56, 0xdc, 0x10, 0x0b, 0x62, 0x19, 0x7f, 0xd6, 0xe2, 0x07, 0x35, 0x09, 0xb3, 0x77, 0xf1, 0x88, 0x57, 0xfd, 0x20, 0xc9, 0x8c, 0x28, 0x3b, 0x96, 0x8b, 0x78, 0x77, 0x5c, 0x74, 0x37, 0x9a, 0xb6, 0xf2, 0x31, 0xba, 0xa0, 0x69, 0xae, 0xfe, 0x2b, 0x3a, 0x09, 0xec, 0x53, 0xfe, 0xfa, 0x8d, 0x2d, 0x49, 0xa3, 0xbb, 0x7e, 0x89, 0x7d, 0x87, 0x38, 0x47, 0xe8, 0x85, 0xc7, 0x40, 0x10, 0x5b, 0xe8, 0x67, 0x40, 0xf6, 0xd2, 0xd0, 0xea, 0x56, 0x8f, 0xeb, 0xf0, 0xba, 0xc6, 0x8e, 0x86, 0x23, 0x11, 0xb6, 0x66, 0xd9, 0x4e, 0x3f, 0xd8, 0x76, 0x3a, 0xda, 0xb0, 0xe6, 0x23, 0x75, 0xfa, 0x39, 0x3a, 0xf3, 0x4d, 0x86, 0x41, 0xfa, 0xd1, 0x26, 0x57, 0xe1, 0x97, 0x45, 0x0a, 0x90, 0x8d, 0xd8, 0x63, 0x9f, 0x7c, 0x56, 0x46, 0x43, 0x2f, 0xa4, 0x1e, 0x14, 0xb1, 0x92, 0xd0, 0x0c, 0x5c, 0x5c, 0x8b, 0x3f, 0xcc, 0x15, 0xc0, 0x37, 0xdd, 0x4a, 0xa5, 0x54, 0xb3, 0x9c, 0x39, 0x1a, 0x06, 0x93, 0xa2, 0x72, 0xf6, 0x86, 0x88, 0xff, 0x28, 0x15, 0x5c, 0x9c, 0xbd, 0xa2, 0x00, 0xd9, 0x3f, 0xb6, 0xe8, 0xb7, 0xb1, 0xef, 0xe6, 0x06, 0x16, 0x64, 0xb2, 0x46, 0x6d, 0xf1, 0xd3, 0xd8, 0x02, 0x08, 0xf4, 0x13, 0x15, 0x04, 0xe3, 0x9c, 0x3e, 0xa1, 0xe9, 0x0f, 0x69, 0x5a, 0x0f, 0x81, 0x94, 0x14, 0x6e, 0xd2, 0x68, 0xd0, 0x74, 0xed, 0xa3, 0xa3, 0x1e, 0xab, 0x7f, 0xc3, 0x54, 0x0d, 0x18, 0x3c, 0x19, 0x2a, 0x3a, 0x92, 0x71, 0xba, 0x45, 0xf6, 0x3c, 0xef, 0x5a, 0xe0, 0x07, 0xb4, 0x0e, 0x31, 0xe2, 0x0f, 0x93, 0xbc, 0xd8, 0xb8, 0x4a, 0xc5, 0x48, 0xd0, 0x3d, 0x79, 0x92, 0xf0, 0xb4, 0xf4, 0xb3, 0xf0, 0xc4, 0x7f, 0xc1, 0xe6, 0xdc, 0xf0, 0x93, 0xf1, 0x5d, 0xb8, 0x7e, 0x4f, 0xaa, 0xf0, 0x9b, 0x2b, 0xda, 0x21, 0xd5, 0xab, 0x8a, 0xad, 0xae, 0xcf, 0x59, 0x36, 0x00, 0xbe, 0xef, 0x16, 0xfa, 0x50, 0xf2, 0xf7, 0x72, 0xd6, 0xe8, 0x8a, 0xfc, 0xf9, 0xa7, 0x1a, 0xc4, 0x18, 0x0d, 0x07, 0x5f, 0x8c, 0x06, 0x68, 0xd0, 0x87, 0x2d, 0xde, 0x30, 0xd3, 0x8c, 0x39, 0x1f, 0x77, 0xae, 0x7c, 0x24, 0xc0, 0xb0, 0x67, 0x18, 0x17, 0xdc, 0x0e, 0x13, 0x93, 0x5a, 0xca, 0xb2, 0x80, 0x00, 0x00, 0x03, 0x2c, 0x1d, 0x4d, 0xe0 };
	int pkt_len = 0;
	int use_mki = 1;
	unsigned char CSKID[5] = { 0x2c, 0x1d, 0x4d, 0xe0 };
	*/

	//QC-RTCP 27 followed by CONNECT
	//Giving 2 packets to WolfSSL for decryption
	unsigned int client_ssrc = 2513047605;
	srtp_crypto_info srtp_cinfo = { 0 };
	srtp_crypto_info* srtp_info = &srtp_cinfo;
	char MasterKey[] = { 0x41, 0xbe, 0x68, 0x89, 0xeb, 0x1e, 0x55, 0x05, 0xdb, 0x81, 0x81, 0xe0, 0xe4, 0x4b, 0xfa, 0x0b, 0xde, 0x7d, 0xb7, 0x03, 0x24, 0xd9, 0xb7, 0x45, 0x59, 0x16, 0x6c, 0xdb };
	unsigned char pkt1[] = { 0x9b, 0xcc, 0x00, 0x16, 0xe7, 0x75, 0x95, 0x38, 0x8d, 0x8c, 0x21, 0x91, 0xcc, 0x2e, 0x22, 0x85, 0x36, 0x46, 0xbd, 0xf3, 0x5d, 0xcb, 0x73, 0xc5, 0xe4, 0x53, 0xd7, 0x9b, 0xd2, 0x7d, 0xbd, 0x6b, 0x4f, 0x92, 0x4c, 0xbb, 0xae, 0x7a, 0x27, 0x7a, 0x92, 0x73, 0x67, 0xfe, 0x35, 0xaf, 0x31, 0xb8, 0xc6, 0x98, 0xa7, 0x01, 0xd9, 0xf6, 0x68, 0x31, 0x15, 0xd2, 0x8a, 0xdb, 0x5f, 0x2c, 0xaa, 0xec, 0x00, 0x0a, 0x9e, 0xa3, 0x69, 0x0c, 0x21, 0x79, 0x70, 0x46, 0x7d, 0x34, 0xe3, 0xec, 0x06, 0x6f, 0xa6, 0xf1, 0xea, 0xa8, 0x16, 0xc9, 0x2f, 0x23, 0x26, 0xab, 0x84, 0x05, 0x00, 0x07, 0xa9, 0x3d, 0xec, 0x4a, 0x13, 0x2c, 0xfd, 0x9c, 0x8e, 0x00, 0x53, 0xca, 0x45, 0x45, 0x80, 0x00, 0x00, 0x0b, 0x2d, 0x62, 0x9a, 0xa6 };
	unsigned char pkt2[] = { 0x90, 0xcc, 0x00, 0x57, 0xe7, 0x75, 0x95, 0x38, 0xf9, 0xb7, 0xdd, 0x04, 0x3e, 0xb4, 0x5b, 0xc6, 0x1d, 0x92, 0xcc, 0x46, 0x87, 0x09, 0x31, 0xf8, 0xb4, 0xf4, 0x1b, 0x46, 0x8d, 0x76, 0xc2, 0xc5, 0x57, 0xf9, 0xaa, 0x7c, 0x5a, 0x3e, 0x68, 0x16, 0x60, 0xba, 0x12, 0x05, 0x63, 0x63, 0x5d, 0x0f, 0xcf, 0x56, 0x3e, 0xe3, 0xe6, 0xd5, 0xb3, 0x75, 0x3a, 0x21, 0x6c, 0xe4, 0x34, 0xd5, 0x0c, 0x2f, 0x7f, 0x0e, 0xe7, 0x92, 0x65, 0x03, 0xa6, 0x24, 0x8f, 0x67, 0x79, 0xd7, 0x20, 0xde, 0xc1, 0x16, 0xcc, 0x12, 0x61, 0xd4, 0x4f, 0xff, 0x72, 0x9e, 0x94, 0x3e, 0xd5, 0x28, 0x36, 0xbf, 0xea, 0xfa, 0x47, 0x63, 0xc4, 0xe2, 0x54, 0xf5, 0x3c, 0x09, 0x84, 0xf0, 0x62, 0xa3, 0x83, 0x7f, 0xeb, 0x65, 0x41, 0x73, 0x79, 0x5a, 0x29, 0x78, 0xd8, 0x00, 0x93, 0x7a, 0xf6, 0x1d, 0x91, 0xa0, 0xfb, 0xc4, 0xbb, 0x96, 0x8c, 0xd6, 0xda, 0xc9, 0x1c, 0x26, 0x3e, 0x35, 0xec, 0x1e, 0xb8, 0x84, 0x7c, 0xa4, 0xf9, 0x59, 0xb0, 0x09, 0xfa, 0x96, 0xaa, 0x04, 0xa0, 0x29, 0x54, 0xbd, 0x33, 0x91, 0x4e, 0x1b, 0x1c, 0x5d, 0x90, 0xae, 0xc8, 0xea, 0x5e, 0xda, 0xb2, 0x62, 0xb8, 0x4a, 0x06, 0xa4, 0xbd, 0xa9, 0xb2, 0xeb, 0xab, 0x07, 0x71, 0x7b, 0xff, 0xfe, 0xe7, 0x69, 0xa6, 0xa6, 0x72, 0x89, 0x28, 0x9e, 0xe9, 0x63, 0x67, 0x91, 0xf1, 0x0a, 0x63, 0xb5, 0xc7, 0x68, 0x8b, 0x22, 0xcd, 0x31, 0x48, 0xa6, 0x64, 0xc2, 0x9b, 0xdc, 0x5d, 0xc3, 0x9e, 0xf1, 0x91, 0x6a, 0x60, 0x43, 0x26, 0xec, 0x1f, 0xf3, 0xf1, 0xd8, 0x7b, 0x33, 0x44, 0xf8, 0xcf, 0xef, 0xe8, 0xc4, 0x6e, 0x1e, 0xd4, 0x47, 0x39, 0x25, 0x3b, 0x3a, 0x49, 0x5b, 0x11, 0x73, 0x38, 0x11, 0x97, 0x95, 0x35, 0x46, 0xf9, 0x9a, 0xad, 0xeb, 0x31, 0x9b, 0x72, 0xe1, 0x32, 0xb3, 0x8d, 0x7c, 0xef, 0xbb, 0x71, 0x0a, 0x1c, 0xb9, 0x67, 0xb9, 0xb8, 0x02, 0x64, 0x0b, 0x6d, 0x61, 0x34, 0x1b, 0xa9, 0x2f, 0x9e, 0xfe, 0x28, 0xe5, 0x61, 0xee, 0xa4, 0x57, 0xae, 0x73, 0xf6, 0xc5, 0x07, 0xbc, 0x52, 0x0e, 0x3b, 0xa8, 0xcf, 0xe8, 0xd5, 0x64, 0x50, 0x77, 0x9e, 0xac, 0x6b, 0x0c, 0xd0, 0x51, 0xcb, 0x8f, 0x7b, 0xff, 0x52, 0x9d, 0xe3, 0x66, 0xe9, 0x8c, 0xb9, 0x7f, 0xce, 0x8c, 0xb3, 0xdc, 0x55, 0x3d, 0x11, 0x6d, 0x6d, 0x6b, 0x56, 0x54, 0x77, 0x86, 0x7a, 0xb7, 0x13, 0x9a, 0x55, 0x1a, 0x71, 0xa3, 0x57, 0xcf, 0xb0, 0x89, 0xd9, 0xcf, 0x62, 0xc1, 0xfb, 0xf4, 0x63, 0x92, 0xed, 0x80, 0x18, 0xad, 0xe1, 0xa2, 0x0b, 0x07, 0x60, 0xb5, 0x80, 0x00, 0x00, 0x0c, 0x2d, 0x62, 0x9a, 0xa6 };
	int pkt1_len = 0;
	int pkt2_len = 0;
	int use_mki = 1;
	unsigned char CSKID[5] = { 0x2d, 0x62, 0x9a, 0xa6 };

	strncpy((char *)srtp_info->csk_mki_idVal,(const char *) CSKID, 4);
	srtp_info->csk_mki_idSize = 4;

	memset(&srtp_info->tx_policy_neg, 0, sizeof(srtp_info->tx_policy_neg));
	memset(&srtp_info->rx_policy_neg, 0, sizeof(srtp_info->rx_policy_neg));

	srtp_info->tx_policy_neg.flags = 0;
	strcpy(srtp_info->tx_policy_neg.name.ptr, "AEAD_AES_128_GCM");
	srtp_info->tx_policy_neg.name.slen = 16;	
	strcpy(srtp_info->tx_policy_neg.key.ptr, MasterKey);
	srtp_info->tx_policy_neg.key.slen = 28;
		
	srtp_info->rx_policy_neg.flags = 0;
	strcpy(srtp_info->rx_policy_neg.name.ptr, "AEAD_AES_128_GCM");
	srtp_info->rx_policy_neg.name.slen = 16;
	strcpy(srtp_info->rx_policy_neg.key.ptr, MasterKey);
	srtp_info->rx_policy_neg.key.slen = 28;

		
	srtp_info->tx_policy_neg.eKeyType = srtp_info->rx_policy_neg.eKeyType = MSI_I_MESSAGE_TYPE_CSK;
	
	//Initint SRTP & Creating Context
	srtp_lib_init();

	srtp_start(srtp_info, client_ssrc);

	srtp_err_status_t srtp_lib_err = srtp_err_status_ok;

	//pkt_len = 376;//QC-CONNECT
	pkt1_len = 116;	//RTCP 27
	pkt2_len = 376; //CONNECT

	print_buffer("\nPacket 1 Data (Encrypted by OpenSSL):", pkt1, pkt1_len);

	//SRTP API Call for Packet Decryption
	srtp_lib_err = srtp_unprotect_rtcp_mki((srtp_t)srtp_info->srtcp_tx_rx_ctx, pkt1, &pkt1_len, use_mki);
	if (srtp_lib_err != srtp_err_status_ok)
	{
		printf("Packet-1 Decrypt Fail\n\n");
		return -1;
	}	
	print_buffer("\nPacket 1 Data (Decrypted by WolfSSL):", pkt1, pkt1_len);
	printf("\nPacket-1 Decryption SUCCESS\n\n");
	

	print_buffer("\nPacket 2 Data (Encrypted by OpenSSL):", pkt2, pkt2_len);
	//SRTP API Call for Packet Decryption
	srtp_lib_err = srtp_unprotect_rtcp_mki((srtp_t)srtp_info->srtcp_tx_rx_ctx, pkt2, &pkt2_len, use_mki);
	if (srtp_lib_err != srtp_err_status_ok)
	{
		printf("\nPacket-2 Decrypt Fail\n\n");
		return -1;
	}
	print_buffer("\nPacket 2 Data (Decrypted by WolfSSL):", pkt2, pkt2_len);
	printf("\nPacket-2 Decryption SUCCESS\n\n");
	
}


int srtp_lib_init ()
{
	srtp_err_status_t err;
	srtp_crypto_info srtp_info;

	err = srtp_init();
	if (err != srtp_err_status_ok) {
		printf("\n\nFailed to initialize libsrtp");
		return -1;
	}

}

int srtp_start(srtp_crypto_info *srtp_info, unsigned int client_ssrc)
{
	srtp_policy_t    tx_;
	srtp_policy_t    rx_;
	srtp_master_key_t mKeys[2] = { 0 }, * pmKeys = NULL;
	srtp_err_status_t err;

	int		     cr_tx_idx = 0;
	int		     au_tx_idx = 0;
	int		     cr_rx_idx = 0;
	int		     au_rx_idx = 0;

	cr_tx_idx = au_tx_idx = 1; // 1 for AES GCM 128
	cr_rx_idx = au_rx_idx = 1; // 1 for AES GCM 128

	/* Init transmit direction */
	memset(&tx_, 0, sizeof(srtp_policy_t));
	memmove(srtp_info->tx_key, srtp_info->tx_policy_neg.key.ptr, srtp_info->tx_policy_neg.key.slen);

	if (cr_tx_idx && au_tx_idx)
		tx_.rtp.sec_serv = sec_serv_conf_and_auth;
	else if (cr_tx_idx)
		tx_.rtp.sec_serv = sec_serv_conf;
	else if (au_tx_idx)
		tx_.rtp.sec_serv = sec_serv_auth;
	else
		tx_.rtp.sec_serv = sec_serv_none;

	/*MKI changes START*/
	mKeys[0].key = (uint8_t*)srtp_info->tx_key;
	mKeys[0].mki_id = srtp_info->csk_mki_idVal;
	mKeys[0].mki_size = srtp_info->csk_mki_idSize;
	pmKeys = &mKeys[0];
	tx_.keys = &pmKeys; // &mKeys[0];
	tx_.num_master_keys = 1;
	/*MKI changes END*/

	tx_.ssrc.type = ssrc_specific;
	tx_.ssrc.value = client_ssrc;

	tx_.rtp.cipher_type = crypto_suites[cr_tx_idx].cipher_type;
	tx_.rtp.cipher_key_len = crypto_suites[cr_tx_idx].cipher_key_len;
	tx_.rtp.auth_type = crypto_suites[au_tx_idx].auth_type;
	tx_.rtp.auth_key_len = crypto_suites[au_tx_idx].auth_key_len;
	tx_.rtp.auth_tag_len = crypto_suites[au_tx_idx].srtp_auth_tag_len;
	tx_.rtcp = tx_.rtp;
	tx_.rtcp.auth_tag_len = crypto_suites[au_tx_idx].srtcp_auth_tag_len;
	tx_.next = &rx_;

	srtp_info->tx_policy = srtp_info->tx_policy_neg;

	//printf("\nTx => tx_policy.name [%.*s]", srtp_info->tx_policy.name.slen, srtp_info->tx_policy.name.ptr);
	//printf("\nTx => auth_type [%d]", tx_.rtp.auth_type);
	//printf("\nTx => auth_key_len [%d]", tx_.rtp.auth_key_len);
	//printf("\nTx => auth_tag_len [%d]", tx_.rtp.auth_tag_len);

	/* Init receive direction */
	memset(&rx_, 0, sizeof(srtp_policy_t));
	memmove(srtp_info->rx_key, srtp_info->rx_policy_neg.key.ptr, srtp_info->rx_policy_neg.key.slen);

	if (cr_rx_idx && au_rx_idx)
		rx_.rtp.sec_serv = sec_serv_conf_and_auth;
	else if (cr_rx_idx)
		rx_.rtp.sec_serv = sec_serv_conf;
	else if (au_rx_idx)
		rx_.rtp.sec_serv = sec_serv_auth;
	else
		rx_.rtp.sec_serv = sec_serv_none;

	/*MKI changes START*/
	mKeys[0].key = (uint8_t*)srtp_info->rx_key;
	mKeys[0].mki_id = srtp_info->csk_mki_idVal;
	mKeys[0].mki_size = srtp_info->csk_mki_idSize;
	pmKeys = &mKeys[0];
	rx_.keys = &pmKeys; // &mKeys[0];
	rx_.num_master_keys = 1;
	/*MKI changes END*/

	rx_.ssrc.type = ssrc_any_inbound;
	rx_.ssrc.value = 0;
	rx_.rtp.sec_serv = crypto_suites[cr_rx_idx].service;
	rx_.rtp.cipher_type = crypto_suites[cr_rx_idx].cipher_type;
	rx_.rtp.cipher_key_len = crypto_suites[cr_rx_idx].cipher_key_len;
	rx_.rtp.auth_type = crypto_suites[au_rx_idx].auth_type;
	rx_.rtp.auth_key_len = crypto_suites[au_rx_idx].auth_key_len;
	rx_.rtp.auth_tag_len = crypto_suites[au_rx_idx].srtp_auth_tag_len;
	rx_.rtcp = rx_.rtp;
	rx_.rtcp.auth_tag_len = crypto_suites[au_rx_idx].srtcp_auth_tag_len;
	rx_.next = NULL;

	err = srtp_create((srtp_t*)&srtp_info->srtcp_tx_rx_ctx, &tx_);
	if (err != srtp_err_status_ok) {
		return -1;
	}

	srtp_info->rx_policy = srtp_info->rx_policy_neg;

	//printf("\nRx => rx_policy.name [%.*s]", srtp_info->rx_policy.name.slen, srtp_info->rx_policy.name.ptr);
	//printf("\nRx => auth_type [%d]", rx_.rtp.auth_type);
	//printf("\nRx => auth_key_len [%d]", rx_.rtp.auth_key_len);
	//printf("\nRx => auth_tag_len [%d]", rx_.rtp.auth_tag_len);
}

//Print Buffer in HEX
void print_buffer(const char* name, unsigned char* data, int len)
{
	int i; printf("%s", name); for (i = 0; i < len; i++) { if ((i % 8) == 0) printf("\n "); printf("0x%02x, ", data[i]); } printf("\n");
}