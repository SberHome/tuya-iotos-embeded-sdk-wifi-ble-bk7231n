/*
 * WPA Supplicant / Configuration backend: empty starting point
 * Copyright (c) 2003-2005, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 *
 * This file implements dummy example of a configuration backend. None of the
 * functions are actually implemented so this can be used as a simple
 * compilation test or a starting point for a new configuration backend.
 */

#include "includes.h"

#include "common.h"
#include "config.h"
#include "base64.h"
#include "param_config.h"
#include "uart_pub.h"
#include "wlan_ui_pub.h"
#include "wpa_common.h"
#include "role_launch.h"

#ifndef CFG_NONE_DEBUG
#define CFG_NONE_DEBUG 0
#endif
#define debug_print(...)  do { if (CFG_NONE_DEBUG) os_printf("[CFGN]"__VA_ARGS__); } while (0);


#if CFG_IEEE80211N
static void wpa_config_ht_cap_by_sec(int sec)
{
	if(sec == SECURITY_TYPE_WEP
		|| sec == SECURITY_TYPE_WPA_TKIP
		|| sec == SECURITY_TYPE_WPA2_TKIP)
	{
		rw_ieee80211_set_ht_cap(0);
	}
	else
	{
		rw_ieee80211_set_ht_cap(1);
	}
}
#endif

static int wpa_config_validate_network(struct wpa_ssid *ssid, int line)
{
	int errors = 0;

	if ((ssid->group_cipher & WPA_CIPHER_CCMP) &&
	    !(ssid->pairwise_cipher & WPA_CIPHER_CCMP) &&
	    !(ssid->pairwise_cipher & WPA_CIPHER_NONE)) {
		/* Group cipher cannot be stronger than the pairwise cipher. */
		wpa_printf(MSG_DEBUG, "Line %d: removed CCMP from group cipher"
			   " list since it was not allowed for pairwise "
			   "cipher", line);
		ssid->group_cipher &= ~WPA_CIPHER_CCMP;
	}

	if (ssid->mode == WPAS_MODE_MESH &&
	    (ssid->key_mgmt != WPA_KEY_MGMT_NONE &&
	    ssid->key_mgmt != WPA_KEY_MGMT_SAE)) {
		wpa_printf(MSG_ERROR,
			   "Line %d: key_mgmt for mesh network should be open or SAE",
			   line);
		errors++;
	}

	return errors;
}

static int set_wpa_psk(struct wpa_ssid *ssid)
{
	int errors = 0;

	if (g_sta_param_ptr->key_len < 8 || g_sta_param_ptr->key_len > 64) {
		debug_print("Invalid passphrase "
		   "length %lu (expected: 8..63) '%s'.",
		   (unsigned long) g_sta_param_ptr->key_len, (char *)g_sta_param_ptr->key);
		errors++;

        return 1;
	} else if (g_sta_param_ptr->key_len == 64) {
		wpa_printf(MSG_ERROR, "use PSK");
		if (hexstr2bin((char*)g_sta_param_ptr->key, ssid->psk, 32) ||
		    g_sta_param_ptr->key[64] != '\0') {
			wpa_printf(MSG_ERROR, "Invalid PSK '%s'.",
				    g_sta_param_ptr->key);
			errors++;
		}
		ssid->passphrase = NULL;
		ssid->psk_set = 1;
	} else {
		str_clear_free(ssid->passphrase);
		ssid->passphrase = dup_binstr(g_sta_param_ptr->key, g_sta_param_ptr->key_len);
		ssid->psk_set = 0;
	}
	ssid->key_mgmt |= WPA_KEY_MGMT_PSK;
	ssid->mem_only_psk = 0;
	
    debug_print("errors=%d,key_len=%d,orig_key_len=%d,%.*s,%.*s\n", errors, g_sta_param_ptr->key_len, g_sta_param_ptr->orig_key_len, g_sta_param_ptr->key_len, g_sta_param_ptr->key, g_sta_param_ptr->orig_key_len, g_sta_param_ptr->orig_key);
	return errors;
}

static int set_wep_key(struct wpa_ssid*ssid)
{
	int errors = 0;
	
	ssid->wep_tx_keyidx = 0;
	ssid->auth_alg = WPA_AUTH_ALG_OPEN|WPA_AUTH_ALG_SHARED;
	if(g_sta_param_ptr->key_len == 5 ||
		g_sta_param_ptr->key_len == 13){
		os_memcpy(ssid->wep_key[0], (char *)g_sta_param_ptr->key, g_sta_param_ptr->key_len);
		ssid->wep_key_len[0] = g_sta_param_ptr->key_len;
	}else if(g_sta_param_ptr->key_len == 10 ||
				g_sta_param_ptr->key_len == 26){
		ssid->wep_key_len[0] = g_sta_param_ptr->key_len / 2;
		hexstr2bin((char*)g_sta_param_ptr->key, (unsigned char*)ssid->wep_key[0],ssid->wep_key_len[0] ); 
	}else if(g_sta_param_ptr->orig_key_len == 5 ||
		g_sta_param_ptr->orig_key_len == 13){
		os_memcpy(ssid->wep_key[0], (char *)g_sta_param_ptr->orig_key, g_sta_param_ptr->orig_key_len);
		ssid->wep_key_len[0] = g_sta_param_ptr->orig_key_len;
	}else if(g_sta_param_ptr->orig_key_len == 10 ||
				g_sta_param_ptr->orig_key_len == 26){
		ssid->wep_key_len[0] = g_sta_param_ptr->orig_key_len / 2;
		hexstr2bin((char*)g_sta_param_ptr->orig_key, (unsigned char*)ssid->wep_key[0],ssid->wep_key_len[0]);
	}else{
		errors++;
#if RL_SUPPORT_FAST_CONNECT
		debug_print("rl_clear_bssid_info\n");
		rl_clear_bssid_info();
#endif
	}
	ssid->mem_only_psk = 0;
	debug_print("errors=%d,key_len=%d,orig_key_len=%d,%.*s,%.*s\n", errors, g_sta_param_ptr->key_len, g_sta_param_ptr->orig_key_len, g_sta_param_ptr->key_len, g_sta_param_ptr->key, g_sta_param_ptr->orig_key_len, g_sta_param_ptr->orig_key);
	return errors;
}

static int cipher2security(struct wpa_ie_data *ie)
{
    debug_print("%d %d %d %d\n", ie->key_mgmt, ie->proto, ie->pairwise_cipher, ie->group_cipher);
	switch(ie->key_mgmt) {
	case WPA_KEY_MGMT_PSK:
		if (ie->proto == WPA_PROTO_WPA) {
			if (ie->pairwise_cipher & WPA_CIPHER_CCMP)
				return SECURITY_TYPE_WPA_AES;
			else
				return SECURITY_TYPE_WPA_TKIP;
		} else if (ie->proto == (WPA_PROTO_WPA|WPA_PROTO_RSN)) {
			return SECURITY_TYPE_WPA2_MIXED;
		} else {
			if (ie->pairwise_cipher == (WPA_CIPHER_CCMP|WPA_CIPHER_TKIP))
				return SECURITY_TYPE_WPA2_MIXED;
			else if (ie->pairwise_cipher & WPA_CIPHER_CCMP)
				return SECURITY_TYPE_WPA2_AES;
			else
				return SECURITY_TYPE_WPA2_TKIP;
		}
		break;
	case WPA_KEY_MGMT_NONE:
		if (ie->pairwise_cipher == WPA_CIPHER_NONE)
			return SECURITY_TYPE_NONE;
		else
			return SECURITY_TYPE_WEP;
		break;
	default:
		return SECURITY_TYPE_AUTO;
	}
}

static int security2cipher(struct wpa_ie_data *ie, int security)
{
	os_memset(ie, 0, sizeof(*ie));
	
	switch(security) {
	case SECURITY_TYPE_NONE:
		ie->key_mgmt = WPA_KEY_MGMT_NONE;
		ie->pairwise_cipher = WPA_CIPHER_NONE;
        break;
    case SECURITY_TYPE_WEP:
        ie->key_mgmt = WPA_KEY_MGMT_NONE;
		ie->pairwise_cipher = WPA_CIPHER_WEP40 | WPA_CIPHER_WEP104;
        break;
    case SECURITY_TYPE_WPA_TKIP:
		ie->key_mgmt = WPA_KEY_MGMT_PSK;
		ie->pairwise_cipher = WPA_CIPHER_TKIP;
		ie->group_cipher = WPA_CIPHER_TKIP;
		ie->proto = WPA_PROTO_WPA;
		break;
	
    case SECURITY_TYPE_WPA2_TKIP:
        ie->key_mgmt = WPA_KEY_MGMT_PSK;
		ie->pairwise_cipher = WPA_CIPHER_TKIP;
		ie->group_cipher = WPA_CIPHER_TKIP;
		ie->proto = WPA_PROTO_RSN;
        break;
    case SECURITY_TYPE_WPA_AES:
		ie->key_mgmt = WPA_KEY_MGMT_PSK;
		ie->pairwise_cipher = WPA_CIPHER_TKIP|WPA_CIPHER_CCMP;
		ie->group_cipher = WPA_CIPHER_TKIP;
		ie->proto = WPA_PROTO_WPA;
		break;
    case SECURITY_TYPE_WPA2_AES:
        ie->key_mgmt = WPA_KEY_MGMT_PSK;
		ie->pairwise_cipher = WPA_CIPHER_CCMP;
		ie->group_cipher = WPA_CIPHER_CCMP;
		ie->proto = WPA_PROTO_RSN;
        break;
    case SECURITY_TYPE_WPA2_MIXED:
        ie->key_mgmt = WPA_KEY_MGMT_PSK;
		ie->pairwise_cipher = WPA_CIPHER_TKIP|WPA_CIPHER_CCMP;
		ie->group_cipher = WPA_CIPHER_TKIP;
		ie->proto = WPA_PROTO_WPA|WPA_PROTO_RSN;
        break;
    default:
		break;
	}
    debug_print("%d %d %d %d security=%d\n", ie->key_mgmt, ie->proto, ie->pairwise_cipher, ie->group_cipher, security);

	return 0;
}

int wpa_config_set_none(struct wpa_ssid *ssid)
{
	ssid->key_mgmt = WPA_KEY_MGMT_NONE;
	ssid->mem_only_psk = 0;
	g_sta_param_ptr->cipher_suite = SECURITY_TYPE_NONE;

	return 0;
}

int wpa_config_set_wep(struct wpa_ssid *ssid)
{
	int ret;
	
	ret = set_wep_key(ssid);
	if(!ret) {
		g_sta_param_ptr->cipher_suite = SECURITY_TYPE_WEP;
	}else{
		mhdr_set_station_status(RW_EVT_STA_PASSWORD_WRONG);
	}
	
	return 0;
}

#if RL_SUPPORT_FAST_CONNECT
static void wpa_config_update_fast_psk(struct wpa_ssid *ssid)
{
	RL_BSSID_INFO_T bssid_info;
	rl_read_bssid_info(&bssid_info);	

	if(os_memcmp(ssid->ssid, bssid_info.ssid, ssid->ssid_len) == 0
		&& os_strcmp(ssid->passphrase, bssid_info.pwd) == 0)
	{
		debug_print("Skip PSK caculation if SSID and passphrase are same \n");
		os_memset(&ssid->psk, 0, sizeof(ssid->psk));
		os_strcpy(ssid->psk, bssid_info.psk);
		ssid->psk_set = 1;
	}
	else
	{
		wpa_config_update_psk(ssid);
	}
}
#endif

int wpa_config_set_wpa(struct wpa_ssid *ssid, struct wpa_ie_data *ie)
{
	int ret;
	
	ssid->group_cipher = ie->group_cipher;
	ssid->pairwise_cipher = ie->pairwise_cipher;
	ssid->key_mgmt = ie->key_mgmt;
	ssid->proto = ie->proto;

	if ((ssid->psk_set) || (ssid->passphrase!=NULL)) {
		return 0;
	}
	
	ret = set_wpa_psk(ssid);
	if(!ret){
		g_sta_param_ptr->cipher_suite = cipher2security(ie);
		#if CFG_IEEE80211N
		wpa_config_ht_cap_by_sec(g_sta_param_ptr->cipher_suite);
		#endif
		if (ssid->passphrase && (ssid->psk_set == 0)) {
			#if RL_SUPPORT_FAST_CONNECT
			wpa_config_update_fast_psk(ssid);
			#else
			wpa_config_update_psk(ssid);
			#endif
		}
	}else{
		mhdr_set_station_status(RW_EVT_STA_PASSWORD_WRONG);
	}		
	
	return ret;
}

static struct wpa_ssid * wpa_config_read_network(int *line, int id)
{
	struct wpa_ssid *ssid;
	struct wpa_ie_data ie;
	int errors = 0;

	ssid = os_zalloc(sizeof(*ssid));
	if (ssid == NULL)
		return NULL;
	
	dl_list_init(&ssid->psk_list);
	ssid->id = id;

	wpa_config_set_network_defaults(ssid);

	ssid->ssid = (u8*)dup_binstr(g_sta_param_ptr->ssid.array, g_sta_param_ptr->ssid.length);
	ssid->ssid_len = g_sta_param_ptr->ssid.length;
	ssid->key_mgmt = 0;

	if ((g_sta_param_ptr->fast_connect_set) && (g_sta_param_ptr->cipher_suite != SECURITY_TYPE_AUTO)){
		os_memcpy(ssid->bssid, g_sta_param_ptr->fast_connect.bssid, 6);
		debug_print("bssid %02x-%02x-%02x-%02x-%02x-%02x\n",
			ssid->bssid[0],ssid->bssid[1],ssid->bssid[2],
			ssid->bssid[3],ssid->bssid[4],ssid->bssid[5]);
		ssid->bssid_set = 1;
		if(g_sta_param_ptr->cipher_suite == SECURITY_TYPE_NONE){
			/*do nothing*/
		}else if(g_sta_param_ptr->cipher_suite == SECURITY_TYPE_WEP){
			errors += wpa_config_set_wep(ssid);			
		}else{
			#if CFG_IEEE80211N
			wpa_config_ht_cap_by_sec(g_sta_param_ptr->cipher_suite);
			#endif
			security2cipher(&ie, g_sta_param_ptr->cipher_suite);
			wpa_config_set_wpa(ssid, &ie);
		} 
	} else {
		ssid->mem_only_psk = 1;
	}
	
	errors += wpa_config_validate_network(ssid, *line);
	if(errors){
		wpa_config_free_ssid(ssid);
		ssid = NULL;
	}

	return ssid;
}

struct wpa_config * wpa_config_read(const char *name, struct wpa_config *cfgp)
{
	struct wpa_config *config;
	struct wpa_ssid *ssid, *tail, *head;
	int id = 0, line = 0, errors = 0;

	if (cfgp)
		config = cfgp;
	else
		config = wpa_config_alloc_empty(NULL, NULL);
	if (config == NULL)
		return NULL;
	tail = head = config->ssid;
	while (tail && tail->next)
		tail = tail->next;
	
	ssid = wpa_config_read_network(&line, id++);
	if(ssid == NULL){
		errors++;
		goto error;
	}
	if (head == NULL) {
		head = tail = ssid;
	} else {
		tail->next = ssid;
		tail = ssid;
	}
	if(wpa_config_add_prio_network(config, ssid)){
		errors++;
		goto error;
	}

	config->ssid = head;
	wpa_config_debug_dump_networks(config);

error:
	if(errors){
		wpa_config_free(config);
		config = NULL;
		head = NULL;
	}
	return config;
}


int wpa_config_write(const char *name, struct wpa_config *config)
{
	struct wpa_ssid *ssid;
	//struct wpa_config_blob *blob;

	wpa_printf(MSG_DEBUG, "Writing configuration file '%s'", name);

	/* TODO: write global config parameters */


#if 0
	for (ssid = config->ssid; ssid; ssid = ssid->next) {
		/* TODO: write networks */
	}

	for (blob = config->blobs; blob; blob = blob->next) {
		/* TODO: write blobs */
	}
#endif

	return 0;
	(void)ssid;
}
// eof

