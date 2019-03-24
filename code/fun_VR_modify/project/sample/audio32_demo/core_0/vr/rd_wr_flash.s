; anything that starts in the first column is considered to be a label

	AREA |.ARM.__at_0x60030000|, DATA, READONLY
	EXPORT CYB_CMD1_INFLASH [WEAK]
	EXPORT CYB_CMD1_INFLASH_END


CYB_CMD1_INFLASH
	INCBIN	fun_chn_pack.bin
CYB_CMD1_INFLASH_END
	
	END
