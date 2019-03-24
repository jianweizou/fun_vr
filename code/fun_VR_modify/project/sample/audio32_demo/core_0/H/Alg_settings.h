
enum Alg_ID{
	BF_ID,
	fNLP_ID,
	NLP_ID,
	NR_ID,//for NR
	MDF_ID,
	EQ_ID,
	IVA_ID,
	NLMS_ID,//for AEC
	PLDNR_ID
};

enum NR_ch{
	NR_ALL,
	NR_MIC_L
};

enum NR_var{
	NR_gain_floor,			//0x0001~0x7fff, short 
	NR_bin_used				//1~256, short 
};
