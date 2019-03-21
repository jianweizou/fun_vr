#include "rtmp_andes.h"
#include "usb_io.h"

__align(8) UCHAR CMDData[1400];

UCHAR GetCmdSeq(PRTMP_ADAPTER pAd)
{
	pAd->CmdSeq >= 0xf ? pAd->CmdSeq = 1 : pAd->CmdSeq++;

	return pAd->CmdSeq;
}


NDIS_STATUS AsicSendCmdToAndes(
	IN PRTMP_ADAPTER 	pAd, 
	IN struct CMD_UNIT 	*CmdUnit)
{
	UINT32 VarLen;
	PUCHAR pPos, pBuf;
	TXINFO_NMAC_CMD *TxInfoCmd;
	NDIS_STATUS Ret = NDIS_SUCCESS;
	__align(8) UCHAR AndesData[1400];
	UCHAR BkInBuf[16];
	
	pBuf = AndesData;
	NdisZeroMemory(pBuf, sizeof(AndesData));


	VarLen = sizeof(*TxInfoCmd) + CmdUnit->u.ANDES.CmdPayloadLen;

	pPos = pBuf;
	
	TxInfoCmd = (TXINFO_NMAC_CMD *)pPos;
	
	TxInfoCmd->info_type = CMD_PACKET;
	TxInfoCmd->d_port = CPU_TX_PORT;
	TxInfoCmd->cmd_type = CmdUnit->u.ANDES.Type;

	if (CmdUnit->u.ANDES.NeedRsp)
	{
		TxInfoCmd->cmd_seq = GetCmdSeq(pAd);
	}
	else
	{	
		TxInfoCmd->cmd_seq = 0;
	}

	TxInfoCmd->pkt_len = CmdUnit->u.ANDES.CmdPayloadLen;

	pPos += sizeof(*TxInfoCmd);

	NdisMoveMemory(pPos, CmdUnit->u.ANDES.CmdPayload, CmdUnit->u.ANDES.CmdPayloadLen);

 	RTUSB_EPOUThndl(pAd, 4, pBuf, VarLen+4);
	
	if (CmdUnit->u.ANDES.NeedRsp)
		RTUSB_BULKINhndl(pAd, 5, BkInBuf, sizeof(BkInBuf));

	return Ret;
}



NDIS_STATUS AndesBBPRandomWrite(
	IN PRTMP_ADAPTER 	pAd, 
	IN UINT32 			Num, ...)
{
	struct CMD_UNIT CmdUnit;
	PUCHAR pPos, pBuf, pCurHeader;
	UINT32 VarLen = Num * 8, CurLen = 0, SentLen;
	UINT32 Value, i, CurIndex = 0;
	NDIS_STATUS Ret = NDIS_SUCCESS;
	va_list argptr;
	BOOLEAN LastPacket = FALSE;
	
	pBuf = CMDData;
	NdisZeroMemory(pBuf, sizeof(CMDData));

	va_start(argptr, Num);

	pPos = pBuf;

	while (CurLen < VarLen)
	{
		SentLen = (VarLen - CurLen) > InbandPacketMaxLen 
									? InbandPacketMaxLen : (VarLen - CurLen);

		if ((SentLen < InbandPacketMaxLen) || (CurLen +InbandPacketMaxLen) == VarLen)
			LastPacket = TRUE;
		
		pCurHeader = pPos;
		
		for (i = 0; i < (SentLen / 8); i++)
		{
			Value = 0;
			/* BBP selection */
			Value = (Value & ~0x40000000) | 0x40000000;

			/* BBP Index */
			Value = (Value & ~0x000000ff) | va_arg(argptr, UINT);
			
			Value = cpu2le32(Value);
			NdisMoveMemory(pPos, &Value, 4);
			pPos += 4;

			Value = 0;
			/* UpdateData */
			Value = (Value & ~0x000000ff) | va_arg(argptr, UINT);

			Value = cpu2le32(Value);
			NdisMoveMemory(pPos, &Value, 4);
			pPos += 4;
		}

		NdisZeroMemory(&CmdUnit, sizeof(CmdUnit));
	
		CmdUnit.u.ANDES.Type = CMD_RANDOM_WRITE;
		CmdUnit.u.ANDES.CmdPayloadLen = SentLen;
		CmdUnit.u.ANDES.CmdPayload = pCurHeader;

		if (LastPacket)
		{
			CmdUnit.u.ANDES.NeedRsp = TRUE;
			CmdUnit.u.ANDES.NeedWait = TRUE;
			CmdUnit.u.ANDES.Timeout = 0;
		}

		Ret = AsicSendCmdToAndes(pAd, &CmdUnit);
		
		if (Ret != NDIS_SUCCESS)
			goto error;

		CurIndex += (SentLen / 8);
		CurLen += InbandPacketMaxLen;
	}

error:
	va_end(argptr);
	return Ret;
}


NDIS_STATUS AndesBBPRandomWritePair(
	IN PRTMP_ADAPTER 	pAd, 
	IN PRTMP_REG_PAIR 	pRegPair, 
	IN UINT32 			Num)
{
	struct CMD_UNIT CmdUnit;
	PUCHAR pPos, pBuf, pCurHeader;
	UINT32 VarLen = Num * 8, CurLen = 0, SentLen;
	UINT32 Value, i, CurIndex = 0;
	NDIS_STATUS Ret = NDIS_SUCCESS;
	BOOLEAN LastPacket = FALSE;
	
	pBuf = CMDData;
	NdisZeroMemory(pBuf, sizeof(CMDData));
	
	pPos = pBuf;

	while (CurLen < VarLen)
	{
		SentLen = (VarLen - CurLen) > InbandPacketMaxLen 
									? InbandPacketMaxLen : (VarLen - CurLen);

		if ((SentLen < InbandPacketMaxLen) || (CurLen + InbandPacketMaxLen) == VarLen)
			LastPacket = TRUE;
		
		pCurHeader = pPos;
		
		for (i = 0; i < (SentLen / 8); i++)
		{
			Value = 0;
			/* BBP selection */
			Value = (Value & ~0x40000000) | 0x40000000;

			/* BBP Index */
			Value = (Value & ~0x000000ff) | pRegPair[i + CurIndex].Register;
			
			Value = cpu2le32(Value);
			NdisMoveMemory(pPos, &Value, 4);
			pPos += 4;

			Value = 0;
			/* UpdateData */
			Value = (Value & ~0x000000ff) | pRegPair[i + CurIndex].Value;

			Value = cpu2le32(Value);
			NdisMoveMemory(pPos, &Value, 4);
			pPos += 4;
		}

		NdisZeroMemory(&CmdUnit, sizeof(CmdUnit));
	
		CmdUnit.u.ANDES.Type = CMD_RANDOM_WRITE;
		CmdUnit.u.ANDES.CmdPayloadLen = SentLen;
		CmdUnit.u.ANDES.CmdPayload = pCurHeader;

		if (LastPacket)
		{
			CmdUnit.u.ANDES.NeedRsp = TRUE;
			CmdUnit.u.ANDES.NeedWait = TRUE;
			CmdUnit.u.ANDES.Timeout = 0;
		}

		Ret = AsicSendCmdToAndes(pAd, &CmdUnit);
		
		if (Ret != NDIS_SUCCESS)
			goto error;

		CurIndex += (SentLen / 8);
		CurLen += InbandPacketMaxLen;
	}
	
error:
	return Ret;
}


NDIS_STATUS AndesBurstWrite(
	IN PRTMP_ADAPTER 	pAd, 
	IN UINT32 			Offset, 
	IN PUINT32 			pData, 
	IN UINT32 			Cnt)
{
	struct CMD_UNIT CmdUnit;
	PUCHAR pPos, pBuf, pCurHeader;
	UINT32 VarLen, OffsetNum, CurLen = 0, SentLen;
	UINT32 Value, i, CurIndex = 0;
	NDIS_STATUS Ret = NDIS_SUCCESS;
	BOOLEAN LastPacket = FALSE;

	OffsetNum = Cnt / ((InbandPacketMaxLen - sizeof(Offset)) / 4);

	if (Cnt % ((InbandPacketMaxLen - sizeof(Offset)) / 4))
		VarLen = sizeof(Offset) * (OffsetNum + 1) + 4 * Cnt;
	else
		VarLen = sizeof(Offset) * OffsetNum + 4 * Cnt;

	pBuf = CMDData;
	NdisZeroMemory(pBuf, sizeof(CMDData));

	pPos = pBuf;
	
	while (CurLen < VarLen)
	{
		SentLen = (VarLen - CurLen) > InbandPacketMaxLen 
									? InbandPacketMaxLen : (VarLen - CurLen);
		
		if ((SentLen < InbandPacketMaxLen) || (CurLen + InbandPacketMaxLen) == VarLen)
			LastPacket = TRUE;
	
		pCurHeader = pPos;

		Value = cpu2le32(Offset + 0x410000 + CurIndex * 4);
		NdisMoveMemory(pPos, &Value, 4);
		pPos += 4;

		for (i = 0; i < ((SentLen - 4) / 4); i++)
		{
			Value = cpu2le32(pData[i + CurIndex]);
			NdisMoveMemory(pPos, &Value, 4);
			pPos += 4;
		};

		NdisZeroMemory(&CmdUnit, sizeof(CmdUnit));
	
		CmdUnit.u.ANDES.Type = CMD_BURST_WRITE;
		CmdUnit.u.ANDES.CmdPayloadLen = SentLen;
		CmdUnit.u.ANDES.CmdPayload = pCurHeader;

		if (LastPacket && (Cnt > 1))
		{
			CmdUnit.u.ANDES.NeedRsp = TRUE;
			CmdUnit.u.ANDES.NeedWait = TRUE;
			CmdUnit.u.ANDES.Timeout = 0;
		}

		Ret = AsicSendCmdToAndes(pAd, &CmdUnit);

		if (Ret != NDIS_SUCCESS)
			goto error;
		
		CurIndex += ((SentLen - 4) / 4);
		CurLen += InbandPacketMaxLen;
	}

error:
	return Ret;
}


NDIS_STATUS AndesCalibrationOP(
	IN PRTMP_ADAPTER 	pAd, 
	IN UINT32 			CalibrationID, 
	IN UINT32 			Param)
{
	struct CMD_UNIT CmdUnit;
	PUCHAR pPos, pBuf;
	UINT32 VarLen;
	UINT32 Value;
	NDIS_STATUS Ret = NDIS_SUCCESS;

	pBuf = CMDData;
	
	NdisZeroMemory(pBuf, sizeof(CMDData));
	
	pPos = pBuf;

	/* Calibration ID and Parameter */
	VarLen = 8;
	
	/* Calibration ID */
	Value = cpu2le32(CalibrationID);
	NdisMoveMemory(pPos, &Value, 4);
	pPos += 4;

	/* Parameter */
	Value = cpu2le32(Param);
	NdisMoveMemory(pPos, &Value, 4);
	pPos += 4;
	
	NdisZeroMemory(&CmdUnit, sizeof(CmdUnit));
	
	CmdUnit.u.ANDES.Type = CMD_CALIBRATION_OP;
	CmdUnit.u.ANDES.CmdPayloadLen = VarLen;
	CmdUnit.u.ANDES.CmdPayload = (PUCHAR)pBuf;
	
	CmdUnit.u.ANDES.NeedRsp = TRUE;
	CmdUnit.u.ANDES.NeedWait = TRUE;
	CmdUnit.u.ANDES.Timeout = 0;

	Ret = AsicSendCmdToAndes(pAd, &CmdUnit);
	return Ret;
}


NDIS_STATUS AndesFunSetOP(
	IN PRTMP_ADAPTER 	pAd, 
	IN UINT32 			FunID, 
	IN UINT32 			Param)
{
	struct CMD_UNIT CmdUnit;
	PCHAR pPos;
	UINT32 VarLen;
	UINT32 Value;
	INT32 Ret;
	CHAR Buf[8];

	/* Function ID and Parameter */
	VarLen = 8;

	pPos = Buf;
	
	/* Function ID */
	Value = cpu2le32(FunID);
	NdisMoveMemory(pPos, &Value, 4);
	pPos += 4;

	/* Parameter */
	Value = cpu2le32(Param);
	NdisMoveMemory(pPos, &Value, 4);
	pPos += 4;
	
	NdisZeroMemory(&CmdUnit, sizeof(CmdUnit));
	
	CmdUnit.u.ANDES.Type = CMD_FUN_SET_OP;
	CmdUnit.u.ANDES.CmdPayloadLen = VarLen;
	CmdUnit.u.ANDES.CmdPayload =(PUCHAR)Buf;

	if (FunID == 5 )
	{
		CmdUnit.u.ANDES.NeedRsp = TRUE;
		CmdUnit.u.ANDES.NeedWait = TRUE;
		CmdUnit.u.ANDES.Timeout = 0;
	}

	Ret = AsicSendCmdToAndes(pAd, &CmdUnit);
	return Ret;
}


NDIS_STATUS AndesRandomWritePair(
	IN PRTMP_ADAPTER 	pAd, 
	IN PRTMP_REG_PAIR 	pRegPair, 
	IN UINT32 			Num)
{
	struct CMD_UNIT CmdUnit;
	PUCHAR pPos, pBuf, pCurHeader;
	UINT32 VarLen = Num * 8, CurLen = 0, SentLen;
	UINT32 Value, i, CurIndex = 0;
	NDIS_STATUS Ret = NDIS_SUCCESS;
	BOOLEAN LastPacket = FALSE;
	
	pBuf = CMDData;
	NdisZeroMemory(pBuf, sizeof(CMDData));

	pPos = pBuf;

	while (CurLen < VarLen)
	{
		SentLen = (VarLen - CurLen) > InbandPacketMaxLen 
									? InbandPacketMaxLen : (VarLen - CurLen);

		if ((SentLen < InbandPacketMaxLen) || (CurLen + InbandPacketMaxLen) == VarLen)
			LastPacket = TRUE;

		pCurHeader = pPos;
		
		for (i = 0; i < (SentLen / 8); i++)
		{
			/* Address */
			Value = cpu2le32(pRegPair[i + CurIndex].Register +   0x410000);
			NdisMoveMemory(pPos, &Value, 4);
			pPos += 4;

			/* UpdateData */
			Value = cpu2le32(pRegPair[i + CurIndex].Value);
			NdisMoveMemory(pPos, &Value, 4);
			pPos += 4;
		};

		NdisZeroMemory(&CmdUnit, sizeof(CmdUnit));
	
		CmdUnit.u.ANDES.Type = CMD_RANDOM_WRITE;
		CmdUnit.u.ANDES.CmdPayloadLen = SentLen;
		CmdUnit.u.ANDES.CmdPayload = pCurHeader;

		if (LastPacket)
		{
			CmdUnit.u.ANDES.NeedRsp = TRUE;
			CmdUnit.u.ANDES.NeedWait = TRUE;
			CmdUnit.u.ANDES.Timeout = 0;
		}

		Ret = AsicSendCmdToAndes(pAd, &CmdUnit);
	
		if (Ret != NDIS_SUCCESS)
			goto error;
	
		CurIndex += (SentLen / 8);
		CurLen += InbandPacketMaxLen;
	}

error:
	return Ret;
}



NDIS_STATUS AndesRFRandomWrite(
	IN PRTMP_ADAPTER 	pAd, 
	IN UINT32 			Num, ...)
{
	struct CMD_UNIT *CmdUnit;
	PUCHAR pPos, pBuf, pCurHeader;
	UINT32 VarLen = Num * 8, CurLen = 0, SentLen;
	UINT32 Value, i, CurIndex = 0;
	NDIS_STATUS Ret = NDIS_SUCCESS;
	va_list argptr;
	BOOLEAN LastPacket = FALSE;
	struct CMD_UNIT CMU;

	pBuf = CMDData;
	NdisZeroMemory(pBuf, sizeof(CMDData));

	CmdUnit = &CMU;
	NdisZeroMemory(CmdUnit, sizeof(struct CMD_UNIT));	

	va_start(argptr, Num);	
	pPos = pBuf;
	
	while (CurLen < VarLen)
	{
		SentLen = (VarLen - CurLen) > InbandPacketMaxLen 
									? InbandPacketMaxLen : (VarLen - CurLen);

		if ((SentLen < InbandPacketMaxLen) || (CurLen + InbandPacketMaxLen) == VarLen)
			LastPacket = TRUE;

		pCurHeader = pPos;
		
		for (i = 0; i < (SentLen / 8); i++)
		{
			Value = 0;
			/* RF selection */
			Value = (Value & ~0x80000000) | 0x80000000;

			/* RF bank */
			Value = (Value & ~0x00ff0000) | (va_arg(argptr, UINT) << 16);

			/* RF Index */
			Value = (Value & ~0x000000ff) | va_arg(argptr, UINT);

			Value = cpu2le32(Value);
			NdisMoveMemory(pPos, &Value, 4);
			pPos += 4;

			Value = 0;
			/* UpdateData */
			Value = (Value & ~0x000000ff) | va_arg(argptr, UINT);

			Value = cpu2le32(Value);
			NdisMoveMemory(pPos, &Value, 4);
			pPos += 4;
		}
	
		CmdUnit->u.ANDES.Type = CMD_RANDOM_WRITE;
		CmdUnit->u.ANDES.CmdPayloadLen = SentLen;
		CmdUnit->u.ANDES.CmdPayload = pCurHeader;

		if (LastPacket)
		{
			CmdUnit->u.ANDES.NeedRsp = TRUE;
			CmdUnit->u.ANDES.NeedWait = TRUE;
			CmdUnit->u.ANDES.Timeout = 0;
		}

		Ret = AsicSendCmdToAndes(pAd, CmdUnit);
		
		if (Ret != NDIS_SUCCESS)
			goto error;
		
		CurIndex += (SentLen / 8);
		CurLen += InbandPacketMaxLen;
	}

error:
	va_end(argptr);
	return Ret;
}



NDIS_STATUS AndesRFRandomWritePair(
	IN PRTMP_ADAPTER 		pAd, 
	IN PBANK_RF_REG_PAIR 	pRegPair, 
	IN UINT32 				Num)
{
	struct CMD_UNIT CmdUnit;
	PUCHAR pPos, pBuf, pCurHeader;
	UINT32 VarLen = Num * 8, CurLen = 0, SentLen;
	UINT32 Value, i, CurIndex = 0;
	NDIS_STATUS Ret = NDIS_SUCCESS;
	BOOLEAN LastPacket = FALSE;

	pBuf = CMDData;
	NdisZeroMemory(pBuf, sizeof(CMDData));

	pPos = pBuf;

	while (CurLen < VarLen)
	{
		SentLen = (VarLen - CurLen) > InbandPacketMaxLen 
									? InbandPacketMaxLen : (VarLen - CurLen);
		
		if ((SentLen < InbandPacketMaxLen) || (CurLen +InbandPacketMaxLen) == VarLen)
			LastPacket = TRUE;

		pCurHeader = pPos;
		
		for (i = 0; i < (SentLen / 8); i++)
		{
			Value = 0;
			/* RF selection */
			Value = (Value & ~0x80000000) | 0x80000000;

			/* RF bank */
			Value = (Value & ~0x00ff0000) | (pRegPair[i + CurIndex].Bank << 16);

			/* RF Index */
			Value = (Value & ~0x000000ff) | pRegPair[i + CurIndex].Register;
			
			//printk("Value = %x RF Bank = %d and Index = %d\n", Value, RegPair[i + CurIndex].Bank, RegPair[i + CurIndex].Register);

			Value = cpu2le32(Value);
			NdisMoveMemory(pPos, &Value, 4);
			pPos += 4;

			Value = 0;
			/* UpdateData */
			Value = (Value & ~0x000000ff) | pRegPair[i + CurIndex].Value;

			Value = cpu2le32(Value);
			NdisMoveMemory(pPos, &Value, 4);
			pPos += 4;
		}

		NdisZeroMemory(&CmdUnit, sizeof(CmdUnit));
	
		CmdUnit.u.ANDES.Type = CMD_RANDOM_WRITE;
		CmdUnit.u.ANDES.CmdPayloadLen = SentLen;
		CmdUnit.u.ANDES.CmdPayload = pCurHeader;

		if (LastPacket)
		{
			CmdUnit.u.ANDES.NeedRsp = TRUE;
			CmdUnit.u.ANDES.NeedWait = TRUE;
			CmdUnit.u.ANDES.Timeout = 0;
		}

		Ret = AsicSendCmdToAndes(pAd, &CmdUnit);
		
		if (Ret != NDIS_SUCCESS)
			goto error;
		
		CurIndex += (SentLen / 8);
		CurLen += InbandPacketMaxLen;
	}

error:
	return Ret;
}


NDIS_STATUS RLT_RF_read(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR 			bank,
	IN UCHAR 			regID,
	INOUT PUCHAR 		pValue)
{
	RLT_RF_CSR_CFG rfcsr = {{ 0}};
	UINT i = 0, k = 0;
	NDIS_STATUS	 ret = NDIS_FAILURE;

	for (i = 0; i < MAX_BUSY_COUNT; i++)
	{
		RTMP_IO_READ32(pAd, RF_CSR_CFG, &rfcsr.word);

		if (rfcsr.field.RF_CSR_KICK == BUSY)
			continue;
		
		rfcsr.word = 0;
		rfcsr.field.RF_CSR_WR = 0;
		rfcsr.field.RF_CSR_KICK = 1;
		rfcsr.field.RF_CSR_REG_ID = regID;
		rfcsr.field.RF_CSR_REG_BANK = bank;
		RTMP_IO_WRITE32(pAd, RF_CSR_CFG, rfcsr.word);
		
		for (k = 0; k < MAX_BUSY_COUNT; k++)
		{	
			RTMP_IO_READ32(pAd, RF_CSR_CFG, &rfcsr.word);

			if (rfcsr.field.RF_CSR_KICK == IDLE)
				break;
		}
		
		if ((rfcsr.field.RF_CSR_KICK == IDLE) &&
			(rfcsr.field.RF_CSR_REG_ID == regID) &&
			(rfcsr.field.RF_CSR_REG_BANK == bank))
		{
			*pValue = (UCHAR)(rfcsr.field.RF_CSR_DATA);
			break;
		}
	}

	if (rfcsr.field.RF_CSR_KICK == BUSY)
	{																	
		DBGPRINT(RT_ERROR, ("[ERR]%s: RF read R%d=0x%X fail, i[%d], k[%d]\n", 
							__FUNCTION__, regID, rfcsr.word, i, k));
		goto done;
	}
	ret = NDIS_SUCCESS;

done:
	return ret;
}


NDIS_STATUS RLT_RF_write(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR 			bank,
	IN UCHAR 			regID,
	IN UCHAR 			value)
{
	RLT_RF_CSR_CFG rfcsr = {{0}};
	UINT i = 0;
	NDIS_STATUS	 ret = NDIS_FAILURE;

	do
	{
		RTMP_IO_READ32(pAd, RF_CSR_CFG, &rfcsr.word);

		if (!rfcsr.field.RF_CSR_KICK)
			break;
		
		i++;
	} while ((i < MAX_BUSY_COUNT) );

	if ((i == MAX_BUSY_COUNT))
	{
		DBGPRINT(RT_ERROR, ("[ERR]%s: Retry count exhausted or device removed!!!\n", __FUNCTION__));
		goto done;
	}

	rfcsr.field.RF_CSR_WR = 1;
	rfcsr.field.RF_CSR_KICK = 1;
	rfcsr.field.RF_CSR_REG_BANK = bank;
	rfcsr.field.RF_CSR_REG_ID = regID;
	rfcsr.field.RF_CSR_DATA = value;
	
	RTMP_IO_WRITE32(pAd, RF_CSR_CFG, rfcsr.word);

	ret = NDIS_SUCCESS;
	
done:
	return ret;
}

