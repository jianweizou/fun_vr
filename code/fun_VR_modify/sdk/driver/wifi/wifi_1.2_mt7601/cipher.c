#include "cipher.h"
#include "wifi_api.h"
#include "asic_ctrl.h"
#include "mlme.h"
#include "nl80211.h"


char*   CipherName[] = {"none","wep64","wep128","TKIP","AES","CKIP64","CKIP128","CKIP152","SMS4"};


/* WPA OUI*/
extern UCHAR 	OUI_WPA[3];
extern UCHAR	OUI_WPA_NONE_AKM[4];
extern UCHAR   	OUI_WPA_VERSION[4];
extern UCHAR 	OUI_WPA_WEP40[4];
extern UCHAR  	OUI_WPA_TKIP[4];
extern UCHAR   	OUI_WPA_CCMP[4];
extern UCHAR   	OUI_WPA_WEP104[4];
extern UCHAR  	OUI_WPA_8021X_AKM[4];
extern UCHAR   	OUI_WPA_PSK_AKM[4];
/* WPA2 OUI*/
extern UCHAR 	OUI_WPA2[3];
extern UCHAR   	OUI_WPA2_WEP40[4];
extern UCHAR   	OUI_WPA2_TKIP[4];
extern UCHAR  	OUI_WPA2_CCMP[4];
extern UCHAR  	OUI_WPA2_8021X_AKM[4];
extern UCHAR   	OUI_WPA2_PSK_AKM[4];
extern UCHAR   	OUI_WPA2_WEP104[4];
extern UCHAR   	OUI_WPA2_1X_SHA256[4];
extern UCHAR   	OUI_WPA2_PSK_SHA256[4];


VOID RTMPSetCipherAlg(
	IN PRTMP_ADAPTER 	pAd)
{
	int idx;
	UCHAR CipherAlg;

	for (idx = 0; idx < SHARE_KEY_NUM; idx++) 
	{
		CipherAlg = pAd->SharedKey[BSS0][idx].CipherAlg;

		if (pAd->SharedKey[BSS0][idx].KeyLen > 0)
		{
			/* Set key material and cipherAlg to Asic */
			AsicAddSharedKeyEntry(pAd, BSS0, idx,
							  &pAd->SharedKey[BSS0][idx]);

			if (idx == pAd->DefaultKeyId) 
			{
				/* Update WCID attribute table and IVEIV table for this group key table */
				RTMPSetWcidSecurityInfo(pAd,
										BSS0,
										idx,
										CipherAlg,
										MCAST_WCID,
										SHAREDKEYTABLE);
			}
		}
	 }
}


VOID RTMPSetChipHWKey(
	IN PRTMP_ADAPTER 	pAd, 
	IN PUCHAR 			key, 
	IN UCHAR 			len, 
	IN UINT 			enc_mode)
{
	int idx = 0;

	if (IS_WPA_CAPABILITY(pAd->ApCfg.AuthMode)) {
		pAd->DefaultKeyId = 1;
	}
	else
		pAd->DefaultKeyId = 0;

	idx = pAd->DefaultKeyId;

	NdisZeroMemory(pAd->SharedKey[BSS0][idx].Key, sizeof(pAd->SharedKey[BSS0][idx].Key));
	
	switch (enc_mode) {
		case Ndis802_11Encryption1Enabled:
			{
				if (len != LEN_WEP128_KEY && len != LEN_WEP64_KEY)
					return;
				
				if (len == LEN_WEP128_KEY) {
					pAd->SharedKey[BSS0][idx].CipherAlg = CIPHER_WEP128;
					pAd->SharedKey[BSS0][idx].KeyLen = len;
					NdisMoveMemory(pAd->SharedKey[0][idx].Key, key, len);
				}
				else {
					pAd->SharedKey[BSS0][idx].CipherAlg = CIPHER_WEP64;
					pAd->SharedKey[BSS0][idx].KeyLen = len;
					NdisMoveMemory(pAd->SharedKey[0][idx].Key, key, len);
				}
			}
			pAd->WEPEnabled = TRUE;
			
			break;
			
		case Ndis802_11WEPDisabled:
			pAd->SharedKey[BSS0][idx].KeyLen = len;
			pAd->SharedKey[BSS0][idx].CipherAlg = CIPHER_NONE ;
			pAd->WEPEnabled = FALSE;
			break;
			
		case Ndis802_11Encryption2Enabled:
			pAd->SharedKey[BSS0][idx].CipherAlg = CIPHER_TKIP;
			pAd->SharedKey[BSS0][idx].KeyLen = len;
			NdisMoveMemory(pAd->SharedKey[0][idx].Key, key, len);
			pAd->WEPEnabled = TRUE;
			break;

		case Ndis802_11Encryption3Enabled:
			pAd->SharedKey[BSS0][idx].CipherAlg = CIPHER_AES;
			pAd->SharedKey[BSS0][idx].KeyLen = len;
			NdisMoveMemory(pAd->SharedKey[BSS0][idx].Key, key, len);
			pAd->WEPEnabled = TRUE;
			break;

		default:

			return;
	}

	/* Security Setting */
	RTMPSetCipherAlg(pAd);
}


VOID RTMPSetWEPCfg(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR 			item, 
	IN PUCHAR 			key, 
	IN UCHAR 			len)
{
//	NDIS_802_11_ENCRYPTION_MODE EncMode;
//	NDIS_802_11_AUTHENTICATION_MODE AuthMode;

//	AuthMode = Ndis802_11AuthModeShared;
//	EncMode = (item == SET_SECURITY_WEP128)? (Ndis802_11Cipher_WEP104):(Ndis802_11Cipher_WEP40);
#if 0	
	MLME_WriteParamToNVARAM(WIFI_DEV_AUTH_MODE, &AuthMode, 1);
	MLME_WriteParamToNVARAM(WIFI_DEV_ENC_MODE, &EncMode, 1);
	MLME_WriteParamToNVARAM(WIFI_DEV_KEY_INFO, key, len);
#endif		

	if (item == SET_SECURITY_WEP128) {
		DBGPRINT(RT_INFO, ("[INFO]%s: Set WEP 128 configuration %s\n", __FUNCTION__, key));
		pAd->ApCfg.EncMode = Ndis802_11Cipher_WEP104;
	}
	else {
		DBGPRINT(RT_INFO, ("[INFO]%s: Set WEP 64 configuration %s\n", __FUNCTION__, key));
		pAd->ApCfg.EncMode = Ndis802_11Cipher_WEP40;
	}

	pAd->ApCfg.WepStatus = Ndis802_11Encryption1Enabled;
	pAd->ApCfg.AuthMode = Ndis802_11AuthModeOpen;
	pAd->ApCfg.GroupKeyWepStatus = Ndis802_11Encryption1Enabled;
	pAd->ApCfg.WpaMixPairCipher = MIX_CIPHER_NOTUSE;

	NdisMoveMemory(pAd->ApCfg.WpaPassPhrase, key, len);
	pAd->ApCfg.WpaPassPhraseLen = len;

#ifdef WIFI_ENCRYPTION_DBG
	DumpEncryptionInformation(pAd);
#endif

	RTMPSetChipHWKey(pAd, key, len, pAd->ApCfg.WepStatus);
}


VOID RTMPUpdateWPAInfo(
	IN PRTMP_ADAPTER 	pAd,
	IN PUCHAR			ssid,
	IN UCHAR			len)
{
	USHORT Wcid = 0;

	/* Generate GMK and GNonce randomly per MBSS */
	GenRandom(pAd, pAd->essid, pAd->ApCfg.GMK);

	GenRandom(pAd, pAd->essid, pAd->ApCfg.GNonce);

	RT_CfgSetWPAPSKKey(pAd, 
						(PSTRING)pAd->ApCfg.WpaPassPhrase, 
						pAd->ApCfg.WpaPassPhraseLen, 
						(PUCHAR)pAd->Ssid, 
						pAd->SsidLen, 
						pAd->ApCfg.PMK);


	GET_GroupKey_WCID(pAd, Wcid, 0);

	pAd->DefaultKeyId = 1;

	/* Derive GTK per BSSID */
	WpaDeriveGTK(pAd->ApCfg.GMK, 
				(UCHAR*)pAd->ApCfg.GNonce, 
				pAd->essid, 
				pAd->ApCfg.GTK, 
				LEN_TKIP_TK);

	/* Install Shared key */
	WPAInstallSharedKey(pAd, 
						pAd->ApCfg.GroupKeyWepStatus, 
						BSS0, 
						pAd->DefaultKeyId, 
						Wcid,
						TRUE,
						pAd->ApCfg.GTK,
						LEN_TKIP_GTK);
}


/* Current version, WPA1-TKIP and WPA2-AES (AP mode)*/
VOID RTMPSetWPACfg(
	IN PRTMP_ADAPTER 	pAd, 
	IN UCHAR 			item, 
	IN PUCHAR 			key, 
	IN UCHAR 			len)

{
//	NDIS_802_11_ENCRYPTION_MODE EncMode;
//	NDIS_802_11_AUTHENTICATION_MODE AuthMode;
	USHORT Wcid = 0;
	
	if (item == SET_SECURITY_WPA) 
	{
//		AuthMode = Ndis802_11AuthModeWPAPSK;
//		EncMode = Ndis802_11Cipher_TKIP;	
	}
	else 
	{
//		AuthMode = Ndis802_11AuthModeWPA2PSK;
//		EncMode = Ndis802_11Cipher_CCMP;
	}
	
#if 0
	MLME_WriteParamToNVARAM(WIFI_DEV_AUTH_MODE, &AuthMode, 1);
	MLME_WriteParamToNVARAM(WIFI_DEV_ENC_MODE, &EncMode, 1);
	MLME_WriteParamToNVARAM(WIFI_DEV_KEY_INFO, key, len);
#endif	

	if (item == SET_SECURITY_WPA) {
		DBGPRINT(RT_INFO, ("[INFO]%s: Set WPA configuration %s\n", __FUNCTION__, key));
		pAd->ApCfg.WepStatus = Ndis802_11Encryption2Enabled;
		pAd->ApCfg.AuthMode = Ndis802_11AuthModeWPAPSK;
		pAd->ApCfg.EncMode = Ndis802_11Cipher_TKIP;
		pAd->ApCfg.GroupKeyWepStatus = Ndis802_11Encryption2Enabled;
		pAd->ApCfg.WpaMixPairCipher = WPA_TKIPAES_WPA2_NONE;
	}
	else {
		DBGPRINT(RT_INFO, ("[INFO]%s: Set WPA2 configuration %s\n", __FUNCTION__, key));
		pAd->ApCfg.WepStatus = Ndis802_11Encryption3Enabled;
		pAd->ApCfg.AuthMode = Ndis802_11AuthModeWPA2PSK;
		pAd->ApCfg.EncMode = Ndis802_11Cipher_CCMP;
		pAd->ApCfg.GroupKeyWepStatus = Ndis802_11Encryption3Enabled;
		pAd->ApCfg.WpaMixPairCipher = WPA_NONE_WPA2_TKIPAES;
	}

	NdisZeroMemory(pAd->ApCfg.WpaPassPhrase, sizeof(pAd->ApCfg.WpaPassPhrase));
	NdisMoveMemory(pAd->ApCfg.WpaPassPhrase, key, len);
	pAd->ApCfg.WpaPassPhraseLen = len;

#ifdef WIFI_ENCRYPTION_DBG
	DumpEncryptionInformation(pAd);
#endif

	RTMPSetChipHWKey(pAd, pAd->ApCfg.WpaPassPhrase, pAd->ApCfg.WpaPassPhraseLen, pAd->ApCfg.WepStatus);
/*
	RT_CfgSetWPAPSKKey(pAd, 
						(PSTRING)pAd->SharedKey[BSS0][1].Key, 
						pAd->SharedKey[BSS0][1].KeyLen, 
						(PUCHAR)pAd->Ssid, 
						pAd->SsidLen, 
						pAd->ApCfg.PMK);
*/

	/* Generate GMK and GNonce randomly per MBSS */
	GenRandom(pAd, pAd->essid, pAd->ApCfg.GMK);

	GenRandom(pAd, pAd->essid, pAd->ApCfg.GNonce);

	RT_CfgSetWPAPSKKey(pAd, 
						(PSTRING)pAd->ApCfg.WpaPassPhrase, 
						pAd->ApCfg.WpaPassPhraseLen, 
						(PUCHAR)pAd->Ssid, 
						pAd->SsidLen, 
						pAd->ApCfg.PMK);

	GET_GroupKey_WCID(pAd, Wcid, 0);

	pAd->DefaultKeyId = 1;

	/* Derive GTK per BSSID */
	WpaDeriveGTK(pAd->ApCfg.GMK, 
				(UCHAR*)pAd->ApCfg.GNonce, 
				pAd->essid, 
				pAd->ApCfg.GTK, 
				LEN_TKIP_TK);

	/* Install Shared key */
	WPAInstallSharedKey(pAd, 
						pAd->ApCfg.GroupKeyWepStatus, 
						BSS0, 
						pAd->DefaultKeyId, 
						Wcid,
						TRUE,
						pAd->ApCfg.GTK,
						LEN_TKIP_GTK);
//	WpaApStartup(pAd);

}


VOID RTMPSetWcidSecurityInfo(
	IN PRTMP_ADAPTER	pAd,
	IN UINT8			BssIdx,
	IN UINT8			KeyIdx,
	IN UINT8			CipherAlg,
	IN UINT8			Wcid,
	IN UINT8			KeyTabFlag)
{
	UINT32			IV = 0;
	UINT8			IV_KEYID = 0;
	
	/* Prepare initial IV value */
	if (CipherAlg == CIPHER_WEP64 || CipherAlg == CIPHER_WEP128)
	{

		UCHAR	TxTsc[LEN_WEP_TSC];

		TxTsc[0] = 0xc8;
		TxTsc[1] = 0x22;
		TxTsc[2] = 0x0;

		/* Update HW IVEIV table */
		IV_KEYID = (KeyIdx << 6);
		IV = (IV_KEYID << 24) | 
			 (TxTsc[2] << 16) |
			 (TxTsc[1] << 8) |
			 (TxTsc[0]);	
	}
	else if (CipherAlg == CIPHER_TKIP || CipherAlg == CIPHER_AES)
	{
		/* Set IVEIV as 1 in Asic -
		In IEEE 802.11-2007 8.3.3.4.3 described :
		The PN shall be implemented as a 48-bit monotonically incrementing
		non-negative integer, initialized to 1 when the corresponding 
		temporal key is initialized or refreshed. */	
		IV_KEYID = (KeyIdx << 6) | 0x20;
		IV = (IV_KEYID << 24) | 1;
	}
	else
	{
		DBGPRINT(RT_ERROR, ("[ERR]%s : Unsupport cipher Alg (%d) for Wcid-%d \n", 
										__FUNCTION__, CipherAlg, Wcid));
		return;
	}
	/* Update WCID IV/EIV table */
	AsicUpdateWCIDIVEIV(pAd, Wcid, IV, 0);
		
	/* Update WCID attribute entry */
	AsicUpdateWcidAttributeEntry(pAd, 
							BssIdx, 
							KeyIdx, 
							CipherAlg,
							Wcid,
							KeyTabFlag);

}


VOID RTMPAddWEPPairwise(
	IN PRTMP_ADAPTER 	pAd, 
	IN USHORT 			aid)
{

	/* Add Pair-wise key to Asic */
	AsicAddPairwiseKeyEntry(
							pAd, 
							aid, 
							&pAd->SharedKey[BSS0][pAd->DefaultKeyId]);

	RTMPSetWcidSecurityInfo(pAd, 
							BSS0, 
							pAd->DefaultKeyId,
							pAd->SharedKey[BSS0][pAd->DefaultKeyId].CipherAlg,
							(UCHAR)aid, 
							PAIRWISEKEYTABLE);	
}


static BOOLEAN RTMPCheckMcast(
    IN PRTMP_ADAPTER    pAd,
    IN PEID_STRUCT      eid_ptr,
    IN PMGMTENTRY  		pEntry)
{

	if (!pEntry)
		return FALSE;

    pEntry->AuthMode = pAd->ApCfg.AuthMode;

    if (eid_ptr->Len >= 6)
    {
        /* WPA and WPA2 format not the same in RSN_IE */
        if (eid_ptr->Eid == IE_WPA)
        {
            if (pAd->ApCfg.AuthMode == Ndis802_11AuthModeWPA1WPA2)
                pEntry->AuthMode = Ndis802_11AuthModeWPA;
            else if (pAd->ApCfg.AuthMode == Ndis802_11AuthModeWPA1PSKWPA2PSK)
                pEntry->AuthMode = Ndis802_11AuthModeWPAPSK;

            if (NdisEqualMemory(&eid_ptr->Octet[6], &pAd->ApCfg.RSN_IE[0][6], 4))
                return TRUE;
        }
        else if (eid_ptr->Eid == IE_WPA2)
        {
            UCHAR   IE_Idx = 0;

            /* When WPA1/WPA2 mix mode, the RSN_IE is stored in different structure */
            if ((pAd->ApCfg.AuthMode == Ndis802_11AuthModeWPA1WPA2) || 
                (pAd->ApCfg.AuthMode == Ndis802_11AuthModeWPA1PSKWPA2PSK))
                IE_Idx = 1;
    
            if (pAd->ApCfg.AuthMode == Ndis802_11AuthModeWPA1WPA2)
                pEntry->AuthMode = Ndis802_11AuthModeWPA2;
            else if (pAd->ApCfg.AuthMode == Ndis802_11AuthModeWPA1PSKWPA2PSK)
                pEntry->AuthMode = Ndis802_11AuthModeWPA2PSK;

            if (NdisEqualMemory(&eid_ptr->Octet[2], &pAd->ApCfg.RSN_IE[IE_Idx][2], 4))
                return TRUE;
        }
    }

    return FALSE;
}



static BOOLEAN RTMPCheckUcast(
    IN PRTMP_ADAPTER    pAd,
    IN PEID_STRUCT      eid_ptr,
    IN PMGMTENTRY		pEntry)
{
	PUCHAR 	pStaTmp;
	USHORT	Count;

	if (!pEntry)
		return FALSE;

	pEntry->WepStatus = pAd->ApCfg.WepStatus;
    
	if (eid_ptr->Len < 16)
	{
		DBGPRINT(RT_ERROR, ("[ERR]%s : the length is too short(%d) \n", __FUNCTION__, eid_ptr->Len));
	    return FALSE;
	}	

	/* Store STA RSN_IE capability */
	pStaTmp = (PUCHAR)&eid_ptr->Octet[0];
	if(eid_ptr->Eid == IE_WPA2)
	{
		/* skip Version(2),Multicast cipter(4) 2+4==6 */
		/* point to number of unicast */
        pStaTmp += 6;
	}
	else if (eid_ptr->Eid == IE_WPA)	
	{
		/* skip OUI(4),Vesrion(2),Multicast cipher(4) 4+2+4==10 */
		/* point to number of unicast */
        pStaTmp += 10;
	}
	else
	{
		DBGPRINT(RT_ERROR, ("[ERR]%s : invalid IE=%d\n", __FUNCTION__, eid_ptr->Eid));
	    return FALSE;
	}

	/* Store unicast cipher count */
    NdisMoveMemory(&Count, pStaTmp, sizeof(USHORT));
    Count = cpu2le16(Count);		


	/* pointer to unicast cipher */
    pStaTmp += sizeof(USHORT);	
			
    if (eid_ptr->Len >= 16)
    {
    	if (eid_ptr->Eid == IE_WPA)
    	{
    		if (pAd->ApCfg.WepStatus == Ndis802_11Encryption4Enabled)
			{/* multiple cipher (TKIP/CCMP) */

				while (Count > 0)
				{
					/* TKIP */
					if (MIX_CIPHER_WPA_TKIP_ON(pAd->ApCfg.WpaMixPairCipher))
					{
						/* Compare if peer STA uses the TKIP as its unicast cipher */
						if (NdisEqualMemory(pStaTmp, &pAd->ApCfg.RSN_IE[0][12], 4))
						{
							pEntry->WepStatus = Ndis802_11Encryption2Enabled;
							return TRUE;
						}

						/* Our AP uses the AES as the secondary cipher */
						/* Compare if the peer STA use AES as its unicast cipher */
						if (MIX_CIPHER_WPA_AES_ON(pAd->ApCfg.WpaMixPairCipher))
						{
							if (NdisEqualMemory(pStaTmp, &pAd->ApCfg.RSN_IE[0][16], 4))
							{
								pEntry->WepStatus = Ndis802_11Encryption3Enabled;
								return TRUE;
							}
						}
					}
					else
					{
						/* AES */
						if (NdisEqualMemory(pStaTmp, &pAd->ApCfg.RSN_IE[0][12], 4))
						{
							pEntry->WepStatus = Ndis802_11Encryption3Enabled;
							return TRUE;
						}
					}
															
					pStaTmp += 4;
					Count--;
				}
    		}
    		else
    		{/* single cipher */
    			while (Count > 0)
    			{
    				if (NdisEqualMemory(pStaTmp , &pAd->ApCfg.RSN_IE[0][12], 4))
		            	return TRUE;

					pStaTmp += 4;
					Count--;
				}
    		}
    	}
    	else if (eid_ptr->Eid == IE_WPA2)
    	{
    		UCHAR	IE_Idx = 0;

			/* When WPA1/WPA2 mix mode, the RSN_IE is stored in different structure */
			if ((pAd->ApCfg.AuthMode == Ndis802_11AuthModeWPA1WPA2) || (pAd->ApCfg.AuthMode == Ndis802_11AuthModeWPA1PSKWPA2PSK))
				IE_Idx = 1;
	
			if (pAd->ApCfg.WepStatus == Ndis802_11Encryption4Enabled)
			{/* multiple cipher (TKIP/CCMP) */

				while (Count > 0)
    			{
    				/* WPA2 TKIP */
					if (MIX_CIPHER_WPA2_TKIP_ON(pAd->ApCfg.WpaMixPairCipher))
					{
						/* Compare if peer STA uses the TKIP as its unicast cipher */
						if (NdisEqualMemory(pStaTmp, &pAd->ApCfg.RSN_IE[IE_Idx][8], 4))
						{
							pEntry->WepStatus = Ndis802_11Encryption2Enabled;
							return TRUE;
						}

						/* Our AP uses the AES as the secondary cipher */
						/* Compare if the peer STA use AES as its unicast cipher */
						if (MIX_CIPHER_WPA2_AES_ON(pAd->ApCfg.WpaMixPairCipher))
						{
							if (NdisEqualMemory(pStaTmp, &pAd->ApCfg.RSN_IE[IE_Idx][12], 4))
							{
								pEntry->WepStatus = Ndis802_11Encryption3Enabled;
								return TRUE;
							}
						}
					}
					else
					{
						/* AES */
						if (NdisEqualMemory(pStaTmp, &pAd->ApCfg.RSN_IE[IE_Idx][8], 4))
						{
							pEntry->WepStatus = Ndis802_11Encryption3Enabled;
							return TRUE;
						}
					}
				    				
					pStaTmp += 4;
					Count--;
				}
			}
			else
			{/* single cipher */
				while (Count > 0)
    			{
					if (NdisEqualMemory(pStaTmp, &pAd->ApCfg.RSN_IE[IE_Idx][8], 4))
						return TRUE;

					pStaTmp += 4;
					Count--;
				}
			}
    	}
    }

    return FALSE;
}



static BOOLEAN RTMPCheckAKM(PUCHAR sta_akm, PUCHAR ap_rsn_ie, INT iswpa2)
{
	PUCHAR pTmp;
	USHORT Count;

	pTmp = ap_rsn_ie;

	if (iswpa2)
    /* skip Version(2),Multicast cipter(4) 2+4==6 */
        pTmp += 6;
    else	
    /*skip OUI(4),Vesrion(2),Multicast cipher(4) 4+2+4==10 */
        pTmp += 10;/*point to number of unicast */
	    
    NdisMoveMemory(&Count, pTmp, sizeof(USHORT));	
    Count = cpu2le16(Count);		

    pTmp   += sizeof(USHORT);/*pointer to unicast cipher */

    /* Skip all unicast cipher suite */
    while (Count > 0)
    	{
		/* Skip OUI */
		pTmp += 4;
		Count--;
	}

	NdisMoveMemory(&Count, pTmp, sizeof(USHORT));
    Count = cpu2le16(Count);		

    pTmp += sizeof(USHORT);/*pointer to AKM cipher */
    while (Count > 0)
    {
		/*rtmp_hexdump(RT_DEBUG_TRACE,"MBSS WPA_IE AKM ",pTmp,4); */
		if (NdisEqualMemory(sta_akm, pTmp, 4))
		   return TRUE;
    	else
		{
			pTmp += 4;
			Count--;
		}
    }
    return FALSE;/* do not match the AKM */

}



static BOOLEAN RTMPCheckAUTH(
    IN PRTMP_ADAPTER    pAd,
    IN PEID_STRUCT      eid_ptr,
    IN PMGMTENTRY		pEntry)
{
	PUCHAR pStaTmp;
	USHORT Count;	
	
	if (!pEntry)
		return FALSE;

	if (eid_ptr->Len < 16)
	{
		DBGPRINT(RT_ERROR, ("[ERR]%s ==> WPAIE len is too short(%d) \n", __FUNCTION__, eid_ptr->Len));
	    return FALSE;
	}	

	/* Store STA RSN_IE capability */
	pStaTmp = (PUCHAR)&eid_ptr->Octet[0];
	if(eid_ptr->Eid == IE_WPA2)
	{
		/* skip Version(2),Multicast cipter(4) 2+4==6 */
		/* point to number of unicast */
        pStaTmp += 6;
	}
	else if (eid_ptr->Eid == IE_WPA)	
	{
		/* skip OUI(4),Vesrion(2),Multicast cipher(4) 4+2+4==10 */
		/* point to number of unicast */
        pStaTmp += 10;
	}
	else
	{
		DBGPRINT(RT_ERROR, ("[ERR]%s ==> Unknown WPAIE, WPAIE=%d\n", __FUNCTION__, eid_ptr->Eid));
	    return FALSE;
	}

	/* Store unicast cipher count */
    NdisMoveMemory(&Count, pStaTmp, sizeof(USHORT));
    Count = cpu2le16(Count);		

	/* pointer to unicast cipher */
    pStaTmp += sizeof(USHORT);	

    /* Skip all unicast cipher suite */
    while (Count > 0)
    {
		/* Skip OUI */
		pStaTmp += 4;
		Count--;
	}

	/* Store AKM count */
	NdisMoveMemory(&Count, pStaTmp, sizeof(USHORT));
    Count = cpu2le16(Count);		

	/*pointer to AKM cipher */
    pStaTmp += sizeof(USHORT);			

    if (eid_ptr->Len >= 16)
    {
    	if (eid_ptr->Eid == IE_WPA)
    	{
			while (Count > 0)
			{
				if (RTMPCheckAKM(pStaTmp, &pAd->ApCfg.RSN_IE[0][0], 0))
					return TRUE;
	
				pStaTmp += 4;
				Count--;
			}
    	}
    	else if (eid_ptr->Eid == IE_WPA2)
    	{
    		UCHAR	IE_Idx = 0;

			/* When WPA1/WPA2 mix mode, the RSN_IE is stored in different structure */
			if ((pAd->ApCfg.AuthMode == Ndis802_11AuthModeWPA1WPA2) || 
				(pAd->ApCfg.AuthMode == Ndis802_11AuthModeWPA1PSKWPA2PSK))
				IE_Idx = 1;

			while (Count > 0)
			{			
				if (RTMPCheckAKM(pStaTmp, &pAd->ApCfg.RSN_IE[IE_Idx][0],1))
					return TRUE;
			
				pStaTmp += 4;
				Count--;
			}
    	}
    }

    return FALSE;
}


USHORT RTMPCheckValidateRSNIE(
	IN PRTMP_ADAPTER    pAd,
	IN PMGMTENTRY		pEntry,
	IN PUCHAR			pRsnIe,
	IN UCHAR			rsnie_len)
{
	USHORT 			StatusCode = SUCCESS;
	PEID_STRUCT  	eid_ptr;

	if (rsnie_len == 0)
		return MLME_SUCCESS;

	eid_ptr = (PEID_STRUCT)pRsnIe;
	if ((eid_ptr->Len + 2) != rsnie_len)
	{
		DBGPRINT(RT_ERROR, ("[ERR]%s: the len is invalid !!!\n", __FUNCTION__));
		return MLME_UNSPECIFY_FAIL;
	}

	/* check group cipher */
	if (!RTMPCheckMcast(pAd, eid_ptr, pEntry))
	{
		DBGPRINT(RT_ERROR, ("[ERR]%s : invalid group cipher !!!\n", __FUNCTION__));	    
	    StatusCode = MLME_INVALID_GROUP_CIPHER;
	}  
	/* Check pairwise cipher */
	else if (!RTMPCheckUcast(pAd, eid_ptr, pEntry))
	{
		DBGPRINT(RT_ERROR, ("[ERR]%s : invalid pairwise cipher !!!\n", __FUNCTION__));	    
	    StatusCode = MLME_INVALID_PAIRWISE_CIPHER;
	}        
	/* Check AKM */
	else if (!RTMPCheckAUTH(pAd, eid_ptr, pEntry))
	{
	    DBGPRINT(RT_ERROR, ("[ERR]%s : invalid AKM !!!\n", __FUNCTION__));
	    StatusCode = MLME_INVALID_AKMP;
	}                        

	if (StatusCode != MLME_SUCCESS)
	{
		/* send wireless event - for RSN IE sanity check fail */
		DBGPRINT(RT_ERROR, ("[ERR]%s : invalid status code(%d) !!!\n", __FUNCTION__, StatusCode));
	}
	else
	{
        UCHAR CipherAlg = CIPHER_NONE;
		
        if (pEntry->WepStatus == Ndis802_11Encryption1Enabled)
            CipherAlg = CIPHER_WEP64;
        else if (pEntry->WepStatus == Ndis802_11Encryption2Enabled)
            CipherAlg = CIPHER_TKIP;
        else if (pEntry->WepStatus == Ndis802_11Encryption3Enabled)
            CipherAlg = CIPHER_AES;
		
        DBGPRINT(RT_INFO, ("[INFO]%s : %02X:%02X:%02X:%02X:%02X:%02X, (AID#%d WepStatus=%s)\n",
							__FUNCTION__, PRINT_MAC(pEntry->Addr), pEntry->Aid, CipherName[CipherAlg]));
	}

	return StatusCode;
}


NDIS_STATUS RTMPApCheckRxError(
	IN PRTMP_ADAPTER 	pAd,
	IN PRXINFO_STRUC 	pRxInfo,
	IN UCHAR 			Wcid)
{
	if (pRxInfo->Crc || pRxInfo->CipherErr)
	{
		/*
			WCID equal to 255 mean MAC couldn't find any matched entry in Asic-MAC table.
			The incoming packet mays come from WDS or AP-Client link.
			We need them for further process. Can't drop the packet here.
		*/
		if ((pRxInfo->U2M) && (pRxInfo->CipherErr) && (Wcid == 255))
		{
			/* pass those packet for further process. */
			return NDIS_SUCCESS;
		}
		else
		{
			DBGPRINT(RT_WARN, ("[WARN]%s(): pRxInfo:Crc=%d, CipherErr=%d, U2M=%d, Wcid=%d\n", 
								__FUNCTION__, pRxInfo->Crc, pRxInfo->CipherErr, pRxInfo->U2M, Wcid));
			return NDIS_FAILURE;
		}
	}
	else
	{
		return NDIS_SUCCESS;
	}
}


VOID RTMPAPRxDErrorHandle(
	IN PRTMP_ADAPTER 	pAd, 
	IN PRXINFO_STRUC 	pRxInfo, 
	IN PRXWI_STRUC 		pRxWI)
{
	PMGMTENTRY pEntry;
/*	
	if (pRxInfo->CipherErr)
		INC_COUNTER64(pAd->WlanCounters.WEPUndecryptableCount);
*/
	if (pRxInfo->U2M && pRxInfo->CipherErr)
	{		
		if (pRxWI->wcid < MAX_LEN_OF_MAC_TABLE)
		{
			pEntry = MGMTENTRY_SearchNodeByWCID(pAd, pRxWI->wcid);
			/*
				MIC error
				Before verifying the MIC, the receiver shall check FCS, ICV and TSC.
				This avoids unnecessary MIC failure events.
			*/
			if ((pEntry->WepStatus == Ndis802_11Encryption2Enabled) && (pRxInfo->CipherErr == 2))
			{
				HandleCounterMeasure(pAd, pEntry);
			}
		}
		DBGPRINT(RT_TRACE, ("[TRACE]%s: Rx u2me Cipher Err(MPDUsize=%d, WCID=%d, CipherErr=%d)\n", 
							__FUNCTION__, pRxWI->MPDUtotalByteCnt, pRxWI->wcid, pRxInfo->CipherErr));
	}

	// pAd->Counters8023.RxErrors++;
	DBGPRINT(RT_TRACE, ("[TRACE]<---%s\n", __FUNCTION__));
}



NDIS_STATUS	RTMPStaCheckRxError(
	IN PRTMP_ADAPTER 	pAd,
	IN PHEADER_802_11 	pHeader,	
	IN PRXWI_STRUC 		pRxWI,	
	IN PRXINFO_STRUC 	pRxInfo)
{	
	PCIPHER_KEY pWpaKey;
	INT dBm;
	
	if (pRxInfo == NULL)
		return(NDIS_FAILURE);

	/* Phy errors & CRC errors*/
	if (pRxInfo->Crc)
	{
		/* Check RSSI for Noise Hist statistic collection.*/
		dBm = (INT) (pRxWI->RSSI0) - pAd->BbpRssiToDbmDelta;

		DBGPRINT(RT_TRACE, ("[TRACE]%s: dBm = %d\n", __FUNCTION__, dBm));
		
		if (dBm <= -87)
			pAd->StaCfg.RPIDensity[0] += 1;
		else if (dBm <= -82)
			pAd->StaCfg.RPIDensity[1] += 1;
		else if (dBm <= -77)
			pAd->StaCfg.RPIDensity[2] += 1;
		else if (dBm <= -72)
			pAd->StaCfg.RPIDensity[3] += 1;
		else if (dBm <= -67)
			pAd->StaCfg.RPIDensity[4] += 1;
		else if (dBm <= -62)
			pAd->StaCfg.RPIDensity[5] += 1;
		else if (dBm <= -57)
			pAd->StaCfg.RPIDensity[6] += 1;
		else if (dBm > -57)
			pAd->StaCfg.RPIDensity[7] += 1;
		
		return(NDIS_FAILURE);
	}

	/* Add Rx size to channel load counter, we should ignore error counts*/
//	pAd->StaCfg.CLBusyBytes += (pRxWI->MPDUtotalByteCnt + 14);

	/* Paul 04-03 for OFDM Rx length issue*/
	if (pRxWI->MPDUtotalByteCnt > MAX_AGGREGATION_SIZE)
	{
		DBGPRINT(RT_ERROR, ("[ERR]%s: received packet too long\n", __FUNCTION__));
		return NDIS_FAILURE;
	}

	/* Drop not U2M frames, cant's drop here because we will drop beacon in this case*/
	/* I am kind of doubting the U2M bit operation*/
	/* if (pRxD->U2M == 0)*/
	/*	return(NDIS_STATUS_FAILURE);*/

	/* drop decyption fail frame*/
	if (pRxInfo->Decrypted && pRxInfo->CipherErr)
	{
#if 0						
		if (((pRxInfo->CipherErr & 1) == 1) && INFRA_ON(pAd))
            RTMPSendWirelessEvent(pAd, IW_ICV_ERROR_EVENT_FLAG, pAd->MacTab.Content[BSSID_WCID].Addr, BSS0, 0);			

		if (((pRxInfo->CipherErr & 2) == 2) && INFRA_ON(pAd))
			RTMPSendWirelessEvent(pAd, IW_MIC_ERROR_EVENT_FLAG, pAd->MacTab.Content[BSSID_WCID].Addr, BSS0, 0);
#endif
		pWpaKey = &pAd->SharedKey[BSS0][pRxWI->key_idx];


		DBGPRINT(RT_WARN,("[WARN]%s: CipherErr = %d, MyBss = %d, key_idx = %d, key_type = %s\n",
							__FUNCTION__, pRxInfo->CipherErr, pRxInfo->MyBss, pRxWI->key_idx, (pWpaKey->Type == PAIRWISEKEY)?("Pair-Key"):("Group-Key")));
		/* MIC Error*/
		if ((pRxInfo->CipherErr == 2) && pRxInfo->MyBss)
		{
			DBGPRINT(RT_ERROR,("[ERR]%s: Rx MIC Value error in KeyIdx = %d (%s)\n", 
								__FUNCTION__, pRxWI->key_idx, (pWpaKey->Type == PAIRWISEKEY)?("Pair-Key"):("Group-Key")));
		}
#if 0		
		if (pRxInfo->Decrypted && (pWpaKey->CipherAlg == CIPHER_AES)) //&& (pHeader->Sequence == pAd->FragFrame.Sequence))
		{
			/* Acceptable since the First FragFrame no CipherErr problem.*/
			return(NDIS_SUCCESS);
		}
#endif
		pAd->StaCfg.CipherErrCnt++;
		if (pAd->StaCfg.CipherErrCnt >= MAX_CIPHER_ERR_CNT)
		{
			// reconnect to router
			SendDeAuthReq(pAd, pAd->StaCfg.RouterMac, &pAd->TxContext, REASON_CLS3ERR);
			MGMTENTRY_Del(pAd, pAd->StaCfg.RouterMac);
			MLME_DoDisconnect(pAd, TRUE);
			pAd->StaCfg.CipherErrCnt = 0;
		}
		return(NDIS_FAILURE);
	}

	return(NDIS_SUCCESS);
}


#if 0 // TKIP unused code..
/* For TKIP frame, calculate the MIC value */
BOOLEAN RTMPCheckTkipMICValue(
	IN	PRTMP_ADAPTER	pAd,
	IN	PHEADER_802_11	pWifiHdr,
	IN 	PUCHAR			pData,
	IN	UINT32			DataSize,
	IN	CHAR			UserPriority)
{
	PMGMTENTRY 		pEntry;
	PCIPHER_KEY		pWpaKey;
	
	pEntry = MGMTENTRY_GetNode(pAd, pWifiHdr->Addr2);
	if (!pEntry)
	{
		DBGPRINT(RT_ERROR, ("[ERR]%s: Cannot find entry node %02X-%02X-%02X-%02X-%02X-%02X\n", 
							__FUNCTION__, PRINT_MAC(pWifiHdr->Addr2)));
		return TRUE;
	}

	if (pEntry->GTKState != REKEY_ESTABLISHED)
		return TRUE;
	
	pWpaKey = &pEntry->PairwiseKey;
	
	if (RTMPTkipCompareMICValue(pAd,
								pData,
								pWifiHdr->Addr3,
								pWifiHdr->Addr2,
								pWpaKey->RxMic,
								(UCHAR)UserPriority,
								DataSize) == FALSE)
	{
		DBGPRINT(RT_ERROR,("[ERR]%s: Rx MIC Value error 2\n", __FUNCTION__));
		HandleCounterMeasure(pAd, pEntry);
		return FALSE;
	}

	return TRUE;
}
#endif


VOID RTMPGetCipherInfo(
	IN PUCHAR 		pIEData, 
	IN INT 			len, 
	OUT PUCHAR 		auth,		/* Auth mode*/ 
	OUT PUCHAR 		uenc,		/* Unicast enc mode */
	OUT PUCHAR		menc)		/* Multicast enc mode */
{
	UCHAR ie, ieLen;
	PUCHAR pVal;
	int done = 0;
	
	*auth = 0;
	*uenc = 0;
	*menc = 0;

	do
	{
//		UCHAR cipher_cnt = 0;
		ie = pIEData[0];
		ieLen = pIEData[1];
		pIEData += 2;

		switch (ie)
		{
			case IE_RSN:
				{
					pVal = pIEData;
					pVal += 2;

					if (pVal[0] == OUI_WPA2[0] && pVal[1] == OUI_WPA2[1] && pVal[2] == OUI_WPA2[2])
					{
						*auth = Ndis802_11AuthModeWPA2PSK;
					}
					else 
					{
						DBGPRINT(RT_ERROR, ("[ERR]%s: Warning Wrong RSN TYPE", __FUNCTION__));
						break;
					}
					
					/* Group cipher suite */
					if (pVal[3] == OUI_WPA2_WEP40[3] || pVal[3] == OUI_WPA2_WEP104[3])
						*menc = Ndis802_11Encryption1Enabled;
					else if (pVal[3] == OUI_WPA2_TKIP[3])
						*menc = Ndis802_11Encryption2Enabled;
					else if	(pVal[3] == OUI_WPA2_CCMP[3])
						*menc = Ndis802_11Encryption3Enabled;
					else {
						DBGPRINT(RT_ERROR, ("[ERR]%s: Warning Wrong MCast OUI (%d)", __FUNCTION__, pVal[3]));
						break;
					}
					pVal += 4;
//					cipher_cnt = *pVal;
					pVal += 2;

					/* find the first one unicast cipher */
					if (pVal[3] == OUI_WPA2_WEP40[3])
						*uenc = Ndis802_11Cipher_WEP40;
					else if (pVal[3] == OUI_WPA2_TKIP[3])
						*uenc = Ndis802_11Cipher_TKIP;
					else if	(pVal[3] == OUI_WPA2_CCMP[3])
						*uenc = Ndis802_11Cipher_CCMP;
					else if (pVal[3] == OUI_WPA2_WEP104[3])
						*uenc = Ndis802_11Cipher_WEP104;
					else {
						DBGPRINT(RT_ERROR, ("[ERR]%s: Warning Wrong UCast OUI (%d)", __FUNCTION__, pVal[3]));
						break;
					}					
				}
				done = 1;
				break;

			case IE_WPA:
				{
					pVal = pIEData;
					if (pVal[0] == OUI_WPA_VERSION[0] && pVal[1] == OUI_WPA_VERSION[1] && pVal[2] == OUI_WPA_VERSION[2] && pVal[3] == OUI_WPA_VERSION[3])
					{
						*auth = Ndis802_11AuthModeWPAPSK;
					}
					else
					{
						//DBGPRINT(RT_ERROR, ("[ERR]%s: Warning Wrong WPA TYPE", __FUNCTION__));
						break;
					}
					pVal += 4;
					/* version */
					pVal += 2;
					/* Mcast*/
					/* Group cipher suite */
					if (pVal[3] == OUI_WPA_WEP40[3] || pVal[3] == OUI_WPA_WEP40[3])
						*menc = Ndis802_11Encryption1Enabled;
					else if (pVal[3] == OUI_WPA_TKIP[3])
						*menc = Ndis802_11Encryption2Enabled;
					else if	(pVal[3] == OUI_WPA_CCMP[3])
						*menc = Ndis802_11Encryption3Enabled;
					else {
						DBGPRINT(RT_ERROR, ("[ERR]%s: Warning Wrong MCast OUI (%d)", __FUNCTION__, pVal[3]));
						break;
					}
					pVal += 4;
//					cipher_cnt = *pVal;
					pVal += 2;

					/* find the first one unicast cipher */
					if (pVal[3] == OUI_WPA_WEP40[3])
						*uenc = Ndis802_11Cipher_WEP40;
					else if (pVal[3] == OUI_WPA_TKIP[3])
						*uenc = Ndis802_11Cipher_TKIP;
					else if	(pVal[3] == OUI_WPA_CCMP[3])
						*uenc = Ndis802_11Cipher_CCMP;
					else if (pVal[3] == OUI_WPA_WEP104[3])
						*uenc = Ndis802_11Cipher_WEP104;
					else {
						DBGPRINT(RT_ERROR, ("[ERR]%s: Warning Wrong UCast OUI (%d)", __FUNCTION__, pVal[3]));
						break;
					}
					done = 1;
				}
				break;
				
			default:
				break;
		}

		if (done)
			break;

		pIEData += ieLen;
		len -= (2+ieLen);
	} while (len);


	if (done == 0)
	{
		*auth = Ndis802_11AuthModeOpen;
		*uenc =Ndis802_11Cipher_WEP40;
	}
}
