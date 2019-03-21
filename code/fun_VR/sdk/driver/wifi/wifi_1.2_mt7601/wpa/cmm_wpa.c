#include "cmm_wpa.h"
#include "rtmp.h"
#include "cipher.h"
#include "mgmtentry.h"
#include "crypt/crypt_aes.h"
#include "crypt/crypt_arc4.h"
#include "crypt/crypt_hmac.h"
#include "crypt/crypt_md5.h"
#include "crypt/crypt_sha2.h"
#include "tx.h"
#include "nl80211.h"
#include "asic_ctrl.h"
#include "usb_io.h"
#include "mlme.h"

/* WPA OUI*/
UCHAR		OUI_WPA[3]				= {0x00, 0x50, 0xF2};
UCHAR		OUI_WPA_NONE_AKM[4]		= {0x00, 0x50, 0xF2, 0x00};
UCHAR       OUI_WPA_VERSION[4]      = {0x00, 0x50, 0xF2, 0x01};
UCHAR       OUI_WPA_WEP40[4]      	= {0x00, 0x50, 0xF2, 0x01};
UCHAR       OUI_WPA_TKIP[4]     	= {0x00, 0x50, 0xF2, 0x02};
UCHAR       OUI_WPA_CCMP[4]     	= {0x00, 0x50, 0xF2, 0x04};
UCHAR       OUI_WPA_WEP104[4]     	= {0x00, 0x50, 0xF2, 0x05};
UCHAR       OUI_WPA_8021X_AKM[4]	= {0x00, 0x50, 0xF2, 0x01};
UCHAR       OUI_WPA_PSK_AKM[4]      = {0x00, 0x50, 0xF2, 0x02};
/* WPA2 OUI*/
UCHAR		OUI_WPA2[3]				= {0x00, 0x0F, 0xAC};
UCHAR       OUI_WPA2_WEP40[4]   	= {0x00, 0x0F, 0xAC, 0x01};
UCHAR       OUI_WPA2_TKIP[4]        = {0x00, 0x0F, 0xAC, 0x02};
UCHAR       OUI_WPA2_CCMP[4]        = {0x00, 0x0F, 0xAC, 0x04};
UCHAR       OUI_WPA2_8021X_AKM[4]   = {0x00, 0x0F, 0xAC, 0x01};
UCHAR       OUI_WPA2_PSK_AKM[4]   	= {0x00, 0x0F, 0xAC, 0x02};
UCHAR       OUI_WPA2_WEP104[4]   	= {0x00, 0x0F, 0xAC, 0x05};
UCHAR       OUI_WPA2_1X_SHA256[4]   = {0x00, 0x0F, 0xAC, 0x05};
UCHAR       OUI_WPA2_PSK_SHA256[4]  = {0x00, 0x0F, 0xAC, 0x06};

#define TX_EAPOL_BUFFER			1500



VOID hex_dump(
	IN PUCHAR 	title, 
	IN PUCHAR 	value, 
	IN int 		size)
{
	int i;
	
	if (value == NULL || size == 0)
		return;
	DBGPRINT(RT_ERROR, ("\n\n>>>> %s hex_dump %d\n", title, size));
	
	for (i = 0; i < size; i++) {
		DBGPRINT(RT_ERROR, ("0x%2x ", *(value+i)));
	}
	DBGPRINT(RT_ERROR, ("\n\n"));
}


/*
	========================================================================
	
	Routine Description:
		Compare two memory block

	Arguments:
		pSrc1		Pointer to first memory address
		pSrc2		Pointer to second memory address
		
	Return Value:
		0:			memory is equal
		1:			pSrc1 memory is larger
		2:			pSrc2 memory is larger

	IRQL = DISPATCH_LEVEL
	
	Note:
		
	========================================================================
*/
ULONG	RTMPCompareMemory(
	IN	PVOID	pSrc1,
	IN	PVOID	pSrc2,
	IN	ULONG	Length)
{
	PUCHAR	pMem1;
	PUCHAR	pMem2;
	ULONG	Index = 0;

	pMem1 = (PUCHAR) pSrc1;
	pMem2 = (PUCHAR) pSrc2;

	for (Index = 0; Index < Length; Index++)
	{
		if (pMem1[Index] > pMem2[Index])
			return (1);
		else if (pMem1[Index] < pMem2[Index])
			return (2);
	}

	/* Equal*/
	return (0);
}


/*
	========================================================================

	Routine Description:
		The pseudo-random function(PRF) that hashes various inputs to 
		derive a pseudo-random value. To add liveness to the pseudo-random 
		value, a nonce should be one of the inputs.

		It is used to generate PTK, GTK or some specific random value.  

	Arguments:
		UCHAR	*key,		-	the key material for HMAC_SHA1 use
		INT		key_len		-	the length of key
		UCHAR	*prefix		-	a prefix label
		INT		prefix_len	-	the length of the label
		UCHAR	*data		-	a specific data with variable length		
		INT		data_len	-	the length of a specific data	
		INT		len			-	the output lenght

	Return Value:
		UCHAR	*output		-	the calculated result 

	Note:
		802.11i-2004	Annex H.3

	========================================================================
*/
VOID	PRF(
	IN	UCHAR	*key,
	IN	INT		key_len,
	IN	UCHAR	*prefix,
	IN	INT		prefix_len,
	IN	UCHAR	*data,
	IN	INT		data_len,
	OUT	UCHAR	*output,
	IN	INT		len)
{
	INT		i;
    UCHAR   *input;
	INT		currentindex = 0;
	INT		total_len;

	/* Allocate memory for input*/
	input = (PUCHAR)pvPortMalloc(1024);
    if (input == NULL)
    {
        DBGPRINT(RT_ERROR, ("!!!PRF: no memory!!!\n"));
        return;
    }
	
	/* Generate concatenation input*/
	NdisMoveMemory(input, prefix, prefix_len);

	/* Concatenate a single octet containing 0*/
	input[prefix_len] =	0;

	/* Concatenate specific data*/
	NdisMoveMemory(&input[prefix_len + 1], data, data_len);
	total_len =	prefix_len + 1 + data_len;

	/* Concatenate a single octet containing 0*/
	/* This octet shall be update later*/
	input[total_len] = 0;
	total_len++;

	/* Iterate to calculate the result by hmac-sha-1*/
	/* Then concatenate to last result*/
	for	(i = 0;	i <	(len + 19) / 20; i++)
	{	
		RT_HMAC_SHA1(key, key_len, input, total_len, &output[currentindex], SHA1_DIGEST_SIZE);
		currentindex +=	20;

		/* update the last octet */
		input[total_len - 1]++;
	}
	
    vPortFree(input);
}


/*
	========================================================================
	
	Routine Description:
		Generate random number by software.

	Arguments:
		pAd		-	pointer to our pAdapter context 
		macAddr	-	pointer to local MAC address
		
	Return Value:

	Note:
		802.1ii-2004  Annex H.5
		
	========================================================================
*/
VOID GenRandom(
	IN	PRTMP_ADAPTER	pAd, 
	IN	UCHAR			*macAddr,
	OUT	UCHAR			*random)
{	
	INT		i, curr;
	UCHAR	local[80], KeyCounter[32];
	UCHAR	result[80];
	ULONG	CurrentTime;
	UCHAR	prefix[] = {'I', 'n', 'i', 't', ' ', 'C', 'o', 'u', 'n', 't', 'e', 'r'};

	/* Zero the related information*/
	NdisZeroMemory(result, 80);
	NdisZeroMemory(local, 80);
	NdisZeroMemory(KeyCounter, 32);	

	for	(i = 0;	i <	32;	i++)
	{		
		/* copy the local MAC address*/
		COPY_MAC_ADDR(local, macAddr);
		curr =	MAC_ADDR_LEN;

		/* concatenate the current time*/
//		isGetSystemUpTime(&CurrentTime);
		CurrentTime = (ULONG)xTaskGetTickCount();

		NdisMoveMemory(&local[curr],  &CurrentTime,	sizeof(CurrentTime));
		curr +=	sizeof(CurrentTime);

		/* concatenate the last result*/
		NdisMoveMemory(&local[curr],  result, 32);
		curr +=	32;
		
		/* concatenate a variable */
		NdisMoveMemory(&local[curr],  &i,  2);		
		curr +=	2;

		/* calculate the result*/
		PRF(KeyCounter, 32, prefix,12, local, curr, result, 32); 
	}
	
	NdisMoveMemory(random, result,	32);	
}


VOID WpaDeriveGTK(
    IN  UCHAR   *GMK,
    IN  UCHAR   *GNonce,
    IN  UCHAR   *AA,
    OUT UCHAR   *output,
    IN  UINT    len)
{
    UCHAR   concatenation[76];
    UINT    CurrPos = 0;
    UCHAR   Prefix[19];
    UCHAR   temp[80];   

    NdisMoveMemory(&concatenation[CurrPos], AA, 6);
    CurrPos += 6;

    NdisMoveMemory(&concatenation[CurrPos], GNonce , 32);
    CurrPos += 32;

    Prefix[0] = 'G';
    Prefix[1] = 'r';
    Prefix[2] = 'o';
    Prefix[3] = 'u';
    Prefix[4] = 'p';
    Prefix[5] = ' ';
    Prefix[6] = 'k';
    Prefix[7] = 'e';
    Prefix[8] = 'y';
    Prefix[9] = ' ';
    Prefix[10] = 'e';
    Prefix[11] = 'x';
    Prefix[12] = 'p';
    Prefix[13] = 'a';
    Prefix[14] = 'n';
    Prefix[15] = 's';
    Prefix[16] = 'i';
    Prefix[17] = 'o';
    Prefix[18] = 'n';

    PRF(GMK, LEN_PMK, Prefix,  19, concatenation, 38 , temp, len);
    NdisMoveMemory(output, temp, len);
	
//	hex_dump((PUCHAR)"GTK =", output, len);

}


/*
	========================================================================
	
	Routine Description:
		It utilizes PRF-384 or PRF-512 to derive session-specific keys from a PMK.
		It shall be called by 4-way handshake processing.

	Arguments:
		pAd 	-	pointer to our pAdapter context
		PMK		-	pointer to PMK
		ANonce	-	pointer to ANonce
		AA		-	pointer to Authenticator Address
		SNonce	-	pointer to SNonce
		SA		-	pointer to Supplicant Address		
		len		-	indicate the length of PTK (octet)		
		
	Return Value:
		Output		pointer to the PTK

	Note:
		Refer to IEEE 802.11i-2004 8.5.1.2
		
	========================================================================
*/
VOID WpaDerivePTK(
	IN	PRTMP_ADAPTER	pAd, 
	IN	UCHAR	*PMK,
	IN	UCHAR	*ANonce,
	IN	UCHAR	*AA,
	IN	UCHAR	*SNonce,
	IN	UCHAR	*SA,
	OUT	UCHAR	*output,
	IN	UINT	len)
{	
	UCHAR	concatenation[76];
	UINT	CurrPos = 0;
	UCHAR	temp[32];
	UCHAR	Prefix[] = {'P', 'a', 'i', 'r', 'w', 'i', 's', 'e', ' ', 'k', 'e', 'y', ' ', 
						'e', 'x', 'p', 'a', 'n', 's', 'i', 'o', 'n'};

	/* initiate the concatenation input*/
	NdisZeroMemory(temp, sizeof(temp));
	NdisZeroMemory(concatenation, 76);

	/* Get smaller address*/
	if (RTMPCompareMemory(SA, AA, 6) == 1)
		NdisMoveMemory(concatenation, AA, 6);
	else
		NdisMoveMemory(concatenation, SA, 6);
	CurrPos += 6;

	/* Get larger address*/
	if (RTMPCompareMemory(SA, AA, 6) == 1)
		NdisMoveMemory(&concatenation[CurrPos], SA, 6);
	else
		NdisMoveMemory(&concatenation[CurrPos], AA, 6);
		
	/* store the larger mac address for backward compatible of */
	/* ralink proprietary STA-key issue		*/
	NdisMoveMemory(temp, &concatenation[CurrPos], MAC_ADDR_LEN);		
	CurrPos += 6;

	/* Get smaller Nonce*/
	if (RTMPCompareMemory(ANonce, SNonce, 32) == 0)
		NdisMoveMemory(&concatenation[CurrPos], temp, 32);	/* patch for ralink proprietary STA-key issue*/
	else if (RTMPCompareMemory(ANonce, SNonce, 32) == 1)
		NdisMoveMemory(&concatenation[CurrPos], SNonce, 32);
	else
		NdisMoveMemory(&concatenation[CurrPos], ANonce, 32);
	CurrPos += 32;

	/* Get larger Nonce*/
	if (RTMPCompareMemory(ANonce, SNonce, 32) == 0)
		NdisMoveMemory(&concatenation[CurrPos], temp, 32);	/* patch for ralink proprietary STA-key issue*/
	else if (RTMPCompareMemory(ANonce, SNonce, 32) == 1)
		NdisMoveMemory(&concatenation[CurrPos], ANonce, 32);
	else
		NdisMoveMemory(&concatenation[CurrPos], SNonce, 32);
	CurrPos += 32;

//	hex_dump((PUCHAR)"PMK", PMK, LEN_PMK);
//	hex_dump((PUCHAR)"concatenation=", concatenation, 76);

	/* Use PRF to generate PTK*/
	PRF(PMK, LEN_PMK, Prefix, 22, concatenation, 76, output, len);

}


/*
	========================================================================
	
	Routine Description:
		Build cipher suite in RSN-IE. 
		It only shall be called by RTMPMakeRSNIE. 

	Arguments:
		pAd			-	pointer to our pAdapter context	
    	ElementID	-	indicate the WPA1 or WPA2
    	WepStatus	-	indicate the encryption type
		bMixCipher	-	a boolean to indicate the pairwise cipher and group 
						cipher are the same or not
		
	Return Value:
		
	Note:
		
	========================================================================
*/
static VOID RTMPMakeRsnIeCipher(
	IN  PRTMP_ADAPTER   pAd,
	IN	UCHAR			ElementID,	
	IN	UINT			WepStatus,
	IN	UCHAR			apidx,
	IN	BOOLEAN			bMixCipher,
	IN	UCHAR			FlexibleCipher,
	OUT	PUCHAR			pRsnIe,
	OUT	UCHAR			*rsn_len)
{		
	UCHAR	PairwiseCnt;

	*rsn_len = 0;

	/* decide WPA2 or WPA1	*/
	if (ElementID == IE_WPA2)
	{
		RSNIE2 *pRsnie_cipher = (RSNIE2 *)pRsnIe;

		/* Assign the verson as 1*/
		pRsnie_cipher->version = 1;

        switch (WepStatus)
        {
        	/* TKIP mode*/
            case Ndis802_11Encryption2Enabled:
                NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA2_TKIP, 4);
                pRsnie_cipher->ucount = 1;
                NdisMoveMemory(pRsnie_cipher->ucast[0].oui, OUI_WPA2_TKIP, 4);
                *rsn_len = sizeof(RSNIE2);
                break;

			/* AES mode*/
            case Ndis802_11Encryption3Enabled:
				if (bMixCipher)
					NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA2_TKIP, 4);
				else
					NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA2_CCMP, 4);
                pRsnie_cipher->ucount = 1;
                NdisMoveMemory(pRsnie_cipher->ucast[0].oui, OUI_WPA2_CCMP, 4);
                *rsn_len = sizeof(RSNIE2);
                break;

			/* TKIP-AES mix mode*/
            case Ndis802_11Encryption4Enabled:
                NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA2_TKIP, 4);

				PairwiseCnt = 1;
				/* Insert WPA2 TKIP as the first pairwise cipher */
				if (MIX_CIPHER_WPA2_TKIP_ON(FlexibleCipher))
				{
                	NdisMoveMemory(pRsnie_cipher->ucast[0].oui, OUI_WPA2_TKIP, 4);
					/* Insert WPA2 AES as the secondary pairwise cipher*/
					if (MIX_CIPHER_WPA2_AES_ON(FlexibleCipher))
					{
						NdisMoveMemory(pRsnIe + sizeof(RSNIE2), OUI_WPA2_CCMP, 4);
						PairwiseCnt = 2;
					}	
				}
				else
				{
					/* Insert WPA2 AES as the first pairwise cipher */
					NdisMoveMemory(pRsnie_cipher->ucast[0].oui, OUI_WPA2_CCMP, 4);	
				}
							
                pRsnie_cipher->ucount = PairwiseCnt;				
                *rsn_len = sizeof(RSNIE2) + (4 * (PairwiseCnt - 1));
                break;			
        }   

		if ((pAd->OpMode == OPMODE_STA) &&
			(pAd->StaCfg.GroupCipher != Ndis802_11Encryption2Enabled) &&
			(pAd->StaCfg.GroupCipher != Ndis802_11Encryption3Enabled)
			)
		{
			UINT	GroupCipher = pAd->StaCfg.GroupCipher;
			switch(GroupCipher)
			{
				case Ndis802_11GroupWEP40Enabled:
					NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA2_WEP40, 4);
					break;
				case Ndis802_11GroupWEP104Enabled:
					NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA2_WEP104, 4);
					break;
			}
		}

		/* swap for big-endian platform*/
		pRsnie_cipher->version = cpu2le16(pRsnie_cipher->version);
	    pRsnie_cipher->ucount = cpu2le16(pRsnie_cipher->ucount);
	}
	else
	{
		RSNIE *pRsnie_cipher = (RSNIE*)pRsnIe;
		 
		/* Assign OUI and version*/
		NdisMoveMemory(pRsnie_cipher->oui, OUI_WPA_VERSION, 4);
        pRsnie_cipher->version = 1;

		switch (WepStatus)
		{
			/* TKIP mode*/
            case Ndis802_11Encryption2Enabled:
                NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA_TKIP, 4);
                pRsnie_cipher->ucount = 1;
                NdisMoveMemory(pRsnie_cipher->ucast[0].oui, OUI_WPA_TKIP, 4);
                *rsn_len = sizeof(RSNIE);
                break;

			/* AES mode*/
            case Ndis802_11Encryption3Enabled:
				if (bMixCipher)
					NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA_TKIP, 4);
				else
					NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA_CCMP, 4);			
                pRsnie_cipher->ucount = 1;
                NdisMoveMemory(pRsnie_cipher->ucast[0].oui, OUI_WPA_CCMP, 4);
                *rsn_len = sizeof(RSNIE);
                break;

			/* TKIP-AES mix mode*/
            case Ndis802_11Encryption4Enabled:
                NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA_TKIP, 4);

				PairwiseCnt = 1;
				/* Insert WPA TKIP as the first pairwise cipher */
				if (MIX_CIPHER_WPA_TKIP_ON(FlexibleCipher))
				{
                	NdisMoveMemory(pRsnie_cipher->ucast[0].oui, OUI_WPA_TKIP, 4);
					/* Insert WPA AES as the secondary pairwise cipher*/
					if (MIX_CIPHER_WPA_AES_ON(FlexibleCipher))
					{
						NdisMoveMemory(pRsnIe + sizeof(RSNIE), OUI_WPA_CCMP, 4);
						PairwiseCnt = 2;
					}	
				}
				else
				{
					/* Insert WPA AES as the first pairwise cipher */
					NdisMoveMemory(pRsnie_cipher->ucast[0].oui, OUI_WPA_CCMP, 4);	
				}
						
                pRsnie_cipher->ucount = PairwiseCnt;				
                *rsn_len = sizeof(RSNIE) + (4 * (PairwiseCnt - 1));				
                break;					
        }

		if ((pAd->OpMode == OPMODE_STA) &&
			(pAd->StaCfg.GroupCipher != Ndis802_11Encryption2Enabled) &&
			(pAd->StaCfg.GroupCipher != Ndis802_11Encryption3Enabled)
			)
		{
			UINT	GroupCipher = pAd->StaCfg.GroupCipher;
			switch(GroupCipher)
			{
				case Ndis802_11GroupWEP40Enabled:
					NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA_WEP40, 4);
					break;
				case Ndis802_11GroupWEP104Enabled:
					NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA_WEP104, 4);
					break;
			}
		}

		/* swap for big-endian platform*/
		pRsnie_cipher->version = cpu2le16(pRsnie_cipher->version);
	    pRsnie_cipher->ucount = cpu2le16(pRsnie_cipher->ucount);
	}
}

/*
	========================================================================
	
	Routine Description:
		Build AKM suite in RSN-IE. 
		It only shall be called by RTMPMakeRSNIE. 

	Arguments:
		pAd			-	pointer to our pAdapter context	
    	ElementID	-	indicate the WPA1 or WPA2
    	AuthMode	-	indicate the authentication mode
		apidx		-	indicate the interface index
		
	Return Value:
		
	Note:
		
	========================================================================
*/
static VOID RTMPMakeRsnIeAKM(	
	IN  PRTMP_ADAPTER   pAd,	
	IN	UCHAR			ElementID,	
	IN	UINT			AuthMode,
	IN	UCHAR			apidx,
	OUT	PUCHAR			pRsnIe,
	OUT	UCHAR			*rsn_len)
{
	RSNIE_AUTH		*pRsnie_auth;	
	UCHAR			AkmCnt = 1;		/* default as 1*/

	pRsnie_auth = (RSNIE_AUTH*)(pRsnIe + (*rsn_len));

	/* decide WPA2 or WPA1	 */
	if (ElementID == IE_WPA2)
	{

		switch (AuthMode)
        {
            case Ndis802_11AuthModeWPA2:
            case Ndis802_11AuthModeWPA1WPA2:
                	NdisMoveMemory(pRsnie_auth->auth[0].oui, OUI_WPA2_8021X_AKM, 4);
                	break;

            case Ndis802_11AuthModeWPA2PSK:
            case Ndis802_11AuthModeWPA1PSKWPA2PSK:
                	NdisMoveMemory(pRsnie_auth->auth[0].oui, OUI_WPA2_PSK_AKM, 4);
                	break;
			default:
				AkmCnt = 0;
				break;
				
        }
	}
	else
	{
		switch (AuthMode)
        {
            case Ndis802_11AuthModeWPA:
            case Ndis802_11AuthModeWPA1WPA2:
                NdisMoveMemory(pRsnie_auth->auth[0].oui, OUI_WPA_8021X_AKM, 4);
                break;

            case Ndis802_11AuthModeWPAPSK:
            case Ndis802_11AuthModeWPA1PSKWPA2PSK:
                NdisMoveMemory(pRsnie_auth->auth[0].oui, OUI_WPA_PSK_AKM, 4);
                break;

			case Ndis802_11AuthModeWPANone:
                NdisMoveMemory(pRsnie_auth->auth[0].oui, OUI_WPA_NONE_AKM, 4);
                break;
			default:
				AkmCnt = 0;
				break;	
        }			
	}
		 
	pRsnie_auth->acount = AkmCnt;
	pRsnie_auth->acount = cpu2le16(pRsnie_auth->acount);
	
	/* update current RSNIE length*/
	(*rsn_len) += (sizeof(RSNIE_AUTH) + (4 * (AkmCnt - 1)));	

}


/*
	========================================================================
	
	Routine Description:
		Build capability in RSN-IE. 
		It only shall be called by RTMPMakeRSNIE. 

	Arguments:
		pAd			-	pointer to our pAdapter context	
    	ElementID	-	indicate the WPA1 or WPA2    	
		apidx		-	indicate the interface index
		
	Return Value:
		
	Note:
		
	========================================================================
*/
static VOID RTMPMakeRsnIeCap(	
	IN  PRTMP_ADAPTER   pAd,	
	IN	UCHAR			ElementID,
	IN	UCHAR			apidx,
	OUT	PUCHAR			pRsnIe,
	OUT	UCHAR			*rsn_len)
{
	RSN_CAPABILITIES    *pRSN_Cap;

	/* it could be ignored in WPA1 mode*/
	if (ElementID == IE_WPA)
		return;
	
	pRSN_Cap = (RSN_CAPABILITIES*)(pRsnIe + (*rsn_len));
#if 0	
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if (apidx < pAd->ApCfg.BssidNum)		
		{
			PAP_ADMIN_CONFIG pApCfg = &pAd->ApCfg;

#ifdef DOT1X_SUPPORT
        	pRSN_Cap->field.PreAuth = (pApCfg->PreAuth == TRUE) ? 1 : 0;
#endif /* DOT1X_SUPPORT */
		}
	}
#endif					 
	pRSN_Cap->word = cpu2le16(pRSN_Cap->word);
	
	(*rsn_len) += sizeof(RSN_CAPABILITIES);	/* update current RSNIE length*/

}


/*
	========================================================================
	
	Routine Description:
		Build PMKID in RSN-IE. 
		It only shall be called by RTMPMakeRSNIE. 

	Arguments:
		pAd			-	pointer to our pAdapter context	
    	ElementID	-	indicate the WPA1 or WPA2    	
		apidx		-	indicate the interface index
		
	Return Value:
		
	Note:
		
	========================================================================
*/

/*
	========================================================================

	Routine Description:
		Build RSN IE context. It is not included element-ID and length.

	Arguments:
		pAd			-	pointer to our pAdapter context	
    	AuthMode	-	indicate the authentication mode
    	WepStatus	-	indicate the encryption type
		apidx		-	indicate the interface index
		
	Return Value:
		
	Note:
		
	========================================================================
*/
VOID RTMPMakeRSNIE(
    IN  PRTMP_ADAPTER   pAd,
    IN  UINT            AuthMode,
    IN  UINT            WepStatus,
	IN	UCHAR			apidx)
{
	PUCHAR		pRsnIe = NULL;			/* primary RSNIE*/
	UCHAR 		*rsnielen_cur_p = 0;	/* the length of the primary RSNIE 		*/
	PUCHAR		pRsnIe_ex = NULL;		/* secondary RSNIE, it's ONLY used in WPA-mix mode */
	BOOLEAN 	bMixRsnIe = FALSE;      /* indicate WPA-mix mode is on or off*/
	UCHAR		s_offset;
	UCHAR		*rsnielen_ex_cur_p = 0;	/* the length of the secondary RSNIE	  	*/
	UCHAR		PrimaryRsnie;			
	BOOLEAN		bMixCipher = FALSE;		/* indicate the pairwise and group cipher are different*/
	UCHAR		p_offset;		
	WPA_MIX_PAIR_CIPHER		FlexibleCipher = MIX_CIPHER_NOTUSE;	/* it provide the more flexible cipher combination in WPA-WPA2 and TKIPAES mode*/
		
	rsnielen_cur_p = NULL;
	rsnielen_ex_cur_p = NULL;

	do
	{
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			DBGPRINT(RT_LOUD,("[LOUD]==> %s(AP)\n", __FUNCTION__));
			/* Sanity check for apidx */
			MBSS_MR_APIDX_SANITY_CHECK(pAd, apidx);

			if ((AuthMode != Ndis802_11AuthModeWPA) && 
				(AuthMode != Ndis802_11AuthModeWPAPSK) && 
				(AuthMode != Ndis802_11AuthModeWPA2) && 
				(AuthMode != Ndis802_11AuthModeWPA2PSK) && 
				(AuthMode != Ndis802_11AuthModeWPA1WPA2) && 
				(AuthMode != Ndis802_11AuthModeWPA1PSKWPA2PSK))
				return;

			/* decide the group key encryption type */
			if (WepStatus == Ndis802_11Encryption4Enabled)
			{
				pAd->ApCfg.GroupKeyWepStatus = Ndis802_11Encryption2Enabled;
				FlexibleCipher = pAd->ApCfg.WpaMixPairCipher;
			}
			else
				pAd->ApCfg.GroupKeyWepStatus = (NDIS_802_11_WEP_STATUS)WepStatus;

			/* Initiate some related information */
			pAd->ApCfg.RSNIE_Len[0] = 0;
			pAd->ApCfg.RSNIE_Len[1] = 0;
			NdisZeroMemory(pAd->ApCfg.RSN_IE[0], MAX_LEN_OF_RSNIE);
			NdisZeroMemory(pAd->ApCfg.RSN_IE[1], MAX_LEN_OF_RSNIE);
			/* Pointer to the first RSNIE context */
			rsnielen_cur_p = &pAd->ApCfg.RSNIE_Len[0];
			pRsnIe = pAd->ApCfg.RSN_IE[0];

			/* Pointer to the secondary RSNIE context */
			rsnielen_ex_cur_p = &pAd->ApCfg.RSNIE_Len[1];
			pRsnIe_ex = pAd->ApCfg.RSN_IE[1];

			/* Decide whether the authentication mode is WPA1-WPA2 mixed mode */
			if ((AuthMode == Ndis802_11AuthModeWPA1WPA2) || 
				(AuthMode == Ndis802_11AuthModeWPA1PSKWPA2PSK))
			{
				bMixRsnIe = TRUE;
			}
			break;
		}
		else IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			/* Support WPAPSK or WPA2PSK in STA-Infra mode */
			/* Support WPANone in STA-Adhoc mode */
			if ((AuthMode != Ndis802_11AuthModeWPA) && 
				(AuthMode != Ndis802_11AuthModeWPAPSK) && 
				(AuthMode != Ndis802_11AuthModeWPA2) && 
				(AuthMode != Ndis802_11AuthModeWPA2PSK) && 
				(AuthMode != Ndis802_11AuthModeWPA1WPA2) && 
				(AuthMode != Ndis802_11AuthModeWPA1PSKWPA2PSK))
				return;

			DBGPRINT(RT_LOUD,("[LOUD]==> %s(STA)\n", __FUNCTION__));

			/* Zero RSNIE context */
			pAd->StaCfg.RSNIE_Len = 0;
			NdisZeroMemory(pAd->StaCfg.RSN_IE, MAX_LEN_OF_RSNIE);

			/* Pointer to RSNIE */
			rsnielen_cur_p = &pAd->StaCfg.RSNIE_Len;
			pRsnIe = pAd->StaCfg.RSN_IE;

			//bMixCipher = pAd->StaCfg.bMixCipher;
			break;
		}

	} while(FALSE);

	/* indicate primary RSNIE as WPA or WPA2*/
	if ((AuthMode == Ndis802_11AuthModeWPA) || 
		(AuthMode == Ndis802_11AuthModeWPAPSK) || 
		(AuthMode == Ndis802_11AuthModeWPANone) || 
		(AuthMode == Ndis802_11AuthModeWPA1WPA2) || 
		(AuthMode == Ndis802_11AuthModeWPA1PSKWPA2PSK))
		PrimaryRsnie = IE_WPA;
	else
		PrimaryRsnie = IE_WPA2;
	
	{
		/* Build the primary RSNIE*/
		/* 1. insert cipher suite*/
		RTMPMakeRsnIeCipher(pAd, PrimaryRsnie, WepStatus, apidx, bMixCipher, FlexibleCipher, pRsnIe, &p_offset);

		/* 2. insert AKM*/
		RTMPMakeRsnIeAKM(pAd, PrimaryRsnie, AuthMode, apidx, pRsnIe, &p_offset);

		/* 3. insert capability*/
		RTMPMakeRsnIeCap(pAd, PrimaryRsnie, apidx, pRsnIe, &p_offset);
	}

	/* 4. update the RSNIE length*/
	if (rsnielen_cur_p == NULL)
	{
		DBGPRINT(RT_ERROR, ("%s: rsnielen_cur_p == NULL!\n", __FUNCTION__));
		return;
	}

	*rsnielen_cur_p = p_offset; 

//	hex_dump((PUCHAR)"The primary RSNIE", pRsnIe, (int)(*rsnielen_cur_p));

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		/* if necessary, build the secondary RSNIE*/
		if (bMixRsnIe)
		{
			/* 1. insert cipher suite*/
			RTMPMakeRsnIeCipher(pAd, IE_WPA2, WepStatus, apidx, bMixCipher, FlexibleCipher, pRsnIe_ex, &s_offset);

			/* 2. insert AKM*/
			RTMPMakeRsnIeAKM(pAd, IE_WPA2, AuthMode, apidx, pRsnIe_ex, &s_offset);

			/* 3. insert capability*/
			RTMPMakeRsnIeCap(pAd, IE_WPA2, apidx, pRsnIe_ex, &s_offset);


			/* Update the RSNIE length*/
			*rsnielen_ex_cur_p = s_offset; 

//			hex_dump((PUCHAR)"The secondary RSNIE", pRsnIe_ex, (*rsnielen_ex_cur_p));
		}
	}
}


VOID RTMPInsertRSNIE(
	IN PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN PUINT8 rsnie_ptr,
	IN UINT8  rsnie_len,
	IN PUINT8 pmkid_ptr,
	IN UINT8  pmkid_len)
{
	PUCHAR	pTmpBuf;
	ULONG 	TempLen = 0;
	UINT8 	extra_len = 0;
	UINT16 	pmk_count = 0;
	UCHAR	ie_num;
	UINT8 	total_len = 0;	
    UCHAR	WPA2_OUI[3]={0x00,0x0F,0xAC};

	pTmpBuf = pFrameBuf;

	/* PMKID-List Must larger than 0 and the multiple of 16. */
	if (pmkid_len > 0 && ((pmkid_len & 0x0f) == 0))
	{		
		extra_len = sizeof(UINT16) + pmkid_len;

		pmk_count = (pmkid_len >> 4);
		pmk_count = cpu2le16(pmk_count);
	}
	else
	{
		DBGPRINT(RT_LOUD, ("[LOUD]%s: no PMKID-List included(%d).\n",
									__FUNCTION__, pmkid_len));
	}

	if (rsnie_len != 0)
	{	
		ie_num = IE_WPA;
		total_len = rsnie_len;
	
		if (NdisEqualMemory(rsnie_ptr + 2, WPA2_OUI, sizeof(WPA2_OUI)))
		{	
			ie_num = IE_RSN;
			total_len += extra_len;
		}

		/* construct RSNIE body */
		MakeOutgoingFrame(pTmpBuf,			&TempLen,
					  	  1,				&ie_num,
					  	  1,				&total_len,
					  	  rsnie_len,		rsnie_ptr,
					  	  END_OF_ARGS);

		pTmpBuf += TempLen;
		*pFrameLen = *pFrameLen + TempLen;

		if (ie_num == IE_RSN)
		{
			/* Insert PMKID-List field */
			if (extra_len > 0)
			{
				MakeOutgoingFrame(pTmpBuf,			&TempLen,
							  	  2,				&pmk_count,
							  	  pmkid_len,		pmkid_ptr,
							  	  END_OF_ARGS);
			
				pTmpBuf += TempLen;
				*pFrameLen = *pFrameLen + TempLen;
			}								
		}
	}
		
	return; 
}

/// --- below new and delop at 0511


UCHAR BtoH(CHAR ch)
{
	if (ch >= '0' && ch <= '9') return (ch - '0');        /* Handle numerals*/
	if (ch >= 'A' && ch <= 'F') return (ch - 'A' + 0xA);  /* Handle capitol hex digits*/
	if (ch >= 'a' && ch <= 'f') return (ch - 'a' + 0xA);  /* Handle small hex digits*/
	return(255);
}


/*  FUNCTION: AtoH(char *, UCHAR *, int)*/
/*  PURPOSE:  Converts ascii string to network order hex*/
/*  PARAMETERS:*/
/*    src    - pointer to input ascii string*/
/*    dest   - pointer to output hex*/
/*    destlen - size of dest*/
/*  COMMENTS:*/
/*    2 ascii bytes make a hex byte so must put 1st ascii byte of pair*/
/*    into upper nibble and 2nd ascii byte of pair into lower nibble.*/
/* IRQL = PASSIVE_LEVEL*/

void AtoH(PSTRING src, PUCHAR dest, int destlen)
{
	PSTRING srcptr;
	PUCHAR destTemp;

	srcptr = src;	
	destTemp = (PUCHAR) dest; 

	while(destlen--)
	{
		*destTemp = BtoH(*srcptr++) << 4;    /* Put 1st ascii byte in upper nibble.*/
		*destTemp += BtoH(*srcptr++);      /* Add 2nd ascii byte to above.*/
		destTemp++;
	}
}

/*
* F(P, S, c, i) = U1 xor U2 xor ... Uc 
* U1 = PRF(P, S || Int(i)) 
* U2 = PRF(P, U1) 
* Uc = PRF(P, Uc-1) 
*/ 

static void F(char *password, unsigned char *ssid, int ssidlength, int iterations, int count, unsigned char *output) 
{ 
    unsigned char digest[36], digest1[SHA1_DIGEST_SIZE]; 
    int i, j, len; 
	
	len = strlen(password);

    /* U1 = PRF(P, S || int(i)) */ 
    memcpy(digest, ssid, ssidlength); 
    digest[ssidlength] = (unsigned char)((count>>24) & 0xff); 
    digest[ssidlength+1] = (unsigned char)((count>>16) & 0xff); 
    digest[ssidlength+2] = (unsigned char)((count>>8) & 0xff); 
    digest[ssidlength+3] = (unsigned char)(count & 0xff); 
    RT_HMAC_SHA1((unsigned char*) password, len, digest, ssidlength+4, digest1, SHA1_DIGEST_SIZE); /* for WPA update*/

    /* output = U1 */ 
    memcpy(output, digest1, SHA1_DIGEST_SIZE); 
    for (i = 1; i < iterations; i++) 
    {
        /* Un = PRF(P, Un-1) */ 
        RT_HMAC_SHA1((unsigned char*) password, len, digest1, SHA1_DIGEST_SIZE, digest, SHA1_DIGEST_SIZE); /* for WPA update*/
        memcpy(digest1, digest, SHA1_DIGEST_SIZE); 
		
        /* output = output xor Un */ 
        for (j = 0; j < SHA1_DIGEST_SIZE; j++) 
        { 
            output[j] ^= digest[j];
        } 
    }
}


/* 
* password - ascii string up to 63 characters in length 
* ssid - octet string up to 32 octets 
* ssidlength - length of ssid in octets 
* output must be 40 octets in length and outputs 256 bits of key 
*/ 
int RtmpPasswordHash(PSTRING password, PUCHAR ssid, INT ssidlength, PUCHAR output) 
{ 
    if ((strlen(password) > 63) || (ssidlength > 32))
        return 0; 

    F(password, ssid, ssidlength, 4096, 1, output); 
    F(password, ssid, ssidlength, 4096, 2, &output[SHA1_DIGEST_SIZE]); 
    return 1; 
}


/* 
    ==========================================================================
    Description:
        Set WPA PSK key

    Arguments:
        pAdapter	Pointer to our adapter
        keyString	WPA pre-shared key string
        pHashStr	String used for password hash function
        hashStrLen	Lenght of the hash string
        pPMKBuf		Output buffer of WPAPSK key

    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT RT_CfgSetWPAPSKKey(
	IN PRTMP_ADAPTER	pAd, 
	IN PSTRING			keyString,
	IN INT				keyStringLen,
	IN PUCHAR			pHashStr,
	IN INT				hashStrLen,
	OUT PUCHAR			pPMKBuf)
{
	UCHAR keyMaterial[40];

	if ((keyStringLen < 8) || (keyStringLen > 64))
	{
		DBGPRINT(RT_TRACE, ("[TRACE]WPAPSK Key length(%d) error, required 8 ~ 64 characters!(keyStr=%s)\n", 
									keyStringLen, keyString));
		return FALSE;
	}

	NdisZeroMemory(pPMKBuf, 32);
	if (keyStringLen == 64)
	{
	    AtoH(keyString, pPMKBuf, 32);
	}
	else
	{
		DBGPRINT(RT_TRACE, ("[TRACE]%s: WPAPSK Key %s, length(%d)\n", __FUNCTION__, keyString, keyStringLen));
	    RtmpPasswordHash(keyString, pHashStr, hashStrLen, keyMaterial);
	    NdisMoveMemory(pPMKBuf, keyMaterial, 32);		
	}
	
//	hex_dump((PUCHAR)"PMK =", pPMKBuf, 32);

	return TRUE;
}

/*
    ==========================================================================
    Description:
		Report the EAP message type

	Arguments:
		msg		-	EAPOL_PAIR_MSG_1
					EAPOL_PAIR_MSG_2
					EAPOL_PAIR_MSG_3
					EAPOL_PAIR_MSG_4
					EAPOL_GROUP_MSG_1
					EAPOL_GROUP_MSG_2
											       
    Return:
         message type string

    ==========================================================================
*/
PSTRING GetEapolMsgType(CHAR msg)
{
    if (msg == EAPOL_PAIR_MSG_1)
        return "Pairwise Message 1";
    else if (msg == EAPOL_PAIR_MSG_2)
        return "Pairwise Message 2";
	else if (msg == EAPOL_PAIR_MSG_3)
        return "Pairwise Message 3";
	else if (msg == EAPOL_PAIR_MSG_4)
        return "Pairwise Message 4";
	else if (msg == EAPOL_GROUP_MSG_1)
        return "Group Message 1";
	else if (msg == EAPOL_GROUP_MSG_2)
        return "Group Message 2";
    else
    	return "Invalid Message";
}

PSTRING GetEncryptType(CHAR enc)
{
    if (enc == Ndis802_11WEPDisabled)
        return "NONE";
    if (enc == Ndis802_11WEPEnabled)
    	return "WEP";
    if (enc == Ndis802_11Encryption2Enabled)
    	return "TKIP";
    if (enc == Ndis802_11Encryption3Enabled)
    	return "AES";
	if (enc == Ndis802_11Encryption4Enabled)
    	return "TKIPAES";
#ifdef WAPI_SUPPORT
	if (enc == Ndis802_11EncryptionSMS4Enabled)
    	return "SMS4";
#endif /* WAPI_SUPPORT */
    else
    	return "UNKNOW";
}

PSTRING GetAuthMode(CHAR auth)
{
    if (auth == Ndis802_11AuthModeOpen)
    	return "OPEN";
    if (auth == Ndis802_11AuthModeShared)
    	return "SHARED";
	if (auth == Ndis802_11AuthModeAutoSwitch)
    	return "AUTOWEP";
    if (auth == Ndis802_11AuthModeWPA)
    	return "WPA";
    if (auth == Ndis802_11AuthModeWPAPSK)
    	return "WPAPSK";
    if (auth == Ndis802_11AuthModeWPANone)
    	return "WPANONE";
    if (auth == Ndis802_11AuthModeWPA2)
    	return "WPA2";
    if (auth == Ndis802_11AuthModeWPA2PSK)
    	return "WPA2PSK";
	if (auth == Ndis802_11AuthModeWPA1WPA2)
    	return "WPA1WPA2";
	if (auth == Ndis802_11AuthModeWPA1PSKWPA2PSK)
    	return "WPA1PSKWPA2PSK";
#ifdef WAPI_SUPPORT
	if (auth == Ndis802_11AuthModeWAICERT)
    	return "WAI-CERT";
	if (auth == Ndis802_11AuthModeWAIPSK)
    	return "WAI-PSK";
#endif /* WAPI_SUPPORT */
	
    	return "UNKNOW";
}		


/*
	========================================================================
	
	Routine Description:
		Calcaulate MIC. It is used during 4-ways handsharking.

	Arguments:
		pAd				-	pointer to our pAdapter context	
    	PeerWepStatus	-	indicate the encryption type    			 
		
	Return Value:

	Note:
	 The EAPOL-Key MIC is a MIC of the EAPOL-Key frames, 
	 from and including the EAPOL protocol version field 
	 to and including the Key Data field, calculated with 
	 the Key MIC field set to 0.
		
	========================================================================
*/
VOID	CalculateMIC(
	IN	UCHAR			KeyDescVer,	
	IN	UCHAR			*PTK,
	OUT PEAPOL_PACKET   pMsg)
{
    UCHAR   *OutBuffer;
	ULONG	FrameLen = 0;
	UCHAR	mic[LEN_KEY_DESC_MIC];
	UCHAR	digest[80];

	/* allocate memory for MIC calculation*/
	OutBuffer = (PUCHAR)pvPortMalloc(512);
    if (OutBuffer == NULL)
    {
		DBGPRINT(RT_ERROR, ("!!!CalculateMIC: no memory!!!\n"));
		return;
    }
		
	/* make a frame for calculating MIC.*/
    MakeOutgoingFrame(OutBuffer,            						&FrameLen,
                      CONV_ARRARY_TO_UINT16(pMsg->Body_Len) + 4,  	pMsg,
                      END_OF_ARGS);

	NdisZeroMemory(mic, sizeof(mic));
			
	/* Calculate MIC*/
    if (KeyDescVer == KEY_DESC_AES)
 	{	
		RT_HMAC_SHA1(PTK, LEN_PTK_KCK, OutBuffer,  FrameLen, digest, SHA1_DIGEST_SIZE);
		NdisMoveMemory(mic, digest, LEN_KEY_DESC_MIC);
	}
	else if (KeyDescVer == KEY_DESC_TKIP)
	{
		RT_HMAC_MD5(PTK, LEN_PTK_KCK, OutBuffer, FrameLen, mic, MD5_DIGEST_SIZE);
	}
	else if (KeyDescVer == KEY_DESC_EXT)
	{
		UINT	mlen = AES_KEY128_LENGTH;
		AES_CMAC(OutBuffer, FrameLen, PTK, LEN_PTK_KCK, mic, &mlen);
	}        

	/* store the calculated MIC*/
	NdisMoveMemory(pMsg->KeyDesc.KeyMic, mic, LEN_KEY_DESC_MIC);

	vPortFree(OutBuffer);
}


VOID WPAInstallPairwiseKey(
	PRTMP_ADAPTER		pAd,
	UINT8				BssIdx,
	PMGMTENTRY			pEntry,
	BOOLEAN				bAE)
{
    NdisZeroMemory(&pEntry->PairwiseKey, sizeof(CIPHER_KEY));   

	/* Assign the pairwise cipher algorithm	*/
    if (pEntry->WepStatus == Ndis802_11Encryption2Enabled)
        pEntry->PairwiseKey.CipherAlg = CIPHER_TKIP;
    else if (pEntry->WepStatus == Ndis802_11Encryption3Enabled)
        pEntry->PairwiseKey.CipherAlg = CIPHER_AES;
	else
	{
		DBGPRINT(RT_ERROR, ("%s : fails (wcid-%d)\n", 
										__FUNCTION__, pEntry->Aid));	
		return;
	}	

	/* Assign key material and its length */
    pEntry->PairwiseKey.KeyLen = LEN_TK;
    NdisMoveMemory(pEntry->PairwiseKey.Key, &pEntry->PTK[OFFSET_OF_PTK_TK], LEN_TK);
	if (pEntry->PairwiseKey.CipherAlg == CIPHER_TKIP)
	{
		if (bAE)
		{
		    NdisMoveMemory(pEntry->PairwiseKey.TxMic, &pEntry->PTK[OFFSET_OF_AP_TKIP_TX_MIC], LEN_TKIP_MIC);
		    NdisMoveMemory(pEntry->PairwiseKey.RxMic, &pEntry->PTK[OFFSET_OF_AP_TKIP_RX_MIC], LEN_TKIP_MIC);
		}
		else
		{
		    NdisMoveMemory(pEntry->PairwiseKey.TxMic, &pEntry->PTK[OFFSET_OF_STA_TKIP_TX_MIC], LEN_TKIP_MIC);
		    NdisMoveMemory(pEntry->PairwiseKey.RxMic, &pEntry->PTK[OFFSET_OF_STA_TKIP_RX_MIC], LEN_TKIP_MIC);
		}
	}

	{
		/* Add Pair-wise key to Asic */
	    AsicAddPairwiseKeyEntry(
						        pAd, 
						        (UCHAR)pEntry->Aid, 
						        &pEntry->PairwiseKey);

		RTMPSetWcidSecurityInfo(pAd, 
								BssIdx, 
								0, //pAd->DefaultKeyId,
								pEntry->PairwiseKey.CipherAlg,
								(UCHAR)pEntry->Aid, 
								PAIRWISEKEYTABLE);		
	}
}


VOID WPAInstallSharedKey(
	PRTMP_ADAPTER		pAd,
	UINT8				GroupCipher,
	UINT8				BssIdx,
	UINT8				KeyIdx,
	UINT8				Wcid,
	BOOLEAN				bAE,
	PUINT8				pGtk,
	UINT8				GtkLen)
{
	PCIPHER_KEY 	pSharedKey;
	
	if (BssIdx >= MAX_MBSSID_NUM(pAd))
	{
		DBGPRINT(RT_ERROR, ("[ERR]%s: The BSS-index(%d) is out of range for MBSSID link. \n", 
									__FUNCTION__, BssIdx));	
		return;
	}
	
	pSharedKey = &pAd->SharedKey[BssIdx][KeyIdx];

	NdisZeroMemory(pSharedKey, sizeof(CIPHER_KEY));
	
	/* Set the group cipher */
	if (GroupCipher == Ndis802_11GroupWEP40Enabled)
		pSharedKey->CipherAlg = CIPHER_WEP64;
	else if (GroupCipher == Ndis802_11GroupWEP104Enabled)
		pSharedKey->CipherAlg = CIPHER_WEP128;
	else if (GroupCipher == Ndis802_11Encryption2Enabled)
		pSharedKey->CipherAlg = CIPHER_TKIP;
	else if (GroupCipher == Ndis802_11Encryption3Enabled)
		pSharedKey->CipherAlg = CIPHER_AES;
	else
	{
		DBGPRINT(RT_ERROR, ("[ERR]%s: fails (IF/ra%d) \n", 
										__FUNCTION__, BssIdx));	
		return;
	}
			
	/* Set the key material and its length */
	if (GroupCipher == Ndis802_11GroupWEP40Enabled || 
		GroupCipher == Ndis802_11GroupWEP104Enabled)
	{
		/* Sanity check the length */
		if ((GtkLen != LEN_WEP64_KEY) && (GtkLen != LEN_WEP128_KEY))
		{
			DBGPRINT(RT_ERROR, ("[ERR]%s: (IF/ra%d) WEP key invlaid(%d) \n", 
										__FUNCTION__, BssIdx, GtkLen));	
			return;
		}
				
		pSharedKey->KeyLen = GtkLen;
		NdisMoveMemory(pSharedKey->Key, pGtk, GtkLen);
	}
	else
	{
		/* Sanity check the length */
		if (GtkLen < LEN_TK)
		{
			DBGPRINT(RT_ERROR, ("[ERR]%s: (IF/ra%d) WPA key invlaid(%d) \n", 
										__FUNCTION__, BssIdx, GtkLen));	
			return;
		}
	
		pSharedKey->KeyLen = LEN_TK;
		NdisMoveMemory(pSharedKey->Key, pGtk, LEN_TK);
		if (pSharedKey->CipherAlg == CIPHER_TKIP)
		{
			if (bAE)
			{
				NdisMoveMemory(pSharedKey->TxMic, pGtk + 16, LEN_TKIP_MIC);
				NdisMoveMemory(pSharedKey->RxMic, pGtk + 24, LEN_TKIP_MIC);            
			}
			else
			{
				NdisMoveMemory(pSharedKey->TxMic, pGtk + 24, LEN_TKIP_MIC);
				NdisMoveMemory(pSharedKey->RxMic, pGtk + 16, LEN_TKIP_MIC);            
			}
		}
	}
	
	/* Update group key table(0x6C00) and group key mode(0x7000) */
    AsicAddSharedKeyEntry(
				pAd, 
				BssIdx, 
				KeyIdx, 
				pSharedKey);

	/* When Wcid isn't zero, it means that this is a Authenticator Role. 
	   Only Authenticator entity needs to set HW IE/EIV table (0x6000)
	   and WCID attribute table (0x6800) for group key. */
	if (Wcid != 0)
	{	
		RTMPSetWcidSecurityInfo(pAd, 
								BssIdx, 
								KeyIdx, 
								pSharedKey->CipherAlg,
								Wcid, 
								SHAREDKEYTABLE);
	}
}



/*
    ========================================================================
    
    Routine Description:
    Parse KEYDATA field.  KEYDATA[] May contain 2 RSN IE and optionally GTK.  
    GTK  is encaptulated in KDE format at  p.83 802.11i D10

    Arguments:
        
    Return Value:

    Note:
        802.11i D10  
        
    ========================================================================
*/
BOOLEAN RTMPParseEapolKeyData(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR          pKeyData,
	IN  UCHAR           KeyDataLen,
	IN	UCHAR			GroupKeyIndex,
	IN	UCHAR			MsgType,
	IN	BOOLEAN			bWPA2,
	IN  PMGMTENTRY 	pEntry)
{
    PUCHAR              pMyKeyData = pKeyData;
    UCHAR               KeyDataLength = KeyDataLen;
	UCHAR				GTK[MAX_LEN_GTK];
    UCHAR               GTKLEN = 0;
	UCHAR				DefaultIdx = 0;
	UCHAR				skip_offset = 0;			


	NdisZeroMemory(GTK, MAX_LEN_GTK);

	/* Verify The RSN IE contained in pairewise_msg_2 && pairewise_msg_3 and skip it*/
	if (MsgType == EAPOL_PAIR_MSG_2 || MsgType == EAPOL_PAIR_MSG_3)
    {
		{
			if (bWPA2 && MsgType == EAPOL_PAIR_MSG_3)
			{
				/*WpaShowAllsuite(pMyKeyData, skip_offset);*/
			
				/* skip RSN IE*/
				pMyKeyData += skip_offset;
				KeyDataLength -= skip_offset;
				DBGPRINT(RT_LOUD, ("[LOUD]%s ==> WPA2/WPA2PSK RSN IE matched in Msg 3, Length(%d) \n", 
									__FUNCTION__, skip_offset));
			}
			else
				return TRUE;			
		}
	}

	DBGPRINT(RT_LOUD,("[LOUD]%s ==> KeyDataLength %d without RSN_IE \n", __FUNCTION__, KeyDataLength));
	/*hex_dump("remain data", pMyKeyData, KeyDataLength);*/


	/* Parse KDE format in pairwise_msg_3_WPA2 && group_msg_1_WPA2*/
	if (bWPA2 && (MsgType == EAPOL_PAIR_MSG_3 || MsgType == EAPOL_GROUP_MSG_1))
	{				
		PEID_STRUCT     pEid;
			
		pEid = (PEID_STRUCT) pMyKeyData;
		skip_offset = 0;
		while ((skip_offset + 2 + pEid->Len) <= KeyDataLength)
		{
			switch(pEid->Eid)
			{
				case WPA_KDE_TYPE:
					{
						PKDE_HDR	pKDE;

						pKDE = (PKDE_HDR)pEid;
						if (NdisEqualMemory(pKDE->OUI, OUI_WPA2, 3))
    					{
							if (pKDE->DataType == KDE_GTK)
							{
								PGTK_KDE pKdeGtk;
								
								pKdeGtk = (PGTK_KDE) &pKDE->octet[0];
								DefaultIdx = pKdeGtk->Kid;

								/* Get GTK length - refer to IEEE 802.11i-2004 p.82 */
								GTKLEN = pKDE->Len -6;
								if (GTKLEN < LEN_WEP64_KEY)
								{
									DBGPRINT(RT_ERROR, ("[ERR]%s: GTK Key length is too short (%d) \n", __FUNCTION__, GTKLEN));
        							return FALSE;
								}
								NdisMoveMemory(GTK, pKdeGtk->GTK, GTKLEN);
								DBGPRINT(RT_LOUD, ("[LOUD]%s: GTK in KDE format ,DefaultKeyID=%d, KeyLen=%d \n", __FUNCTION__, DefaultIdx, GTKLEN));
    						}
						}
					}
					break;
			}
			skip_offset = skip_offset + 2 + pEid->Len;
	        pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);   
		}

		/* skip KDE Info*/
		pMyKeyData += skip_offset;
		KeyDataLength -= skip_offset;		
	}
	else if (!bWPA2 && MsgType == EAPOL_GROUP_MSG_1)
	{
		DefaultIdx = GroupKeyIndex;
		GTKLEN = KeyDataLength;
		NdisMoveMemory(GTK, pMyKeyData, KeyDataLength);
		DBGPRINT(RT_LOUD, ("[LOUD]%s: GTK without KDE, DefaultKeyID=%d, KeyLen=%d \n", __FUNCTION__, DefaultIdx, GTKLEN));
	}
		
	/* Sanity check - shared key index must be 0 ~ 3*/
	if (DefaultIdx > 3)	
    {
     	DBGPRINT(RT_ERROR, ("[ERR]%s: GTK Key index(%d) is invalid in %s %s \n", __FUNCTION__, DefaultIdx, ((bWPA2) ? "WPA2" : "WPA"), GetEapolMsgType(MsgType)));
        return FALSE;
    } 

	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		/* set key material, TxMic and RxMic		*/
		NdisZeroMemory(pAd->StaCfg.GTK, MAX_LEN_GTK);
		NdisMoveMemory(pAd->StaCfg.GTK, GTK, GTKLEN);
		pAd->StaCfg.DefaultKeyId = DefaultIdx;

		/* Wcid must set 0 */
		WPAInstallSharedKey(pAd, 
							pAd->StaCfg.GroupCipher, 
							BSS0, 
							pAd->StaCfg.DefaultKeyId, 
							0, 
							FALSE, 
							pAd->StaCfg.GTK,
							GTKLEN); 
	}

	return TRUE;
 
}


/*
    ==========================================================================
    Description:
        Check the validity of the received EAPoL frame
    Return:
        TRUE if all parameters are OK, 
        FALSE otherwise
    ==========================================================================
 */
BOOLEAN PeerWpaMessageSanity(
    IN 	PRTMP_ADAPTER 		pAd, 
    IN 	PEAPOL_PACKET 		pMsg, 
    IN 	ULONG 				MsgLen, 
    IN 	UCHAR				MsgType,
    IN 	PMGMTENTRY  		pEntry)
{
	UCHAR			mic[LEN_KEY_DESC_MIC], digest[80]; /*, KEYDATA[MAX_LEN_OF_RSNIE];*/
	PUCHAR			KEYDATA = NULL;
	BOOLEAN			bReplayDiff = FALSE;
	BOOLEAN			bWPA2 = FALSE;
	KEY_INFO		EapolKeyInfo;	
	UCHAR			GroupKeyIndex = 0;
	USHORT			U16Tmp = 0;

	/* allocate memory */
	KEYDATA = (PUCHAR)pvPortMalloc(MAX_LEN_OF_RSNIE);
	if (KEYDATA == NULL)
	{
		DBGPRINT(RT_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		return FALSE;
	}

	NdisZeroMemory(mic, sizeof(mic));
	NdisZeroMemory(digest, sizeof(digest));
	NdisZeroMemory(KEYDATA, MAX_LEN_OF_RSNIE);
	NdisZeroMemory((PUCHAR)&EapolKeyInfo, sizeof(EapolKeyInfo));
	NdisMoveMemory((PUCHAR)&EapolKeyInfo, (PUCHAR)&pMsg->KeyDesc.KeyInfo, sizeof(KEY_INFO));

	NdisMoveMemory(&U16Tmp, &EapolKeyInfo, sizeof(EapolKeyInfo));
	U16Tmp = cpu2le16(U16Tmp);
	NdisMoveMemory(&EapolKeyInfo, &U16Tmp, sizeof(EapolKeyInfo));

//	*((USHORT *)&EapolKeyInfo) = cpu2le16(*((USHORT *)&EapolKeyInfo));

	/* Choose WPA2 or not*/
	if ((pEntry->AuthMode == Ndis802_11AuthModeWPA2) || (pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK))
		bWPA2 = TRUE;

	/* 0. Check MsgType*/
	if ((MsgType > EAPOL_GROUP_MSG_2) || (MsgType < EAPOL_PAIR_MSG_1))
	{
		DBGPRINT(RT_ERROR, ("[ERR]%s: The message type is invalid(%d)! \n", __FUNCTION__, MsgType));
		goto LabelErr;
	}
				
	/* 1. Replay counter check	*/
 	if (MsgType == EAPOL_PAIR_MSG_1 || MsgType == EAPOL_PAIR_MSG_3 || MsgType == EAPOL_GROUP_MSG_1)	/* For supplicant*/
    {
    	/* First validate replay counter, only accept message with larger replay counter.*/
		/* Let equal pass, some AP start with all zero replay counter*/
		UCHAR	ZeroReplay[LEN_KEY_DESC_REPLAY];
		
        NdisZeroMemory(ZeroReplay, LEN_KEY_DESC_REPLAY);
		if ((RTMPCompareMemory(pMsg->KeyDesc.ReplayCounter, pEntry->R_Counter, LEN_KEY_DESC_REPLAY) != 1) &&
			(RTMPCompareMemory(pMsg->KeyDesc.ReplayCounter, ZeroReplay, LEN_KEY_DESC_REPLAY) != 0))
    	{
			bReplayDiff = TRUE;
    	}						
 	}
	else if (MsgType == EAPOL_PAIR_MSG_2 || MsgType == EAPOL_PAIR_MSG_4 || MsgType == EAPOL_GROUP_MSG_2)	/* For authenticator*/
	{
		/* check Replay Counter coresponds to MSG from authenticator, otherwise discard*/
    	if (!NdisEqualMemory(pMsg->KeyDesc.ReplayCounter, pEntry->R_Counter, LEN_KEY_DESC_REPLAY))
    	{	
			bReplayDiff = TRUE;	        
    	}
	}

	/* Replay Counter different condition*/
	if (bReplayDiff)
	{
		/* send wireless event - for replay counter different*/
		//RTMPSendWirelessEvent(pAd, IW_REPLAY_COUNTER_DIFF_EVENT_FLAG, pEntry->Addr, pEntry->apidx, 0); 

		if (MsgType < EAPOL_GROUP_MSG_1)
		{
           	DBGPRINT(RT_ERROR, ("[ERR]%s: Replay Counter Different in pairwise msg %d of 4-way handshake!\n", __FUNCTION__, MsgType));
		}
		else
		{
			DBGPRINT(RT_ERROR, ("[ERR]%s: Replay Counter Different in group msg %d of 2-way handshake!\n", __FUNCTION__, (MsgType - EAPOL_PAIR_MSG_4)));
		}
		
		hex_dump((PUCHAR)"Receive replay counter ", pMsg->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);
		hex_dump((PUCHAR)"Current replay counter ", pEntry->R_Counter, LEN_KEY_DESC_REPLAY);	
        goto LabelErr;
	}

	/* 2. Verify MIC except Pairwise Msg1*/
	if (MsgType != EAPOL_PAIR_MSG_1)
	{
		UCHAR			rcvd_mic[LEN_KEY_DESC_MIC];
		UINT			eapol_len = CONV_ARRARY_TO_UINT16(pMsg->Body_Len) + 4;

		/* Record the received MIC for check later*/
		NdisMoveMemory(rcvd_mic, pMsg->KeyDesc.KeyMic, LEN_KEY_DESC_MIC);
		NdisZeroMemory(pMsg->KeyDesc.KeyMic, LEN_KEY_DESC_MIC);
							
        if (EapolKeyInfo.KeyDescVer == KEY_DESC_TKIP)	/* TKIP*/
        {	
            RT_HMAC_MD5(pEntry->PTK, LEN_PTK_KCK, (PUCHAR)pMsg, eapol_len, mic, MD5_DIGEST_SIZE);
        }
        else if (EapolKeyInfo.KeyDescVer == KEY_DESC_AES)	/* AES        */
        {                        
            RT_HMAC_SHA1(pEntry->PTK, LEN_PTK_KCK, (PUCHAR)pMsg, eapol_len, digest, SHA1_DIGEST_SIZE);
            NdisMoveMemory(mic, digest, LEN_KEY_DESC_MIC);
        }
        else if (EapolKeyInfo.KeyDescVer == KEY_DESC_EXT)	/* AES-128 */        
        {                
            UINT mlen = AES_KEY128_LENGTH;
            AES_CMAC((PUCHAR)pMsg, eapol_len, pEntry->PTK, LEN_PTK_KCK, mic, &mlen);			
        }        
        
        if (!NdisEqualMemory(rcvd_mic, mic, LEN_KEY_DESC_MIC))
        {
			if (MsgType < EAPOL_GROUP_MSG_1)
			{
            	DBGPRINT(RT_ERROR, ("[ERR]%s: MIC Different in pairwise msg %d of 4-way handshake!\n", __FUNCTION__, MsgType));
			}
			else
			{
				DBGPRINT(RT_ERROR, ("[ERR]%s: MIC Different in group msg %d of 2-way handshake!\n", __FUNCTION__, (MsgType - EAPOL_PAIR_MSG_4)));
			}
	
			hex_dump((PUCHAR)"Received MIC", rcvd_mic, LEN_KEY_DESC_MIC);
			hex_dump((PUCHAR)"Desired  MIC", mic, LEN_KEY_DESC_MIC);

			goto LabelErr;
        }        
	}

	/* 1. Decrypt the Key Data field if GTK is included.*/
	/* 2. Extract the context of the Key Data field if it exist.	 */
	/* The field in pairwise_msg_2_WPA1(WPA2) & pairwise_msg_3_WPA1 is clear.*/
	/* The field in group_msg_1_WPA1(WPA2) & pairwise_msg_3_WPA2 is encrypted.*/
	if (CONV_ARRARY_TO_UINT16(pMsg->KeyDesc.KeyDataLen) > 0)
	{		
		/* Decrypt this field		*/
		if ((MsgType == EAPOL_PAIR_MSG_3 && bWPA2) || (MsgType == EAPOL_GROUP_MSG_1))
		{					
			if((EapolKeyInfo.KeyDescVer == KEY_DESC_EXT) || (EapolKeyInfo.KeyDescVer == KEY_DESC_AES))
			{
				UINT aes_unwrap_len = 0;
				
				/* AES */
				AES_Key_Unwrap(pMsg->KeyDesc.KeyData, 
									CONV_ARRARY_TO_UINT16(pMsg->KeyDesc.KeyDataLen),
							   &pEntry->PTK[LEN_PTK_KCK], LEN_PTK_KEK, 
							   KEYDATA, &aes_unwrap_len);
				SET_UINT16_TO_ARRARY(pMsg->KeyDesc.KeyDataLen, aes_unwrap_len);
			} 
			else	  
			{
				TKIP_GTK_KEY_UNWRAP(&pEntry->PTK[LEN_PTK_KCK], 
									pMsg->KeyDesc.KeyIv,									
									pMsg->KeyDesc.KeyData, 
									CONV_ARRARY_TO_UINT16(pMsg->KeyDesc.KeyDataLen),
									KEYDATA);
			}	

			if (!bWPA2 && (MsgType == EAPOL_GROUP_MSG_1))
				GroupKeyIndex = EapolKeyInfo.KeyIndex;
			
		}
		else if ((MsgType == EAPOL_PAIR_MSG_2) || (MsgType == EAPOL_PAIR_MSG_3 && !bWPA2))
		{					
			NdisMoveMemory(KEYDATA, pMsg->KeyDesc.KeyData, CONV_ARRARY_TO_UINT16(pMsg->KeyDesc.KeyDataLen));			     
		}
		else
		{
			
			goto LabelOK;
		}

		/* Parse Key Data field to */
		/* 1. verify RSN IE for pairwise_msg_2_WPA1(WPA2) ,pairwise_msg_3_WPA1(WPA2)*/
		/* 2. verify KDE format for pairwise_msg_3_WPA2, group_msg_1_WPA2*/
		/* 3. update shared key for pairwise_msg_3_WPA2, group_msg_1_WPA1(WPA2)*/
		if (!RTMPParseEapolKeyData(pAd, KEYDATA, 
								  CONV_ARRARY_TO_UINT16(pMsg->KeyDesc.KeyDataLen), 
								  GroupKeyIndex, MsgType, bWPA2, pEntry))
		{
			goto LabelErr;
		}
	}

LabelOK:
	if (KEYDATA != NULL)
		vPortFree(KEYDATA);
	return TRUE;
	
LabelErr:
	if (KEYDATA != NULL)
		vPortFree(KEYDATA);
	return FALSE;
}



/*
	========================================================================
	
	Routine Description:
		Construct KDE common format  
		Its format is below,
		
		+--------------------+
		| Type (0xdd)		 |  1 octet
		+--------------------+
		| Length			 |	1 octet	
		+--------------------+
		| OUI				 |  3 octets
		+--------------------+
		| Data Type			 |	1 octet
		+--------------------+
		
	Arguments:
				
	Return Value:
		
	Note:
		It's defined in IEEE 802.11-2007 Figure 8-25.
		
	========================================================================
*/
VOID WPA_ConstructKdeHdr(
	IN 	UINT8	data_type,	
	IN 	UINT8 	data_len,
	OUT PUCHAR 	pBuf)
{
	PKDE_HDR	pHdr;

	pHdr = (PKDE_HDR)pBuf;

	NdisZeroMemory(pHdr, sizeof(KDE_HDR));

    pHdr->Type = WPA_KDE_TYPE;

	/* The Length field specifies the number of octets in the OUI, Data
	   Type, and Data fields. */	   
	pHdr->Len = 4 + data_len;

	NdisMoveMemory(pHdr->OUI, OUI_WPA2, 3);
	pHdr->DataType = data_type;

}



/*
	========================================================================
	
	Routine Description:
		Construct the Key Data field of EAPoL message 

	Arguments:
		pAd			Pointer	to our adapter
		Elem		Message body
		
	Return Value:
		None
		
	Note:
		
	========================================================================
*/
VOID	ConstructEapolKeyData(
	IN	PMGMTENTRY		pEntry,
	IN	UCHAR			GroupKeyWepStatus,
	IN	UCHAR			keyDescVer,
	IN 	UCHAR			MsgType,
	IN	UCHAR			DefaultKeyIdx,
	IN	UCHAR			*GTK,
	IN	UCHAR			*RSNIE,
	IN	UCHAR			RSNIE_LEN,
	OUT PEAPOL_PACKET   pMsg)
{
	UCHAR		*mpool, *Key_Data, *eGTK;  	  
	ULONG		data_offset;
	BOOLEAN		bWPA2Capable = FALSE;
	BOOLEAN		GTK_Included = FALSE;

	/* Choose WPA2 or not*/
	if ((pEntry->AuthMode == Ndis802_11AuthModeWPA2) || 
		(pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK))
		bWPA2Capable = TRUE;

	if (MsgType == EAPOL_PAIR_MSG_1 || 
		MsgType == EAPOL_PAIR_MSG_4 || 
		MsgType == EAPOL_GROUP_MSG_2)
		return;
 
	/* allocate memory pool*/
	mpool = (PUCHAR)pvPortMalloc(TX_EAPOL_BUFFER);
    if (mpool == NULL) {
		DBGPRINT(RT_ERROR, ("[ERR]%s: Can't malloc memory\n", __FUNCTION__));
		return;
    }
	
	/* eGTK Len = 512 */
	eGTK = (UCHAR *) ROUND_UP(mpool, 4);
	/* Key_Data Len = 512 */
	Key_Data = (UCHAR *) ROUND_UP(eGTK + 512, 4);

	NdisZeroMemory(Key_Data, 512);
	SET_UINT16_TO_ARRARY(pMsg->KeyDesc.KeyDataLen, 0);
	data_offset = 0;
	
	/* Encapsulate RSNIE in pairwise_msg2 & pairwise_msg3		*/
	if (RSNIE_LEN && ((MsgType == EAPOL_PAIR_MSG_2) || (MsgType == EAPOL_PAIR_MSG_3)))
	{
		PUINT8	pmkid_ptr = NULL;
		UINT8 	pmkid_len = 0;

		RTMPInsertRSNIE(&Key_Data[data_offset], 
						&data_offset,
						RSNIE, 
						RSNIE_LEN, 
						pmkid_ptr, 
						pmkid_len);
	}

	/* Encapsulate GTK 		*/
	/* Only for pairwise_msg3_WPA2 and group_msg1*/
	if ((MsgType == EAPOL_PAIR_MSG_3 && bWPA2Capable) || (MsgType == EAPOL_GROUP_MSG_1))
	{
		UINT8	gtk_len;

		/* Decide the GTK length */ 
		if (GroupKeyWepStatus == Ndis802_11Encryption3Enabled)
			gtk_len = LEN_AES_GTK;
		else
			gtk_len = LEN_TKIP_GTK;
		
		/* Insert GTK KDE format in WAP2 mode */
		if (bWPA2Capable)
		{
			/* Construct the common KDE format */
			WPA_ConstructKdeHdr(KDE_GTK, 2 + gtk_len, &Key_Data[data_offset]);
			data_offset += sizeof(KDE_HDR);

			/* GTK KDE format - 802.11i-2004  Figure-43x*/
			/* KeyID bit (0~1) */
	        Key_Data[data_offset] = (DefaultKeyIdx & 0x03);
	        Key_Data[data_offset + 1] = 0x00;	/* Reserved Byte*/
	        data_offset += 2;
		}

		/* Fill in GTK */
		NdisMoveMemory(&Key_Data[data_offset], GTK, gtk_len);
		data_offset += gtk_len;

		GTK_Included = TRUE;

//		hex_dump((PUCHAR)"GTK_Included", GTK, gtk_len);
	}

	/* If the Encrypted Key Data subfield (of the Key Information field) 
	   is set, the entire Key Data field shall be encrypted. */
	/* This whole key-data field shall be encrypted if a GTK is included.*/
	/* Encrypt the data material in key data field with KEK*/
	if (GTK_Included)
	{
		if ((keyDescVer == KEY_DESC_AES))
		{
			UCHAR 	remainder = 0;
			UCHAR	pad_len = 0;			
			UINT	wrap_len = 0;

			/* Key Descriptor Version 2 or 3: AES key wrap, defined in IETF RFC 3394, */
			/* shall be used to encrypt the Key Data field using the KEK field from */
			/* the derived PTK.*/

			/* If the Key Data field uses the NIST AES key wrap, then the Key Data field */
			/* shall be padded before encrypting if the key data length is less than 16 */
			/* octets or if it is not a multiple of 8. The padding consists of appending*/
			/* a single octet 0xdd followed by zero or more 0x00 octets. */
			if ((remainder = data_offset & 0x07) != 0)
			{
				INT		i;
			
				pad_len = (8 - remainder);
				Key_Data[data_offset] = 0xDD;
				for (i = 1; i < pad_len; i++)
					Key_Data[data_offset + i] = 0;

				data_offset += pad_len;
			}
			AES_Key_Wrap(Key_Data, (UINT) data_offset, 
						 &pEntry->PTK[LEN_PTK_KCK], LEN_PTK_KEK, 
						 eGTK, &wrap_len);

			data_offset = wrap_len;
			
		}
		else
		{
			TKIP_GTK_KEY_WRAP(&pEntry->PTK[LEN_PTK_KCK], 
								pMsg->KeyDesc.KeyIv,									
								Key_Data, 
								data_offset,
								eGTK);
		}

		NdisMoveMemory(pMsg->KeyDesc.KeyData, eGTK, data_offset);
	}
	else
	{
		NdisMoveMemory(pMsg->KeyDesc.KeyData, Key_Data, data_offset);
	}

	/* Update key data length field and total body length*/
	SET_UINT16_TO_ARRARY(pMsg->KeyDesc.KeyDataLen, data_offset);
	INC_UINT16_TO_ARRARY(pMsg->Body_Len, data_offset);

	vPortFree(mpool);
}



/*
	========================================================================
	
	Routine Description:
		Construct EAPoL message for WPA handshaking 
		Its format is below,
		
		+--------------------+
		| Protocol Version	 |  1 octet
		+--------------------+
		| Protocol Type		 |	1 octet	
		+--------------------+
		| Body Length		 |  2 octets
		+--------------------+
		| Descriptor Type	 |	1 octet
		+--------------------+
		| Key Information    |	2 octets
		+--------------------+
		| Key Length	     |  1 octet
		+--------------------+
		| Key Repaly Counter |	8 octets
		+--------------------+
		| Key Nonce		     |  32 octets
		+--------------------+
		| Key IV			 |  16 octets
		+--------------------+
		| Key RSC			 |  8 octets
		+--------------------+
		| Key ID or Reserved |	8 octets
		+--------------------+
		| Key MIC			 |	16 octets
		+--------------------+
		| Key Data Length	 |	2 octets
		+--------------------+
		| Key Data			 |	n octets
		+--------------------+
		

	Arguments:
		pAd			Pointer	to our adapter
				
	Return Value:
		None
		
	Note:
		
	========================================================================
*/
VOID	ConstructEapolMsg(
	IN 	PMGMTENTRY			pEntry,
    IN 	UCHAR				GroupKeyWepStatus,
    IN 	UCHAR				MsgType,  
    IN	UCHAR				DefaultKeyIdx,
	IN 	UCHAR				*KeyNonce,
	IN	UCHAR				*TxRSC,
	IN	UCHAR				*GTK,
	IN	UCHAR				*RSNIE,
	IN	UCHAR				RSNIE_Len,
    OUT PEAPOL_PACKET       pMsg)
{
	BOOLEAN	bWPA2 = FALSE;
	UCHAR	KeyDescVer;

	/* Choose WPA2 or not*/
	if ((pEntry->AuthMode == Ndis802_11AuthModeWPA2) || 
		(pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK))
		bWPA2 = TRUE;
		
    /* Init Packet and Fill header    */
    pMsg->ProVer = EAPOL_VER;
    pMsg->ProType = EAPOLKey;

	/* Default 95 bytes, the EAPoL-Key descriptor exclude Key-data field*/
	SET_UINT16_TO_ARRARY(pMsg->Body_Len, MIN_LEN_OF_EAPOL_KEY_MSG);

	/* Fill in EAPoL descriptor*/
	if (bWPA2)
		pMsg->KeyDesc.Type = WPA2_KEY_DESC;
	else
		pMsg->KeyDesc.Type = WPA1_KEY_DESC;
			
	/* Key Descriptor Version (bits 0-2) specifies the key descriptor version type*/
	{
		/* Fill in Key information, refer to IEEE Std 802.11i-2004 page 78 */
		/* When either the pairwise or the group cipher is AES, the KEY_DESC_AES shall be used.*/
		KeyDescVer = (((pEntry->WepStatus == Ndis802_11Encryption3Enabled) || 
		        		(GroupKeyWepStatus == Ndis802_11Encryption3Enabled)) ? (KEY_DESC_AES) : (KEY_DESC_TKIP));
	}

	pMsg->KeyDesc.KeyInfo.KeyDescVer = KeyDescVer;

	/* Specify Key Type as Group(0) or Pairwise(1)*/
	if (MsgType >= EAPOL_GROUP_MSG_1)
		pMsg->KeyDesc.KeyInfo.KeyType = GROUPKEY;
	else
		pMsg->KeyDesc.KeyInfo.KeyType = PAIRWISEKEY;

	/* Specify Key Index, only group_msg1_WPA1*/
	if (!bWPA2 && (MsgType >= EAPOL_GROUP_MSG_1))
		pMsg->KeyDesc.KeyInfo.KeyIndex = DefaultKeyIdx;
	
	if (MsgType == EAPOL_PAIR_MSG_3)
		pMsg->KeyDesc.KeyInfo.Install = 1;
	
	if ((MsgType == EAPOL_PAIR_MSG_1) || (MsgType == EAPOL_PAIR_MSG_3) || (MsgType == EAPOL_GROUP_MSG_1)) {
		// need client's ACK
		pMsg->KeyDesc.KeyInfo.KeyAck = 1;
	}

	if (MsgType != EAPOL_PAIR_MSG_1)	
		pMsg->KeyDesc.KeyInfo.KeyMic = 1;

 	// In WAP2, Secure bit will be set. But this bit won't be set in WPA1.
	if ((bWPA2 && (MsgType >= EAPOL_PAIR_MSG_3)) || 
		(!bWPA2 && (MsgType >= EAPOL_GROUP_MSG_1)))
    {                        
       	pMsg->KeyDesc.KeyInfo.Secure = 1;                   
    }

	/* This subfield shall be set, and the Key Data field shall be encrypted, if
	   any key material (e.g., GTK or SMK) is included in the frame. */
	if (bWPA2 && ((MsgType == EAPOL_PAIR_MSG_3) || 
		(MsgType == EAPOL_GROUP_MSG_1)))
    {                               	
        pMsg->KeyDesc.KeyInfo.EKD_DL = 1;            
    }

	USHORT U16Tmp;
	/* key Information element has done. */
	NdisMoveMemory(&U16Tmp, &pMsg->KeyDesc.KeyInfo, sizeof(pMsg->KeyDesc.KeyInfo));
	U16Tmp = cpu2le16(U16Tmp);
	NdisMoveMemory(&pMsg->KeyDesc.KeyInfo, &U16Tmp, sizeof(pMsg->KeyDesc.KeyInfo));

	
//	*(USHORT *)(&pMsg->KeyDesc.KeyInfo) = cpu2le16(*(USHORT *)(&pMsg->KeyDesc.KeyInfo));

	/* Fill in Key Length*/
	if (bWPA2)
	{
		/* In WPA2 mode, the field indicates the length of pairwise key cipher, */
		/* so only pairwise_msg_1 and pairwise_msg_3 need to fill. */
		if ((MsgType == EAPOL_PAIR_MSG_1) || (MsgType == EAPOL_PAIR_MSG_3))
			pMsg->KeyDesc.KeyLength[1] = ((pEntry->WepStatus == Ndis802_11Encryption2Enabled) ? LEN_TKIP_TK : LEN_AES_TK);
	}
	else if (!bWPA2)
	{
		if (MsgType >= EAPOL_GROUP_MSG_1)
		{
			/* the length of group key cipher*/
			pMsg->KeyDesc.KeyLength[1] = ((GroupKeyWepStatus == Ndis802_11Encryption2Enabled) ? LEN_TKIP_GTK : LEN_AES_GTK);
		}
		else
		{
			/* the length of pairwise key cipher*/
			pMsg->KeyDesc.KeyLength[1] = ((pEntry->WepStatus == Ndis802_11Encryption2Enabled) ? LEN_TKIP_TK : LEN_AES_TK);			
		}				
	}			
	
 	/* Fill in replay counter        		*/
    NdisMoveMemory(pMsg->KeyDesc.ReplayCounter, pEntry->R_Counter, LEN_KEY_DESC_REPLAY);

	/* Fill Key Nonce field		  */
	/* ANonce : pairwise_msg1 & pairwise_msg3*/
	/* SNonce : pairwise_msg2*/
	/* GNonce : group_msg1_wpa1	*/
	if ((MsgType <= EAPOL_PAIR_MSG_3) || ((!bWPA2 && (MsgType == EAPOL_GROUP_MSG_1))))
    	NdisMoveMemory(pMsg->KeyDesc.KeyNonce, KeyNonce, LEN_KEY_DESC_NONCE);

	/* Fill key IV - WPA2 as 0, WPA1 as random*/
	if (!bWPA2 && (MsgType == EAPOL_GROUP_MSG_1))
	{		
		/* Suggest IV be random number plus some number,*/
		NdisMoveMemory(pMsg->KeyDesc.KeyIv, &KeyNonce[16], LEN_KEY_DESC_IV);		
        pMsg->KeyDesc.KeyIv[15] += 2;		
	}
	
    /* Fill Key RSC field        */
    /* It contains the RSC for the GTK being installed.*/
	if ((MsgType == EAPOL_PAIR_MSG_3 && bWPA2) || (MsgType == EAPOL_GROUP_MSG_1))
	{		
        NdisMoveMemory(pMsg->KeyDesc.KeyRsc, TxRSC, 6);
	}

	/* Clear Key MIC field for MIC calculation later   */
    NdisZeroMemory(pMsg->KeyDesc.KeyMic, LEN_KEY_DESC_MIC);
	
	ConstructEapolKeyData(pEntry,
						  GroupKeyWepStatus, 
						  KeyDescVer,
						  MsgType, 
						  DefaultKeyIdx, 
						  GTK,
						  RSNIE,
						  RSNIE_Len,
						  pMsg);
 
	/* Calculate MIC and fill in KeyMic Field except Pairwise Msg 1.*/
	if (MsgType != EAPOL_PAIR_MSG_1)
	{
		CalculateMIC(KeyDescVer, pEntry->PTK, pMsg);
	}

	DBGPRINT(RT_LOUD, ("[LOUD]%s===>	ConstructEapolMsg for %s %s\n", 
								__FUNCTION__, ((bWPA2) ? "WPA2" : "WPA"), GetEapolMsgType(MsgType)));
	DBGPRINT(RT_LOUD, ("[LOUD]%s		Body length = %d \n", 
								__FUNCTION__, CONV_ARRARY_TO_UINT16(pMsg->Body_Len)));
	DBGPRINT(RT_LOUD, ("[LOUD]%s		Key length  = %d \n", 
								__FUNCTION__, CONV_ARRARY_TO_UINT16(pMsg->KeyDesc.KeyLength)));
}


VOID RTMPGetTxTscFromAsic(
	IN  PRTMP_ADAPTER   pAd,
	IN	UCHAR			apidx,
	OUT	PUCHAR			pTxTsc)
{
	USHORT			Wcid;
	USHORT			offset;
	UCHAR			IvEiv[8];
	int				i;

	/* Sanity check of apidx */
	if (apidx >= MAX_MBSSID_NUM(pAd))
	{
		DBGPRINT(RT_ERROR, ("[ERR]%s: invalid apidx(%d)\n", __FUNCTION__, apidx));
		return;
	}

	/* Initial value */
	NdisZeroMemory(IvEiv, 8);
	NdisZeroMemory(pTxTsc, 6);

	//Wcid = MCAST_WCID;
	/* Get apidx for this BSSID */
	GET_GroupKey_WCID(pAd, Wcid, BSS0);

	/* When the group rekey action is triggered, a count-down(3 seconds) is started. 
	   During the count-down, use the initial PN as TSC.
	   Otherwise, get the IVEIV from ASIC. */
//	if (pAd->ApCfg.RekeyCountDown > 0)
//	{
		/*
		In IEEE 802.11-2007 8.3.3.4.3 described :
		The PN shall be implemented as a 48-bit monotonically incrementing
		non-negative integer, initialized to 1 when the corresponding 
		temporal key is initialized or refreshed. */	
//		IvEiv[0] = 1;
//	}
//	else
	{
		UINT32 temp1, temp2;
		/* Read IVEIV from Asic */
		offset = MAC_IVEIV_TABLE_BASE + (Wcid * HW_IVEIV_ENTRY_SIZE);
		
		/* Use Read32 to avoid endian problem */
		RTMP_IO_READ32(pAd, offset, &temp1);
		RTMP_IO_READ32(pAd, offset+4, &temp2);
		for (i = 0; i < 4; i++)
		{
			IvEiv[i] = (UCHAR)(temp1 >> (i*8));
			IvEiv[i+4] = (UCHAR)(temp2 >> (i*8));
		}
	}

	/* Record current TxTsc */
	if (pAd->ApCfg.GroupKeyWepStatus == Ndis802_11Encryption3Enabled)
	{	/* AES */
		*pTxTsc 	= IvEiv[0];
		*(pTxTsc+1) = IvEiv[1];
		*(pTxTsc+2) = IvEiv[4];
		*(pTxTsc+3) = IvEiv[5];
		*(pTxTsc+4) = IvEiv[6];
		*(pTxTsc+5) = IvEiv[7];					
	}
	else
	{	/* TKIP */
		*pTxTsc 	= IvEiv[2];
		*(pTxTsc+1) = IvEiv[0];
		*(pTxTsc+2) = IvEiv[4];
		*(pTxTsc+3) = IvEiv[5];
		*(pTxTsc+4) = IvEiv[6];
		*(pTxTsc+5) = IvEiv[7];	
	}
	DBGPRINT(RT_LOUD, ("[LOUD]%s: WCID(%d) TxTsc 0x%02x-0x%02x-0x%02x-0x%02x-0x%02x-0x%02x \n",
							__FUNCTION__, Wcid, *pTxTsc, *(pTxTsc+1), *(pTxTsc+2), *(pTxTsc+3), *(pTxTsc+4), *(pTxTsc+5)));
			

}


/*
    ==========================================================================
    Description:
        This is a function to send the first packet of 2-way groupkey handshake
    Return:
         
    ==========================================================================
*/
VOID WPAStart2WayGroupHS(
    IN PRTMP_ADAPTER    pAd, 
    IN PMGMTENTRY  	pEntry) 
{
	UCHAR   			TxTsc[6]; 
	UCHAR   			*mpool;
	PEAPOL_PACKET		pEapolFrame;
	UCHAR				group_cipher = Ndis802_11WEPDisabled;	
	UCHAR				default_key = 0;
	PUINT8				gnonce_ptr = NULL;
	PUINT8				gtk_ptr = NULL;
	//PUINT8				pBssid = NULL;
    
	DBGPRINT(RT_LOUD, ("[LOUD]===> %s\n", __FUNCTION__));

    if (!pEntry)
        return;

	DBGPRINT(RT_TRACE, ("[TRACE]%s: Do 2-Way GTK Update (%02X:%02X:%02X:%02X:%02X:%02X)\n", __FUNCTION__, PRINT_MAC(pEntry->Addr)));

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		UCHAR	apidx = 0;
	
		group_cipher = pAd->ApCfg.GroupKeyWepStatus;
		default_key = pAd->DefaultKeyId;
		gnonce_ptr = pAd->ApCfg.GNonce;
		gtk_ptr = pAd->ApCfg.GTK;
		//pBssid = pAd->essid;

		/* Get Group TxTsc form Asic*/
		RTMPGetTxTscFromAsic(pAd, apidx, TxTsc);
	}

	/* Allocate memory for output*/
	mpool = (PUCHAR)pvPortMalloc(TX_EAPOL_BUFFER);
	if (mpool == NULL)
    {
        DBGPRINT(RT_ERROR, ("!!!%s : no memory!!!\n", __FUNCTION__));
        return;
    }

	pEapolFrame = (PEAPOL_PACKET)mpool;
	NdisZeroMemory(pEapolFrame, TX_EAPOL_BUFFER);

    /* Increment replay counter by 1*/
	ADD_ONE_To_64BIT_VAR(pEntry->R_Counter);
		
	/* Construct EAPoL message - Group Msg 1*/
	ConstructEapolMsg(pEntry,
					  group_cipher, 
					  EAPOL_GROUP_MSG_1,
					  default_key,
					  (UCHAR *)gnonce_ptr,
					  TxTsc,
					  (UCHAR *)gtk_ptr,
					  NULL,
					  0,
				  	  pEapolFrame);

	DBGPRINT(RT_LOUD, ("[LOUD]<=== %s : send out Group Message 1 \n", __FUNCTION__));
	StartEapol(pAd, pEntry, (PUCHAR)pEapolFrame, CONV_ARRARY_TO_UINT16(pEapolFrame->Body_Len) + 4, TRUE);

	/* Trigger Retry Timer*/
	vPortFree(mpool);   
    return;
}


/*
    ==========================================================================
    Description:
        This is a function to initilize 4-way handshake
        
    Return:
         
    ==========================================================================
*/
VOID WPAStart4WayHS(
    IN PRTMP_ADAPTER    pAd, 
    IN PMGMTENTRY  	pEntry,
    IN ULONG			TimeInterval) 
{
	UCHAR   		*mpool;
    PEAPOL_PACKET	pEapolFrame;
	PUINT8			pBssid = NULL;
	UCHAR			group_cipher = Ndis802_11WEPDisabled;
//	UCHAR			apidx = 0;


    DBGPRINT(RT_LOUD, ("[LOUD]===> %s\n", __FUNCTION__));

#if 0
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS | fRTMP_ADAPTER_HALT_IN_PROGRESS))
	{
		DBGPRINT(RT_ERROR, ("[ERROR]WPAStart4WayHS : The interface is closed...\n"));
		return;		
	}
#endif

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{			
        if (!pEntry)
        {
			DBGPRINT(RT_ERROR, ("[ERR]WPAStart4WayHS : The entry doesn't exist.\n"));		
			return;
        }

		//apidx = 0;
		
		/* pointer to the corresponding position*/
		pBssid = pAd->essid;
		group_cipher = pAd->ApCfg.GroupKeyWepStatus;
	}	

	if (pBssid == NULL)
	{
		DBGPRINT(RT_ERROR, ("[ERR]WPAStart4WayHS : No corresponding Authenticator.\n"));		
		return;
    }


	/* Check the status*/
    if ((pEntry->WpaState > AS_PTKSTART) || (pEntry->WpaState < AS_INITPMK))
    {
        DBGPRINT(RT_ERROR, ("[ERR]%s : Not expect calling\n", __FUNCTION__));
        return;
    }
    
	/* Increment replay counter by 1*/
	ADD_ONE_To_64BIT_VAR(pEntry->R_Counter);
	
	/* Randomly generate ANonce		*/
	GenRandom(pAd, (UCHAR *)pBssid, pEntry->ANonce);	

	/* Allocate memory for output*/
	mpool = (PUCHAR)pvPortMalloc(TX_EAPOL_BUFFER);
	if (mpool == NULL)
    {
        DBGPRINT(RT_ERROR, ("[ERR]%s!!! : no memory!!!\n", __FUNCTION__));
        return;
    }

	pEapolFrame = (PEAPOL_PACKET)mpool;
	NdisZeroMemory(pEapolFrame, TX_EAPOL_BUFFER);
	
	/* Construct EAPoL message - Pairwise Msg 1*/
	/* EAPOL-Key(0,0,1,0,P,0,0,ANonce,0,DataKD_M1)		*/
	ConstructEapolMsg(pEntry,
					  group_cipher,
					  EAPOL_PAIR_MSG_1,
					  0,					/* Default key index*/
					  pEntry->ANonce,
					  NULL,					/* TxRSC*/
					  NULL,					/* GTK*/
					  NULL,					/* RSNIE*/
					  0,					/* RSNIE length	*/
					  pEapolFrame);

	/* If PMKID match in WPA2-enterprise mode, fill PMKID into Key data field and update PMK here	*/
	if ((pEntry->AuthMode == Ndis802_11AuthModeWPA2) /* && (pEntry->PMKID_CacheIdx != ENTRY_NOT_FOUND) */)
	{
		/* Fill in value for KDE */
		pEapolFrame->KeyDesc.KeyData[0] = 0xDD;
        pEapolFrame->KeyDesc.KeyData[2] = 0x00;
        pEapolFrame->KeyDesc.KeyData[3] = 0x0F;
        pEapolFrame->KeyDesc.KeyData[4] = 0xAC;
        pEapolFrame->KeyDesc.KeyData[5] = 0x04;  		
    	pEapolFrame->KeyDesc.KeyData[1] = 0x14;/* 4+LEN_PMKID*/
    	INC_UINT16_TO_ARRARY(pEapolFrame->KeyDesc.KeyDataLen, 6 + LEN_PMKID);    			
    	INC_UINT16_TO_ARRARY(pEapolFrame->Body_Len, 6 + LEN_PMKID);
	}


	/* Trigger Retry Timer*/
    RTMPSetTimer(&pEntry->RetryTimer, TimeInterval);		

	/* Update State*/
    pEntry->WpaState = AS_PTKSTART;

	DBGPRINT(RT_LOUD, ("[LOUD]<=== %s: send Msg1 of 4-way \n", __FUNCTION__));		
	StartEapol(pAd, pEntry, (PUCHAR)pEapolFrame, CONV_ARRARY_TO_UINT16(pEapolFrame->Body_Len) + 4, FALSE);

	vPortFree(mpool);
}



/*
	========================================================================
	
	Routine Description:
		Process Pairwise key Msg-1 of 4-way handshaking and send Msg-2 

	Arguments:
		pAd			Pointer	to our adapter
		Elem		Message body
		
	Return Value:
		None
		
	Note:
		
	========================================================================
*/
VOID PeerPairMsg1Action(
	IN PRTMP_ADAPTER    pAd, 
    IN PMGMTENTRY  	pEntry,
    IN PEAPOL_PACKET  	pEapolPacket)
{
	UCHAR				PTK[80];
	UCHAR   			*mpool;
    PEAPOL_PACKET		pEapolRspFrame;
	PUINT8				pCurrentAddr = NULL;
	PUINT8				pmk_ptr = NULL;
	UCHAR				group_cipher = Ndis802_11WEPDisabled;
	PUINT8				rsnie_ptr = NULL;
	UCHAR				rsnie_len = 0;
	   
	DBGPRINT(RT_LOUD, ("[LOUD]===> %s \n", __FUNCTION__));
	
	if (!pEntry)
		return;

	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{				
		pCurrentAddr = pAd->CurrentAddress;

		/*RT_CfgSetWPAPSKKey(pAd, 
					(PSTRING)pAd->StaCfg.WpaPassPhrase, 
					pAd->StaCfg.WpaPassPhraseLen,
					(PUCHAR)pAd->Ssid, 
					pAd->SsidLen, 
					pAd->StaCfg.PMK);*/
		
		pmk_ptr = pAd->StaCfg.PMK;
		group_cipher = pAd->StaCfg.GroupCipher;
		rsnie_ptr = pAd->StaCfg.RSN_IE;
		rsnie_len = pAd->StaCfg.RSNIE_Len;
	}

	if (pCurrentAddr == NULL)
		return;
	
	/* Sanity Check peer Pairwise message 1 - Replay Counter*/
	if (PeerWpaMessageSanity(pAd, pEapolPacket, (CONV_ARRARY_TO_UINT16(pEapolPacket->Body_Len) + LENGTH_EAPOL_H), EAPOL_PAIR_MSG_1, pEntry) == FALSE)
	{
		DBGPRINT(RT_ERROR, ("[ERR]%s: Sanity check failed\n", __FUNCTION__));
		return;
	}
	
	/* Store Replay counter, it will use to verify message 3 and construct message 2*/
	NdisMoveMemory(pEntry->R_Counter, pEapolPacket->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);		

	/* Store ANonce*/
	NdisMoveMemory(pEntry->ANonce, pEapolPacket->KeyDesc.KeyNonce, LEN_KEY_DESC_NONCE);
		
	/* Generate random SNonce*/
	GenRandom(pAd, (UCHAR *)pCurrentAddr, pEntry->SNonce);
	{
	    /* Calculate PTK(ANonce, SNonce)*/
	    WpaDerivePTK(pAd,
	    			pmk_ptr,
			     	pEntry->ANonce,
				 	pEntry->Addr, 
				 	pEntry->SNonce,
				 	pCurrentAddr, 
				    PTK, 
				    LEN_PTK);

		/* Save key to PTK entry*/
		NdisMoveMemory(pEntry->PTK, PTK, LEN_PTK);
	}		    
		
	/* Update WpaState*/
	pEntry->WpaState = AS_PTKINIT_NEGOTIATING;

	/* Allocate memory for output*/
	mpool = (PUCHAR)pvPortMalloc(TX_EAPOL_BUFFER);
	if (mpool == NULL)
    {
        DBGPRINT(RT_ERROR, ("[ERR]%s!!! : no memory!!!\n", __FUNCTION__));
        return;
    }
	NdisZeroMemory(mpool, TX_EAPOL_BUFFER);

	pEapolRspFrame = (PEAPOL_PACKET)mpool;
	NdisZeroMemory(pEapolRspFrame, TX_EAPOL_BUFFER);
	
	/* Construct EAPoL message - Pairwise Msg 2*/
	/*  EAPOL-Key(0,1,0,0,P,0,0,SNonce,MIC,DataKD_M2)*/
	ConstructEapolMsg(pEntry,
					  group_cipher,
					  EAPOL_PAIR_MSG_2,  
					  0,				/* DefaultKeyIdx*/
					  pEntry->SNonce,
					  NULL,				/* TxRsc*/
					  NULL,				/* GTK*/
					  (UCHAR *)rsnie_ptr,
					  rsnie_len,
					  pEapolRspFrame);

	DBGPRINT(RT_LOUD, ("[LOUD]<=== %s: send Msg2 of 4-way \n", __FUNCTION__));
	StartEapol(pAd, pEntry, (PUCHAR)pEapolRspFrame, CONV_ARRARY_TO_UINT16(pEapolRspFrame->Body_Len) + 4, FALSE);
	
	vPortFree(mpool);
		
	
}	


/*
    ==========================================================================
    Description:
        When receiving the second packet of 4-way pairwisekey handshake.
    Return:
    ==========================================================================
*/
VOID PeerPairMsg2Action(
    IN PRTMP_ADAPTER    pAd, 
    IN PMGMTENTRY  	pEntry,
    IN PEAPOL_PACKET  	pEapolPacket) 
{   
	UCHAR				PTK[80];
	UCHAR   			*mpool;
	PEAPOL_PACKET		pEapolFrame;
	UCHAR 				TxTsc[6] = {0};	
	PUINT8				pBssid = NULL;
	PUINT8				pmk_ptr = NULL;
	PUINT8				gtk_ptr = NULL;
	UCHAR				default_key = 0;
	UCHAR				group_cipher = Ndis802_11WEPDisabled;
	PUINT8				rsnie_ptr = NULL;
	UCHAR				rsnie_len = 0;
	BOOLEAN				Cancelled;

    DBGPRINT(RT_LOUD, ("[LOUD]===> %s \n", __FUNCTION__));

    if (!pEntry)
        return;

    /* check Entry in valid State*/
    if (pEntry->WpaState < AS_PTKSTART) {
		DBGPRINT(RT_ERROR, ("[ERR]%s: Error status (%d), It should be bigger than AS_PTKSTART(%d)\n",
					__FUNCTION__, pEntry->WpaState, AS_PTKSTART));
        return;
    }

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{			
		pBssid = pAd->essid;
		pmk_ptr = pAd->ApCfg.PMK;
		gtk_ptr = pAd->ApCfg.GTK;
		group_cipher = pAd->ApCfg.GroupKeyWepStatus;
		default_key = pAd->DefaultKeyId;

		/* Get Group TxTsc form Asic*/
		RTMPGetTxTscFromAsic(pAd, BSS0, TxTsc);

		if ((pEntry->AuthMode == Ndis802_11AuthModeWPA) || (pEntry->AuthMode == Ndis802_11AuthModeWPAPSK))
        {
			rsnie_len = pAd->ApCfg.RSNIE_Len[0];
			rsnie_ptr = &pAd->ApCfg.RSN_IE[0][0];
        }
        else
        {
            if ((pAd->ApCfg.AuthMode == Ndis802_11AuthModeWPA1PSKWPA2PSK) ||
				(pAd->ApCfg.AuthMode == Ndis802_11AuthModeWPA1WPA2)) 
            {
                rsnie_len = pAd->ApCfg.RSNIE_Len[1];
                rsnie_ptr = &pAd->ApCfg.RSN_IE[1][0];
            }
            else
            {
                rsnie_len = pAd->ApCfg.RSNIE_Len[0];
                rsnie_ptr = &pAd->ApCfg.RSN_IE[0][0];
            }
        }
	}

	/* Store SNonce*/
	NdisMoveMemory(pEntry->SNonce, pEapolPacket->KeyDesc.KeyNonce, LEN_KEY_DESC_NONCE);
	{
		/* Derive PTK*/
		if ((pmk_ptr == NULL) || (pBssid == NULL))
		{
			DBGPRINT(RT_ERROR,("[ERR]%s: pmk_ptr or pBssid == NULL!\n", __FUNCTION__));
			return;
		}

		WpaDerivePTK(pAd, 
					(UCHAR *)pmk_ptr,  
					pEntry->ANonce, 		/* ANONCE*/
					(UCHAR *)pBssid, 
					pEntry->SNonce, 		/* SNONCE*/
					pEntry->Addr, 
					PTK,
					LEN_PTK); 		

    	NdisMoveMemory(pEntry->PTK, PTK, LEN_PTK);
	}

	/* Sanity Check peer Pairwise message 2 - Replay Counter, MIC, RSNIE*/
	if (PeerWpaMessageSanity(pAd, pEapolPacket, (CONV_ARRARY_TO_UINT16(pEapolPacket->Body_Len) + LENGTH_EAPOL_H), EAPOL_PAIR_MSG_2, pEntry) == FALSE)
	{
		DBGPRINT(RT_ERROR, ("[ERR]%s: Sanity check failed\n", __FUNCTION__));
		return;
	}

    do
    {
		/* Allocate memory for input*/
		mpool = (PUCHAR)pvPortMalloc(TX_EAPOL_BUFFER);
		if (mpool == NULL)
	    {
	        DBGPRINT(RT_ERROR, ("!!!%s : no memory!!!\n", __FUNCTION__));
	        return;
	    }

		pEapolFrame = (PEAPOL_PACKET)mpool;
		NdisZeroMemory(pEapolFrame, TX_EAPOL_BUFFER);
	    
        /* delete retry timer*/
		RTMPCancelTimer(&pEntry->RetryTimer, &Cancelled);

		/* Change state*/
    	 pEntry->WpaState = AS_PTKINIT_NEGOTIATING;

		/* Increment replay counter by 1*/
		ADD_ONE_To_64BIT_VAR(pEntry->R_Counter);

		/* Construct EAPoL message - Pairwise Msg 3*/
		ConstructEapolMsg(pEntry,
						  group_cipher,
						  EAPOL_PAIR_MSG_3,
						  default_key,
						  pEntry->ANonce,
						  TxTsc,
						  (UCHAR *)gtk_ptr,
						  (UCHAR *)rsnie_ptr,
						  rsnie_len,
						  pEapolFrame);

		DBGPRINT(RT_LOUD, ("[LOUD]<=== %s: send Msg3 of 4-way \n", __FUNCTION__));		
		StartEapol(pAd, pEntry, (PUCHAR)pEapolFrame, CONV_ARRARY_TO_UINT16(pEapolFrame->Body_Len) + 4, FALSE);

        pEntry->ReTryCounter = PEER_MSG3_RETRY_TIMER_CTR;
		RTMPSetTimer(&pEntry->RetryTimer, PEER_MSG3_RETRY_EXEC_INTV);

		/* Update State*/
        pEntry->WpaState = AS_PTKINIT_NEGOTIATING;
	
		vPortFree(mpool);
	
    }while(FALSE);

	
}

/*
	========================================================================
	
	Routine Description:
		Process Pairwise key Msg 3 of 4-way handshaking and send Msg 4 

	Arguments:
		pAd	Pointer	to our adapter
		Elem		Message body
		
	Return Value:
		None
		
	Note:
		
	========================================================================
*/
VOID PeerPairMsg3Action(
    IN PRTMP_ADAPTER    pAd, 
    IN PMGMTENTRY  	pEntry,
    IN PEAPOL_PACKET  	pEapolPacket) 
{
	UCHAR				*mpool;
	PEAPOL_PACKET		pEapolFrame;
	PEAPOL_PACKET		pMsg3;		
	PUINT8				pCurrentAddr = NULL;
	UCHAR				group_cipher = Ndis802_11WEPDisabled;

	DBGPRINT(RT_LOUD, ("[LOUD]===> %s \n", __FUNCTION__));
	
	if (!pEntry)
		return;

	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{				
		pCurrentAddr = pAd->CurrentAddress;
		group_cipher = pAd->StaCfg.GroupCipher;
	}

	if (pCurrentAddr == NULL)
		return;

	pMsg3 = pEapolPacket;
	
	/* Sanity Check peer Pairwise message 3 - Replay Counter, MIC, RSNIE*/
	if (PeerWpaMessageSanity(pAd, pEapolPacket, (CONV_ARRARY_TO_UINT16(pEapolPacket->Body_Len) + LENGTH_EAPOL_H), EAPOL_PAIR_MSG_3, pEntry) == FALSE)
	{
		DBGPRINT(RT_ERROR, ("[ERR]%s: Sanity check failed\n", __FUNCTION__));
		return;
	}

	
	/* Save Replay counter, it will use construct message 4*/
	NdisMoveMemory(pEntry->R_Counter, pMsg3->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);

	/* Double check ANonce*/
	if (!NdisEqualMemory(pEntry->ANonce, pMsg3->KeyDesc.KeyNonce, LEN_KEY_DESC_NONCE))
	{
		return;
	}

	/* Allocate memory for output*/
	mpool = (PUCHAR)pvPortMalloc(TX_EAPOL_BUFFER);
	if (mpool == NULL)
    {
        DBGPRINT(RT_ERROR, ("[ERR]%s!!!: no memory!!!\n", __FUNCTION__));
        return;
    }

	pEapolFrame = (PEAPOL_PACKET)mpool;
	NdisZeroMemory(pEapolFrame, TX_EAPOL_BUFFER);

	/* Construct EAPoL message - Pairwise Msg 4*/
	ConstructEapolMsg(pEntry,
					  group_cipher,
					  EAPOL_PAIR_MSG_4,  
					  0,					/* group key index not used in message 4*/
					  NULL,					/* Nonce not used in message 4*/
					  NULL,					/* TxRSC not used in message 4*/
					  NULL,					/* GTK not used in message 4*/
					  NULL,					/* RSN IE not used in message 4*/
					  0,
					  pEapolFrame);

	/* Update WpaState*/
	pEntry->WpaState = AS_PTKINITDONE;	 	
	/* Update pairwise key		*/

	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		{
			NdisMoveMemory(pAd->StaCfg.PTK, pEntry->PTK, LEN_PTK);
		
			WPAInstallPairwiseKey(pAd, 
								  BSS0, 
								  pEntry, 
								  FALSE);
			NdisMoveMemory(&pAd->SharedKey[BSS0][0], &pEntry->PairwiseKey, sizeof(CIPHER_KEY));
		}
	}

	/* open 802.1x port control and privacy filter*/
	if (pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK || 
		pEntry->AuthMode == Ndis802_11AuthModeWPA2)
	{
//		pEntry->PortSecured = WPA_802_1X_PORT_SECURED;
//		pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;	
#ifdef CONFIG_MULTI_CHANNEL
		if (pAd->Multi_Channel_Enable == TRUE)
		{
			MultiChannelTimerStart(pAd,pEntry);
		}
#endif /*CONFIG_MULTI_CHANNEL*/

		DBGPRINT(RT_INFO, ("[INFO]%s: AuthMode(%s) PairwiseCipher(%s) GroupCipher(%s) \n",
									__FUNCTION__,
									GetAuthMode(pEntry->AuthMode),
									GetEncryptType(pEntry->WepStatus),
									GetEncryptType(group_cipher)));
	}
	
	DBGPRINT(RT_LOUD, ("[LOUD]<=== %s: send Msg4 of 4-way \n", __FUNCTION__));
	StartEapol(pAd, pEntry, (PUCHAR)pEapolFrame, CONV_ARRARY_TO_UINT16(pEapolFrame->Body_Len) + 4, FALSE);
	vPortFree(mpool);

	MLME_Startdhcpc(pAd);
}

/*
    ==========================================================================
    Description:
        When receiving the last packet of 4-way pairwisekey handshake.
        Initilize 2-way groupkey handshake following.
    Return:
    ==========================================================================
*/
VOID PeerPairMsg4Action(
    IN PRTMP_ADAPTER    pAd, 
    IN PMGMTENTRY		pEntry,
    IN PEAPOL_PACKET  	pEapolPacket) 
{    
	UCHAR				group_cipher = Ndis802_11WEPDisabled;
	BOOLEAN				Cancelled;

    DBGPRINT(RT_LOUD, ("[LOUD]===> %s\n", __FUNCTION__));

    do
    {
        if (!pEntry)
            break;

        if (pEntry->WpaState < AS_PTKINIT_NEGOTIATING) {
			DBGPRINT(RT_ERROR, ("[ERR]%s: Error status (%d), It should be bigger than AS_PTKINIT_NEGOTIATING(%d)\n",
								__FUNCTION__, pEntry->WpaState, AS_PTKINIT_NEGOTIATING));
            break;
        }

		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			group_cipher = pAd->ApCfg.GroupKeyWepStatus;			
		}

        /* Sanity Check peer Pairwise message 4 - Replay Counter, MIC*/
		if (PeerWpaMessageSanity(pAd, pEapolPacket, (CONV_ARRARY_TO_UINT16(pEapolPacket->Body_Len) + LENGTH_EAPOL_H), EAPOL_PAIR_MSG_4, pEntry) == FALSE)
		{
			DBGPRINT(RT_ERROR, ("[ERR]%s: Sanity check failed\n", __FUNCTION__));
			break;
		}


        /* 3. Install pairwise key */
		WPAInstallPairwiseKey(pAd, BSS0, pEntry, TRUE);
        
        /* 4. upgrade state */
//    	pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
		pEntry->WpaState = AS_PTKINITDONE;
//		pEntry->PortSecured = WPA_802_1X_PORT_SECURED;
        
		if (pEntry->AuthMode == Ndis802_11AuthModeWPA2 || 
			pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK)
		{
			pEntry->GTKState = REKEY_ESTABLISHED;
			RTMPCancelTimer(&pEntry->RetryTimer, &Cancelled);
	 
	        DBGPRINT(RT_INFO, ("[INFO]%s: AP SETKEYS DONE - WPA2, AuthMode(%d)=%s, WepStatus(%d)=%s, GroupWepStatus(%d)=%s\n\n", 
	        							__FUNCTION__, 
	        							pEntry->AuthMode, 	GetAuthMode(pEntry->AuthMode), 
										pEntry->WepStatus, 	GetEncryptType(pEntry->WepStatus), 
										group_cipher, 		GetEncryptType(group_cipher)));
		}
		else
		{
        	/* 5. init Group 2-way handshake if necessary.*/
	        WPAStart2WayGroupHS(pAd, pEntry);

	       	pEntry->ReTryCounter = GROUP_MSG1_RETRY_TIMER_CTR;
			RTMPSetTimer(&pEntry->RetryTimer, PEER_MSG3_RETRY_EXEC_INTV);
		}
    }while(FALSE);
    
}

     
/*
	========================================================================
	
	Routine Description:
		Process Group key 2-way handshaking

	Arguments:
		pAd	Pointer	to our adapter
		Elem		Message body
		
	Return Value:
		None
		
	Note:
		
	========================================================================
*/
VOID	PeerGroupMsg1Action(
	IN PRTMP_ADAPTER    pAd, 
    IN PMGMTENTRY  	pEntry,
    IN PEAPOL_PACKET	pEapolPacket)
{
	UCHAR				*mpool;
	PEAPOL_PACKET		pEapolFrame;
	UCHAR				default_key = 0;
	UCHAR				group_cipher = Ndis802_11WEPDisabled;
	PUINT8				pCurrentAddr = NULL;

	DBGPRINT(RT_LOUD, ("[LOUD]===>%s \n", __FUNCTION__));

	if (!pEntry)
        return;

	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{				
		pCurrentAddr = pAd->CurrentAddress;
		group_cipher = pAd->StaCfg.GroupCipher;
		/* Group key*/
		default_key = pAd->StaCfg.DefaultKeyId;
	}	

	if (pCurrentAddr == NULL)
		return;

	/* Sanity Check peer group message 1 - Replay Counter, MIC, RSNIE*/
	if (PeerWpaMessageSanity(pAd, pEapolPacket, (CONV_ARRARY_TO_UINT16(pEapolPacket->Body_Len) + LENGTH_EAPOL_H), EAPOL_GROUP_MSG_1, pEntry) == FALSE)
		return;

	/* Save Replay counter, it will use to construct message 2*/
	NdisMoveMemory(pEntry->R_Counter, pEapolPacket->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);	

	/* Allocate memory for output*/
	mpool = (PUCHAR)pvPortMalloc(TX_EAPOL_BUFFER);
	if (mpool == NULL)
    {
        DBGPRINT(RT_ERROR, ("!!!%s : no memory!!!\n", __FUNCTION__));
        return;
    }

	pEapolFrame = (PEAPOL_PACKET)mpool;
	NdisZeroMemory(pEapolFrame, TX_EAPOL_BUFFER);


	/* Construct EAPoL message - Group Msg 2*/
	ConstructEapolMsg(pEntry,
					  group_cipher,
					  EAPOL_GROUP_MSG_2,  
					  default_key,
					  NULL,					/* Nonce not used*/
					  NULL,					/* TxRSC not used*/
					  NULL,					/* GTK not used*/
					  NULL,					/* RSN IE not used*/
					  0,
					  pEapolFrame);
					
    /* open 802.1x port control and privacy filter*/
//	pEntry->PortSecured = WPA_802_1X_PORT_SECURED;
//	pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
	
	DBGPRINT(RT_INFO, ("[INFO]%s: AuthMode(%s) PairwiseCipher(%s) GroupCipher(%s) \n",
						__FUNCTION__, 
						GetAuthMode(pEntry->AuthMode),
						GetEncryptType(pEntry->WepStatus),
						GetEncryptType(group_cipher)));
	
	DBGPRINT(RT_LOUD, ("[LOUD]<=== %s: send group message 2\n", __FUNCTION__));

	StartEapol(pAd, pEntry, (PUCHAR)pEapolFrame, CONV_ARRARY_TO_UINT16(pEapolFrame->Body_Len) + 4, TRUE);
	vPortFree(mpool);
}	




/*
    ==========================================================================
    Description:
        When receiving the last packet of 2-way groupkey handshake.
    Return:
    ==========================================================================
*/
VOID PeerGroupMsg2Action(
    IN PRTMP_ADAPTER    pAd, 
    IN PMGMTENTRY  	pEntry,
    IN PEAPOL_PACKET	pEapolPacket)
{
	UCHAR				group_cipher = Ndis802_11WEPDisabled;
	BOOLEAN				Cancelled;

	DBGPRINT(RT_LOUD, ("[LOUD]===> %s \n", __FUNCTION__));

    if (!pEntry)
        return;
                        
 	if (pEntry->WpaState != AS_PTKINITDONE)
  		return;

    do
    {
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			group_cipher = pAd->ApCfg.GroupKeyWepStatus;			
		}
        
		/* Sanity Check peer group message 2 - Replay Counter, MIC*/
		if (PeerWpaMessageSanity(pAd, pEapolPacket, (CONV_ARRARY_TO_UINT16(pEapolPacket->Body_Len) + LENGTH_EAPOL_H), EAPOL_GROUP_MSG_2, pEntry) == FALSE)
      		break;

        /* 3.  upgrade state*/
		RTMPCancelTimer(&pEntry->RetryTimer, &Cancelled);
     	pEntry->GTKState = REKEY_ESTABLISHED;
        
		if ((pEntry->AuthMode == Ndis802_11AuthModeWPA2) || (pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK))
		{
			DBGPRINT(RT_INFO, ("[INFO]%s: AP SETKEYS DONE (id:%d) - WPA2, AuthMode(%d)=%s, WepStatus(%d)=%s, GroupWepStatus(%d)=%s\n\n",
										__FUNCTION__,
										pEntry->Aid,
										pEntry->AuthMode, GetAuthMode(pEntry->AuthMode), 
										pEntry->WepStatus, GetEncryptType(pEntry->WepStatus), 
										group_cipher, GetEncryptType(group_cipher)));
		}
		else
		{
			/* send wireless event - for set key done WPA*/
//			RTMPSendWirelessEvent(pAd, IW_SET_KEY_DONE_WPA1_EVENT_FLAG, pEntry->Addr, pEntry->apidx, 0); 

        	DBGPRINT(RT_INFO, ("[INFO]%s: AP SETKEYS DONE (id:%d) - WPA1, AuthMode(%d)=%s, WepStatus(%d)=%s, GroupWepStatus(%d)=%s\n\n",
        								__FUNCTION__,
        								pEntry->Aid,
										pEntry->AuthMode, GetAuthMode(pEntry->AuthMode), 
										pEntry->WepStatus, GetEncryptType(pEntry->WepStatus), 
										group_cipher, GetEncryptType(group_cipher)));
		}	
    }while(FALSE);  
}


#define MLME_TASK_EXEC_INTV        	100/*200*/       /* */
#define MLME_TASK_EXEC_MULTIPLE    	10  /*5*/       /* MLME_TASK_EXEC_MULTIPLE * MLME_TASK_EXEC_INTV = 1 sec */

/*
    ==========================================================================
    Description:
        Function to handle countermeasures active attack.  Init 60-sec timer if necessary.
    Return:
    ==========================================================================
*/
VOID HandleCounterMeasure(
    IN PRTMP_ADAPTER    pAd, 
    IN PMGMTENTRY 		pEntry) 
{
    BOOLEAN     Cancelled;

    if (!pEntry)
        return;

	/* if entry not set key done, ignore this RX MIC ERROR */
	if ((pEntry->WpaState < AS_PTKINITDONE) || (pEntry->GTKState != REKEY_ESTABLISHED))
		return;

	DBGPRINT(RT_WARN, ("[WARN]%s: HandleCounterMeasure ===> \n", __FUNCTION__));

    /* record which entry causes this MIC error, if this entry sends disauth/disassoc, AP doesn't need to log the CM */
    pEntry->CMTimerRunning = TRUE;
    pAd->ApCfg.MICFailureCounter++;
	
    if (pAd->ApCfg.CMTimerRunning == TRUE)
    {
        DBGPRINT(RT_ERROR, ("[ERR]%s: Receive CM Attack Twice within 60 seconds ====>>> \n", __FUNCTION__));
       		
        /* renew GTK */
		GenRandom(pAd, pAd->essid, pAd->ApCfg.GNonce);

		/* Cancel CounterMeasure Timer */
		RTMPCancelTimer(&pAd->ApCfg.CounterMeasureTimer, &Cancelled);
		pAd->ApCfg.CMTimerRunning = FALSE;

		pEntry = pAd->pFirstLinkNode;
		while (pEntry != NULL)
		{
			SendDeAuthReq(pAd, pEntry->Addr, &pAd->TxContext, REASON_4_WAY_TIMEOUT);
			MGMTENTRY_Del(pAd, pEntry->Addr);
			pEntry = pEntry->next_node;
		}
        
        /* Further,  ban all Class 3 DATA transportation for  a period 0f 60 sec */
        /* disallow new association , too */
        pAd->ApCfg.BANClass3Data = TRUE;        

        /* check how many entry left...  should be zero */
        /*pAd->ApCfg.MBSSID[pEntry->apidx].GKeyDoneStations = pAd->MacTab.Size; */
        /*DBGPRINT(RT_DEBUG_TRACE, ("GKeyDoneStations=%d \n", pAd->ApCfg.MBSSID[pEntry->apidx].GKeyDoneStations)); */
    }

	RTMPSetTimer(&pAd->ApCfg.CounterMeasureTimer, 60 * MLME_TASK_EXEC_INTV * MLME_TASK_EXEC_MULTIPLE);
    pAd->ApCfg.CMTimerRunning = TRUE;

	/* FIXME: to do record fail time */
//  pAd->ApCfg.PrevaMICFailTime = pAd->ApCfg.aMICFailTime;
//	RTMP_GetCurrentSystemTime(&pAd->ApCfg.aMICFailTime);
}


/*
    ==========================================================================
    Description:
        Timer execution function for periodically updating group key.
    Return:
    ==========================================================================
*/  
VOID WpaGREKEYPeriodicExec(
    IN PVOID FunctionContext)
{
    ULONG           	temp_counter = 0;
    PRTMP_ADAPTER   	pAd = (PRTMP_ADAPTER)pvTimerGetTimerID(FunctionContext);
	PAP_ADMIN_CONFIG	pApCfg = &pAd->ApCfg;

	
	if (pApCfg->AuthMode < Ndis802_11AuthModeWPA || 
		pApCfg->AuthMode > Ndis802_11AuthModeWPA1PSKWPA2PSK)
		return;
	
    if ((pApCfg->WPAREKEY.ReKeyMethod == TIME_REKEY) && (pApCfg->REKEYCOUNTER < 0xffffffff))
        temp_counter = (++pApCfg->REKEYCOUNTER);
    /* REKEYCOUNTER is incremented every MCAST packets transmitted, */
    /* But the unit of Rekeyinterval is 1K packets */
    else if (pApCfg->WPAREKEY.ReKeyMethod == PKT_REKEY)
        temp_counter = pApCfg->REKEYCOUNTER/1000;
    else
    {
		return;
    }

	DBGPRINT(RT_LOUD, ("[LOUD]%s: accumulation counter = %d\n", __FUNCTION__, temp_counter));
	
	if (temp_counter > (pApCfg->WPAREKEY.ReKeyInterval))
    {
        pApCfg->REKEYCOUNTER = 0;
		pApCfg->RekeyCountDown = 3;
        DBGPRINT(RT_LOUD, ("[LOUD]%s: Rekey Interval Excess, GKeyDoneStations=%d\n", __FUNCTION__, pApCfg->StaCount));
        
        /* take turn updating different groupkey index, */
		if ((pApCfg->StaCount) > 0)
        {
			/* change key index */
			pAd->DefaultKeyId = (pAd->DefaultKeyId == 1) ? 2 : 1;         
	
			/* Generate GNonce randomly */
			GenRandom(pAd, pAd->essid, pApCfg->GNonce);

			/* Update GTK */
			WpaDeriveGTK(pApCfg->GMK, 
							pApCfg->GNonce, 
							pAd->essid, 
							pApCfg->GTK, 
							LEN_TKIP_GTK);

			/* Process 2-way handshaking */
			PMGMTENTRY pEntry = pAd->pFirstLinkNode;
			while (pEntry != NULL)
			{
				if (pEntry->WpaState == AS_PTKINITDONE) 
                {
					pEntry->GTKState = REKEY_NEGOTIATING;
                    DBGPRINT(RT_TRACE, ("[TRACE]%s: Rekey interval excess, Update Group Key for  (%d)%02X-%02X-%02X-%02X-%02X-%02X\n", 
										__FUNCTION__, pEntry->Aid, PRINT_MAC(pEntry->Addr)));
                	WPAStart2WayGroupHS(pAd, pEntry);

					pEntry->ReTryCounter = GROUP_MSG1_RETRY_TIMER_CTR;
					RTMPSetTimer(&pEntry->RetryTimer, PEER_MSG3_RETRY_EXEC_INTV);
				}
				pEntry = pEntry->next_node;
			}
		}
	}       

	/* Use countdown to ensure the 2-way handshaking had completed */
	if (pApCfg->RekeyCountDown > 0)
	{
		pApCfg->RekeyCountDown--;
		if (pApCfg->RekeyCountDown == 0)
		{
			USHORT	Wcid;
			/* Get a specific WCID to record this MBSS key attribute */
			GET_GroupKey_WCID(pAd, Wcid, BSS0);

			/* Install shared key table */
			WPAInstallSharedKey(pAd, 
								pApCfg->GroupKeyWepStatus, 
								BSS0, 
								pAd->DefaultKeyId, 
								Wcid, 
								TRUE, 
								pApCfg->GTK,
								LEN_TKIP_GTK);
		}
	}
}



/*
    ==========================================================================
    Description:
       Set group re-key timer

    Return:
		
    ==========================================================================
*/
VOID WpaAPSetGroupRekeyAction(
	IN  PRTMP_ADAPTER   pAd)
{
	DBGPRINT(RT_TRACE, ("[TRACE]%s ===> \n", __FUNCTION__));
	
	PAP_ADMIN_CONFIG pApCfg = &pAd->ApCfg;

	if ((pApCfg->WepStatus == Ndis802_11Encryption2Enabled) ||
		(pApCfg->WepStatus == Ndis802_11Encryption3Enabled) ||
		(pApCfg->WepStatus == Ndis802_11Encryption4Enabled))
	{
		/* Group rekey related */	
		
		if ((pApCfg->WPAREKEY.ReKeyInterval != 0) 
			&& ((pApCfg->WPAREKEY.ReKeyMethod == TIME_REKEY) || (
				pApCfg->WPAREKEY.ReKeyMethod == PKT_REKEY))) 
		{
			/* Regularly check the timer */
			if (pApCfg->REKEYTimerRunning == FALSE)
			{
				RTMPSetTimer(&pApCfg->REKEYTimer, GROUP_KEY_UPDATE_EXEC_INTV);

				pApCfg->REKEYTimerRunning = TRUE;
				pApCfg->REKEYCOUNTER = 0;
			}
			DBGPRINT(RT_INFO, ("[INFO]%s: Group rekey method= %ld , interval = %d\n",
										__FUNCTION__, pApCfg->WPAREKEY.ReKeyMethod,
										pApCfg->WPAREKEY.ReKeyInterval));
		}
		else
			pApCfg->REKEYTimerRunning = FALSE;
	}
}



/*
    ==========================================================================
    Description:
        countermeasures active attack timer execution
    Return:
    ==========================================================================
*/
VOID CMTimerExec(
    IN PVOID FunctionContext) 
{
	int mic_fail_cnt = 0;
    PRTMP_ADAPTER   pAd = (PRTMP_ADAPTER)pvTimerGetTimerID(FunctionContext);
	PMGMTENTRY pEntry = NULL;
        
    pAd->ApCfg.BANClass3Data = FALSE;

	pEntry = pAd->pFirstLinkNode;
	while (pEntry != NULL)
	{
		if (pEntry->CMTimerRunning == TRUE) 
		{
			mic_fail_cnt++;
			pEntry->CMTimerRunning = FALSE;
		}
		pEntry = pEntry->next_node;
	}

    if (mic_fail_cnt > 1)
        DBGPRINT(RT_ERROR, ("[ERR]%s: Find more than one entry which generated MIC Fail ..  \n", __FUNCTION__));

    pAd->ApCfg.CMTimerRunning = FALSE;
}


   
VOID WPARetryExec(
    IN PVOID FunctionContext)
{
	PMGMTENTRY pEntry = (PMGMTENTRY)pvTimerGetTimerID(FunctionContext);
	
    if (pEntry)
    {
        PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pEntry->pAd;
        
        pEntry->ReTryCounter++;
        DBGPRINT(RT_WARN, ("[WARN]%s---> ReTryCounter=%d, WpaState=%d \n", 
							__FUNCTION__, pEntry->ReTryCounter, pEntry->WpaState));

        switch (pEntry->AuthMode)
        {
			case Ndis802_11AuthModeWPA:
            case Ndis802_11AuthModeWPAPSK:
			case Ndis802_11AuthModeWPA2:
            case Ndis802_11AuthModeWPA2PSK:
				/* 1. GTK already retried, give up and disconnect client. */
                if (pEntry->ReTryCounter > (GROUP_MSG1_RETRY_TIMER_CTR + 1))
                {					
                    DBGPRINT(RT_WARN, ("[WARN]%s::Group Key HS exceed retry count, Disassociate client, ReTryCounter %d\n", 
										__FUNCTION__, pEntry->ReTryCounter));
                    SendDeAuthReq(pAd, pEntry->Addr, &pAd->TxContext, REASON_GROUP_KEY_HS_TIMEOUT);
					MGMTENTRY_Del(pAd, pEntry->Addr);
                }
				/* 2. Retry GTK. */
                else if (pEntry->ReTryCounter > GROUP_MSG1_RETRY_TIMER_CTR)
                {
                    DBGPRINT(RT_WARN, ("[WARN]%s::ReTry 2-way group-key Handshake \n", __FUNCTION__));
                    if (pEntry->GTKState == REKEY_NEGOTIATING)
                    {
                        WPAStart2WayGroupHS(pAd, pEntry);
						RTMPSetTimer(&pEntry->RetryTimer, PEER_MSG3_RETRY_EXEC_INTV);
                    }
                }
				/* 3. 4-way message 1 retried more than three times. Disconnect client */
                else if (pEntry->ReTryCounter > (PEER_MSG1_RETRY_TIMER_CTR + 3))
                {
                    DBGPRINT(RT_WARN, ("[WARN]%s::MSG1 timeout, pEntry->ReTryCounter = %d\n", __FUNCTION__, pEntry->ReTryCounter));
                    SendDeAuthReq(pAd, pEntry->Addr, &pAd->TxContext, REASON_4_WAY_TIMEOUT);
					MGMTENTRY_Del(pAd, pEntry->Addr);
                }
				/* 4. Retry 4 way message 1, the last try, the timeout is 3 sec for EAPOL-Start */
                else if (pEntry->ReTryCounter == (PEER_MSG1_RETRY_TIMER_CTR + 3))                
                {
                    DBGPRINT(RT_WARN, ("[WARN]%s::Retry MSG1, the last try\n", __FUNCTION__));
                    WPAStart4WayHS(pAd , pEntry, PEER_MSG3_RETRY_EXEC_INTV);
                }
				/* 4. Retry 4 way message 1 */
                else if (pEntry->ReTryCounter < (PEER_MSG1_RETRY_TIMER_CTR + 3))
                {
                    if ((pEntry->WpaState == AS_PTKSTART) || (pEntry->WpaState == AS_INITPSK) || (pEntry->WpaState == AS_INITPMK))
                    {
                        DBGPRINT(RT_WARN, ("[WARN]%s::ReTry MSG1 of 4-way Handshake\n", __FUNCTION__));
                        WPAStart4WayHS(pAd, pEntry, PEER_MSG1_RETRY_EXEC_INTV);
                    }
                }
                break;

            default:
                break;
        }
    }
}


VOID WpaApStartup(
	IN PRTMP_ADAPTER    pAd)
{
	USHORT Wcid = 0;
	DBGPRINT(RT_LOUD, ("[LOUD]%s ===> \n", __FUNCTION__));
	
	/* Generate the corresponding RSNIE */
	RTMPMakeRSNIE(pAd, pAd->ApCfg.AuthMode, pAd->ApCfg.WepStatus, BSS0);

	/* Generate GMK and GNonce randomly per MBSS */
	GenRandom(pAd, pAd->essid, pAd->ApCfg.GMK);

	GenRandom(pAd, pAd->essid, pAd->ApCfg.GNonce);

	RT_CfgSetWPAPSKKey(pAd, 
						(PSTRING)pAd->SharedKey[BSS0][pAd->DefaultKeyId].Key, 
						pAd->SharedKey[BSS0][pAd->DefaultKeyId].KeyLen, 
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
	
	/* Install Timer control for AP mode */
#ifdef TIMER_DEBUG
		RTMPInitTimer(pAd, &pAd->ApCfg.CounterMeasureTimer, (PCHAR)"CMTimerExec", CMTimerExec, (PVOID)pAd, FALSE);
#else
		RTMPInitTimer(pAd, &pAd->ApCfg.CounterMeasureTimer, CMTimerExec, (PVOID)pAd, FALSE);
		DBGPRINT(RT_TRACE,("[TRACE]%s: Create CounterMeasureTimer\n", __FUNCTION__));
#endif	

#ifdef TIMER_DEBUG
		RTMPInitTimer(pAd, &pAd->ApCfg.REKEYTimer, (PCHAR)"WpaGREKEYPeriodicExec", WpaGREKEYPeriodicExec, (PVOID)pAd, TRUE);
#else
		RTMPInitTimer(pAd, &pAd->ApCfg.REKEYTimer, WpaGREKEYPeriodicExec, (PVOID)pAd,  TRUE);
		DBGPRINT(RT_TRACE,("[TRACE]%s: Create WpaGREKEYPeriodicExec\n", __FUNCTION__));
#endif

	pAd->ApCfg.WPAREKEY.ReKeyInterval	= 1800;
	pAd->ApCfg.WPAREKEY.ReKeyMethod 	= TIME_REKEY;

//	WpaAPSetGroupRekeyAction(pAd);
}	


VOID WpaApStop(
	IN PRTMP_ADAPTER    pAd)
{
	BOOLEAN     Cancelled;

	DBGPRINT(RT_LOUD, ("[LOUD]%s ===> \n", __FUNCTION__));

	pAd->ApCfg.WPAREKEY.ReKeyInterval 	= 0;
	pAd->ApCfg.WPAREKEY.ReKeyMethod 	= DISABLE_REKEY;

	if (pAd->ApCfg.REKEYTimerRunning == TRUE)
	{
		RTMPCancelTimer(&pAd->ApCfg.REKEYTimer, &Cancelled);
		pAd->ApCfg.REKEYTimerRunning = FALSE;
		RTMPReleaseTimer(&pAd->ApCfg.REKEYTimer, &Cancelled);
	}

	if (pAd->ApCfg.CMTimerRunning == TRUE)
	{
		RTMPCancelTimer(&pAd->ApCfg.CounterMeasureTimer, &Cancelled);
		pAd->ApCfg.CMTimerRunning = FALSE;
		RTMPReleaseTimer(&pAd->ApCfg.CounterMeasureTimer, &Cancelled);
	}
}


/*
    ==========================================================================
    Description:
       Start 4-way HS when rcv EAPOL_START which may create by our driver in assoc.c
    Return:
    ==========================================================================
*/
VOID WpaEAPOLStartAction(
    IN PRTMP_ADAPTER    pAd,
    IN PMGMTENTRY		pEntry) 
{   
    DBGPRINT(RT_LOUD, ("[LOUD]%s ===> \n", __FUNCTION__));
    
//	pEntry = MGMTENTRY_GetNode(pAd, pDest_Addr);    
    if (pEntry) 
    {
//		DBGPRINT(RT_TRACE, (" PortSecured(%d), WpaState(%d), AuthMode(%d), PMKID_CacheIdx(%d) \n", pEntry->PortSecured, pEntry->WpaState, pEntry->AuthMode, pEntry->PMKID_CacheIdx));
//    	if ((pEntry->PortSecured == WPA_802_1X_PORT_NOT_SECURED)
//			&& (pEntry->WpaState < AS_PTKSTART)
//         	&& ((pEntry->AuthMode == Ndis802_11AuthModeWPAPSK) || (pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK) || ((pEntry->AuthMode == Ndis802_11AuthModeWPA2) && (pEntry->PMKID_CacheIdx != ENTRY_NOT_FOUND))))

		if (((pEntry->AuthMode == Ndis802_11AuthModeWPAPSK) || pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK) || ((pEntry->AuthMode == Ndis802_11AuthModeWPA2)))
		{
//        	pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
        	pEntry->WpaState = AS_INITPSK;
//        	pEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
            NdisZeroMemory(pEntry->R_Counter, sizeof(pEntry->R_Counter));
        	pEntry->ReTryCounter = PEER_MSG1_RETRY_TIMER_CTR;
            
            WPAStart4WayHS(pAd, pEntry, PEER_MSG1_RETRY_EXEC_INTV);
        }
    }
}


/*
    ==========================================================================
    [Pairwise]
	Message 1: (MIC, ACK, Replay counter) = (0, 1, 0)
	Message 2: (MIC, ACK, Replay counter) = (1, 0, 0)
	Message 3: (MIC, ACK, Replay counter) = (1, 1, 1)
	Message 4: (MIC, ACK, Replay counter) = (1, 0, 1)

	[Group]
	Message 1: (MIC, ACK, Replay counter) = (1, 1, 2)
	Message 2: (MIC, ACK, Replay counter) = (1, 0, 2)	
    
    Description:
        This is state machine function. 
        When receiving EAPOL packets which is  for 802.1x key management. 
        Use both in WPA, and WPAPSK case. 
        In this function, further dispatch to different functions according to the received packet.  3 categories are : 
          1.  normal 4-way pairwisekey and 2-way groupkey handshake
          2.  MIC error (Countermeasures attack)  report packet from STA.
          3.  Request for pairwise/group key update from STA
    Return:
    ==========================================================================
*/
VOID WpaEAPOLKeyAction(
    IN PRTMP_ADAPTER    pAd,
    IN PUCHAR			pDest_Addr,
    IN PUCHAR			pBuf,
    IN USHORT			buf_len) 
{	
    PMGMTENTRY     	pEntry;
	PEAPOL_PACKET       pEapol_packet;	
	KEY_INFO			peerKeyInfo;
	//UINT				eapol_len;

    pEapol_packet = (PEAPOL_PACKET)pBuf;
//	eapol_len = CONV_ARRARY_TO_UINT16(pEapol_packet->Body_Len) + LENGTH_EAPOL_H;

	NdisZeroMemory((PUCHAR)&peerKeyInfo, sizeof(peerKeyInfo));
	NdisMoveMemory((PUCHAR)&peerKeyInfo, (PUCHAR)&pEapol_packet->KeyDesc.KeyInfo, sizeof(KEY_INFO));

	USHORT U16Tmp;
	/* key Information element has done. */
	NdisMoveMemory(&U16Tmp, &peerKeyInfo, sizeof(peerKeyInfo));
	U16Tmp = cpu2le16(U16Tmp);
	NdisMoveMemory(&peerKeyInfo, &U16Tmp, sizeof(peerKeyInfo));


//	*((USHORT *)&peerKeyInfo) = cpu2le16(*((USHORT *)&peerKeyInfo));

    do
    {
        pEntry = MGMTENTRY_GetNode(pAd, pDest_Addr);
		if (!pEntry) {
			DBGPRINT(RT_ERROR, ("[ERR]%s: Can't find STA node %02X-%02X-%02X-%02X-%02X-%02X\n", 
								__FUNCTION__, PRINT_MAC(pDest_Addr)));
           break;
		}

		if (pEntry->AuthMode < Ndis802_11AuthModeWPA)
			break;		

		DBGPRINT(RT_LOUD, ("[LOUD]%s: EAPoL-Key frame from %02X-%02X-%02X-%02X-%02X-%02X\n", 
							__FUNCTION__, PRINT_MAC(pEntry->Addr)));


        if (((pEapol_packet->ProVer != EAPOL_VER) && (pEapol_packet->ProVer != EAPOL_VER2)) || 
			((pEapol_packet->KeyDesc.Type != WPA1_KEY_DESC) && (pEapol_packet->KeyDesc.Type != WPA2_KEY_DESC)))
        {
            DBGPRINT(RT_ERROR, ("[ERR]%s: Key descripter does not match with WPA rule\n", __FUNCTION__));
            break;
        }

		/* The value 1 shall be used for all EAPOL-Key frames to and from a STA when */
		/* neither the group nor pairwise ciphers are CCMP for Key Descriptor 1.*/
		if ((pEntry->WepStatus == Ndis802_11Encryption2Enabled) && (peerKeyInfo.KeyDescVer != KEY_DESC_TKIP))
        {
	        DBGPRINT(RT_ERROR, ("[ERR]%s: Key descripter version not match(TKIP)\n", __FUNCTION__));
    	    break;
    	}	
		/* The value 2 shall be used for all EAPOL-Key frames to and from a STA when */
		/* either the pairwise or the group cipher is AES-CCMP for Key Descriptor 2 or 3.*/
        else if ((pEntry->WepStatus == Ndis802_11Encryption3Enabled) 
                        && (peerKeyInfo.KeyDescVer != KEY_DESC_AES)
                        && (peerKeyInfo.KeyDescVer != KEY_DESC_EXT))
    	{
            DBGPRINT(RT_ERROR, ("[ERR]%s: Key descripter version not match(AES) pEntry->WepStatus=%d, peerKeyInfo.KeyDescVer=%d\n", 
								__FUNCTION__, pEntry->WepStatus, peerKeyInfo.KeyDescVer));
        	break;
    	}

		/* Check if this STA is in class 3 state and the WPA state is started 						*/
//        if ((pEntry->Sst == SST_ASSOC) && (pEntry->WpaState >= AS_INITPSK))
        {			 		
			/* Check the Key Ack (bit 7) of the Key Information to determine the Authenticator */
			/* or not.*/
			/* An EAPOL-Key frame that is sent by the Supplicant in response to an EAPOL-*/
			/* Key frame from the Authenticator must not have the Ack bit set.*/
			if (peerKeyInfo.KeyAck == 1)
			{
				/* The frame is snet by Authenticator. */
				/* So the Supplicant side shall handle this.*/

				if ((peerKeyInfo.Secure == 0) && (peerKeyInfo.Request == 0) && 
					(peerKeyInfo.Error == 0) && (peerKeyInfo.KeyType == PAIRWISEKEY))
				{
					/*
						Process 
							1. the message 1 of 4-way HS in WPA or WPA2 
									EAPOL-Key(0,0,1,0,P,0,0,ANonce,0,DataKD_M1)
							2. the message 3 of 4-way HS in WPA 
									EAPOL-Key(0,1,1,1,P,0,KeyRSC,ANonce,MIC,DataKD_M3)
					*/
					if (peerKeyInfo.KeyMic == 0)
                    	PeerPairMsg1Action(pAd, pEntry, pEapol_packet);
	                else                	                	
    	                PeerPairMsg3Action(pAd, pEntry, pEapol_packet);
				}
				else if ((peerKeyInfo.Secure == 1) && 
						 (peerKeyInfo.KeyMic == 1) &&
						 (peerKeyInfo.Request == 0) && 
						 (peerKeyInfo.Error == 0))
				{
					/*
						Process
							1. the message 3 of 4-way HS in WPA2
								EAPOL-Key(1,1,1,1,P,0,KeyRSC,ANonce,MIC,DataKD_M3)
							2. the message 1 of group KS in WPA or WPA2
								EAPOL-Key(1,1,1,0,G,0,Key RSC,0, MIC,GTK[N])
					*/
					if (peerKeyInfo.KeyType == PAIRWISEKEY)
						PeerPairMsg3Action(pAd, pEntry, pEapol_packet);
					else
						PeerGroupMsg1Action(pAd, pEntry, pEapol_packet);					
				}
			}
			else
			{
				/*
					The frame is snet by Supplicant.So the Authenticator 
					side shall handle this.
				*/
#ifdef QOS_DLS_SUPPORT				
				if ((peerKeyInfo.KeyMic == 1) && 
					(peerKeyInfo.Secure == 1) && 
					(peerKeyInfo.Error == 0) &&
					(peerKeyInfo.KeyType == GROUPKEY) &&
					(pEapol_packet->KeyDesc.KeyDataLen[1] == 12))
	    	    {
	    	    	/* This is a ralink proprietary DLS STA-Key processing*/
	        		RTMPHandleSTAKey(pAd, pEntry, NULL);
		    	}
				else
#endif /* QOS_DLS_SUPPORT */
				if ((peerKeyInfo.KeyMic == 1) && 
					(peerKeyInfo.Request == 1) && 
					(peerKeyInfo.Error == 1))
	            {	                
					/* The Supplicant uses a single Michael MIC Failure Report frame */
					/* to report a MIC failure event to the Authenticator. */
					/* A Michael MIC Failure Report is an EAPOL-Key frame with */
					/* the following Key Information field bits set to 1: */
					/* MIC bit, Error bit, Request bit, Secure bit.*/

	                DBGPRINT(RT_ERROR, ("[ERR]%s: Received an Michael MIC Failure Report, active countermeasure \n", __FUNCTION__));
	                HandleCounterMeasure(pAd, pEntry);
	            }
				else if ((peerKeyInfo.Request == 0) && 
					 	 (peerKeyInfo.Error == 0) && 
					 	 (peerKeyInfo.KeyMic == 1))
				{
					if (peerKeyInfo.Secure == 0 && peerKeyInfo.KeyType == PAIRWISEKEY)
					{
						/* EAPOL-Key(0,1,0,0,P,0,0,SNonce,MIC,Data)*/
						/* Process 1. message 2 of 4-way HS in WPA or WPA2 */
						/*		   2. message 4 of 4-way HS in WPA	*/
						if (CONV_ARRARY_TO_UINT16(pEapol_packet->KeyDesc.KeyDataLen) == 0)
						{
							PeerPairMsg4Action(pAd, pEntry, pEapol_packet);
    	            	}
						else
						{
							PeerPairMsg2Action(pAd, pEntry, pEapol_packet);
						}
					}
					else if (peerKeyInfo.Secure == 1 && peerKeyInfo.KeyType == PAIRWISEKEY)
					{
						/* EAPOL-Key(1,1,0,0,P,0,0,0,MIC,0)						*/
						/* Process message 4 of 4-way HS in WPA2*/
						PeerPairMsg4Action(pAd, pEntry, pEapol_packet);
					}
					else if (peerKeyInfo.Secure == 1 && peerKeyInfo.KeyType == GROUPKEY)
					{
						/* EAPOL-Key(1,1,0,0,G,0,0,0,MIC,0)*/
						/* Process message 2 of Group key HS in WPA or WPA2 */
						PeerGroupMsg2Action(pAd, pEntry, pEapol_packet);
					}
				}				
				else if ((peerKeyInfo.Request == 1) && (peerKeyInfo.Error == 0))
				{							
	                /* Need to check KeyType for groupkey or pairwise key update, refer to 8021i P.114, */
	                if (peerKeyInfo.KeyType == GROUPKEY)
	                {
						UINT8				wcid;
						PAP_ADMIN_CONFIG	pApCfg;
	            
	                    DBGPRINT(RT_TRACE, ("[TRACE]%s: REQUEST=1, ERROR=0, update group key\n", __FUNCTION__));
	        
						pApCfg = &pAd->ApCfg;
			
						GenRandom(pAd, pAd->essid, pApCfg->GNonce);
						//pAd->DefaultKeyId = (pMbssEntry->DefaultKeyId == 1) ? 2 : 1;   
						//pAd->DefaultKeyId = 1;
						WpaDeriveGTK(pApCfg->GMK, 
									 pApCfg->GNonce, 
									 pAd->essid, 
									 pApCfg->GTK, LEN_TKIP_GTK);
#if 0 // no used	                    
						for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++)
	                    {
	                    	if (IS_ENTRY_CLIENT(&pAd->MacTab.Content[i]) 
								&& (pAd->MacTab.Content[i].WpaState == AS_PTKINITDONE)
								&& (pAd->MacTab.Content[i].apidx == apidx))
	                    	{
	                        	pAd->MacTab.Content[i].GTKState = REKEY_NEGOTIATING;
	                        	WPAStart2WayGroupHS(pAd, &pAd->MacTab.Content[i]);
								pAd->MacTab.Content[i].ReTryCounter = GROUP_MSG1_RETRY_TIMER_CTR;
								RTMPModTimer(&pAd->MacTab.Content[i].RetryTimer, PEER_MSG3_RETRY_EXEC_INTV);
	                    	}
	                	}
#endif
						/* Get a specific WCID to record this MBSS key attribute */
						GET_GroupKey_WCID(pAd, wcid, BSS0);
						WPAInstallSharedKey(pAd, 
											pApCfg->GroupKeyWepStatus, 
											BSS0, 
											pAd->DefaultKeyId, 
											wcid, 
											TRUE, 
											pApCfg->GTK,
											LEN_TKIP_GTK);
	                }
	                else
	                {
	                    DBGPRINT(RT_TRACE, ("[TRACE]%s: REQUEST=1, ERROR= 0, update pairwise key\n", __FUNCTION__));
	                    
						NdisZeroMemory(&pEntry->PairwiseKey, sizeof(CIPHER_KEY));  
						
						/* clear this entry as no-security mode*/
						AsicRemovePairwiseKeyEntry(pAd, pEntry->Aid); 

//	                    pEntry->Sst = SST_ASSOC;
	                    if (pEntry->AuthMode == Ndis802_11AuthModeWPA || pEntry->AuthMode == Ndis802_11AuthModeWPA2)
	                        pEntry->WpaState = AS_INITPMK;  
	                    else if (pEntry->AuthMode == Ndis802_11AuthModeWPAPSK || pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK)
	                        pEntry->WpaState = AS_INITPSK;  
						
	                    pEntry->GTKState = REKEY_NEGOTIATING;
//						pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
	                    pEntry->ReTryCounter = PEER_MSG1_RETRY_TIMER_CTR;
//						pEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
						NdisZeroMemory(pEntry->R_Counter, sizeof(pEntry->R_Counter));
	            
			            WPAStart4WayHS(pAd, pEntry, PEER_MSG1_RETRY_EXEC_INTV);
	                }
	            }				
			}			            
        }
    }while(FALSE);
}

