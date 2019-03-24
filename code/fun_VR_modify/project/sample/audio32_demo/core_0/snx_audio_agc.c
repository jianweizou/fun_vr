//version 2.3, release by Jerry Fu
//-------------------------------------------------------------
#define AGC_PEAKAMP_THRES_SATURATION 2000//26000
#define AGC_CONCERT_ENV_AMP_CASE 1500//4000
#define AGC_CONCERT_ENV_AMP_CASE_NOISE  (int)(AGC_CONCERT_ENV_AMP_CASE/1.5)
#define AGC_SILCNT_THRS 40.0 //18
#define AGC_SATURATION_LAST 32767
#define AGC_GAIN_UP 1
#define AGC_GAIN_DOWN 1
#define AGC_NOISEFLOOR_MAX 500
#define AGC_NOISEFLOOR_MIN 100//28 //100,update noise level mini
#define MIN32(a,b) ((a) < (b) ? (a) : (b))  
#define MAX32(a,b) ((a) > (b) ? (a) : (b))  

int g_AGC_bufsize								;
int g_AGC_gain									;
int g_AGC_peakamp_thres					;
int g_AGC_peakcnt_thres					;
int g_AGC_peakcnt_thres2				; //saturation
int g_AGC_noise_floor						;
int g_AGC_saturation_alarm			;
int g_AGC_silence_cnt						;
int g_AGC_current_amp						;
int g_AGC_max_gain							;
int g_AGC_default_gain					;
int g_AGC_frame_trigger;


//-------------------------------
int g_AGC_dynamic_default_gain	;
int g_AGC_dynamic_up_control_frmcnt;
int g_AGC_dynamic_down_control_frmcnt;
int g_AGC_dynamic_max_gain;
int g_AGC_dynamic_min_gain;

int g_AGC_dynamic_targetLv_lowbound;
int g_AGC_dynamic_targetLv_highbound;
int g_AGC_dynamic_targetLv_speed;


int g_AGC_min_gain							;
int	g_AGC_response_up_frmcnt		;
int g_AGC_response_up_speed		  ;
int	g_AGC_response_down_frmcnt	;
int g_AGC_response_down_speed		;

int update_NoiseFloor(int Energy);

int update_frmcnt = 0						;  //總共的framecnt
int tracking_frmcnt = 0					;  //每次update的frmcnt
int EngMin = 500								;
int EngMinTmp = 500							;

int noise_update_lv1;
int noise_update_lv2;

int tracking_update_init;
int noise_tracking_update_lv1;
int noise_tracking_update_lv2;

int g_silencecnt_thres ;

int g_silence_tag;
int peakcnt=0,peakcnt2=0;
int g_temp;
int g_AGC_TV_flag;
int g_silence_flag;

//int *AGC_ptr[41];

short saturation_protect(int current_amp,int peak_cnt,int peak_cnt2, int noise_floor);
void dynamic_default_gain(int current_amp , int noise_floor);


//void Int_to_Ptr(void)
//{
//	for(int k=0;k<41;k++){
//		AGC_ptr[k]= &(g_AGC_bufsize)+k ;
//	}
//}
int SNX_AGC_Process(short *inputbuf, short *outputbuf, short sample_num)
{
	int i;
	//static int peakcnt=0,peakcnt2=0;
	int temp;
	static int sumt=0;
	static int sumt_out=0;
	static short sample_cnt=0;
	static short conti_cnt=0, prev_gain=0;

	/*sumt = 0;
	peakcnt = 0;
	peakcnt2 = 0;*/
	for(i=0;i<sample_num;i++)
	{	
		temp = (int)abs(inputbuf[i]);
		sumt += temp;
		if (temp>g_AGC_peakamp_thres)
		{
					peakcnt++;
			if (temp >= AGC_PEAKAMP_THRES_SATURATION)		
						peakcnt2 ++;					
		}
		temp = (int)abs(outputbuf[i]);
		sumt_out+=temp;
	}
	//g_AGC_current_amp: input
	//sumt_out: BF & compressor output
	
	sample_cnt+=sample_num;
	temp=0;
	if(sample_cnt>=g_AGC_bufsize){
		sample_cnt=0;
		if(g_AGC_bufsize == 128){
			g_AGC_current_amp = (int)(sumt>>7);	
			sumt_out>>=7;
		}else if (g_AGC_bufsize == 256){
			g_AGC_current_amp = (int)(sumt>>8);	
			sumt_out>>=8;			
		}else if (g_AGC_bufsize == 512){
			g_AGC_current_amp = (int)(sumt>>9);	
			sumt_out>>=9;
		}else{
			g_AGC_current_amp = (int)(sumt/g_AGC_bufsize);//general case
			sumt_out=(int)(sumt_out/g_AGC_bufsize);
		}
			
		//g_AGC_current_amp=(int)(g_AGC_current_amp*10./sumt_out);
		//g_AGC_current_amp=sumt_out;
		g_AGC_noise_floor = update_NoiseFloor(sumt_out);
		/*if((sumt_out < g_AGC_noise_floor*1.5)){
			conti_cnt--;
			if(conti_cnt<0) conti_cnt=0;
		}else*/ if((g_AGC_current_amp<(sumt_out*2.2)) ){//&& (sumt_out > g_AGC_noise_floor*1.5)){
			conti_cnt++;
		}else{
			conti_cnt--;
			if(conti_cnt<0) conti_cnt=0;
		}
		if(conti_cnt>=4){
			if(conti_cnt>7)	conti_cnt=7;//conti_cnt=0;//
			g_AGC_TV_flag=0;
		}else{
			g_AGC_TV_flag=1;
		}

		g_silence_tag=saturation_protect(sumt_out,peakcnt,peakcnt2,g_AGC_noise_floor);//v1 module
			
		dynamic_default_gain(sumt_out , g_AGC_noise_floor);//v2 feature

		//gain index boundary
		if (g_AGC_gain>g_AGC_max_gain)
			g_AGC_gain = g_AGC_max_gain;
		else if  (g_AGC_gain<g_AGC_min_gain)
			g_AGC_gain = g_AGC_min_gain;
		
		//peakcnt*=-1;
		temp = peakcnt2;
		peakcnt=0;
		peakcnt2=0;
		sumt=0;
		sumt_out=0;
//		if(conti_cnt>=3){
//			if(prev_gain!=g_AGC_gain)
//				return prev_gain;
//		}
//		else
//			prev_gain=g_AGC_gain;
		//return temp;
	}
	g_temp = temp;//
	return g_AGC_gain;//temp;//
}


int update_NoiseFloor(int Energy)
{
   int tracking_update;
   int itemp;

   if (update_frmcnt <noise_update_lv1)  
	   tracking_update=tracking_update_init;	  //noisefloor update speed
   else if (update_frmcnt <noise_update_lv2)
	   tracking_update=noise_tracking_update_lv1;		//noisefloor update speed
   else
	   tracking_update=noise_tracking_update_lv2;   //noisefloor update speed
	   

   if(Energy>AGC_NOISEFLOOR_MIN &&  Energy<AGC_NOISEFLOOR_MAX)
   {
 	   if (tracking_frmcnt >= tracking_update)
	   {     
			//local minmum refresh
		  tracking_frmcnt = 0;
		  EngMin = MIN32(EngMinTmp, Energy); //use EngMinTmp update EngMin.
		  EngMinTmp = Energy;	//forget old EngMinTmp value.
	   } 
	   else 
	   {
		  EngMin = MIN32(EngMin, Energy);
		  EngMinTmp = MIN32(EngMinTmp, Energy);      
	   }	          
   }


	update_frmcnt++;
	if (update_frmcnt>30000)
		update_frmcnt = 30000;
	
	tracking_frmcnt++;

	itemp = EngMin;
	//itemp *= 2;

	if(itemp > AGC_NOISEFLOOR_MAX)
		itemp = AGC_NOISEFLOOR_MAX;
	
	if(itemp < AGC_NOISEFLOOR_MIN)
		itemp = AGC_NOISEFLOOR_MIN;
	

	return (int)itemp;
}


void SNX_AGC_Process_init(int AGC_gain_max,
													int AGC_gain_min,	
													int AGC_gain_default,
													int AGC_Dynamic_gain_max,
													int AGC_Dynamic_gain_min,
													int AGC_Dynamic_Target_Level_Low,
													int AGC_Dynamic_Target_Level_High,
													int AGC_Dynamic_updateSpeed ,
													int AGC_bufsize,
													int sample_rate,
													int peakamp_thres,
													int peakcnt_thres,
													int response_up_step,
													int response_down_step)
{
	
	g_AGC_peakcnt_thres = peakcnt_thres;	//(1/5)*g_AGC_bufsize ~ (1/3)*g_AGC_bufsize

	//peak cnt for saturation case
	g_AGC_peakcnt_thres2 = (int) ( (  (float)13.0/16000.0 )*(float)sample_rate   );
	if(g_AGC_peakcnt_thres2<10)
		g_AGC_peakcnt_thres2 = 10;
	
	g_AGC_peakamp_thres = peakamp_thres;
	
	g_AGC_response_up_speed = response_up_step;
	g_AGC_response_down_speed = response_down_step;
	g_AGC_response_up_frmcnt = 0;
	g_AGC_response_down_frmcnt = 0;
	
	g_AGC_bufsize = AGC_bufsize;		
	g_AGC_gain = AGC_gain_default;

	
	g_AGC_noise_floor = AGC_NOISEFLOOR_MAX;
	g_AGC_saturation_alarm = 0;
	g_AGC_silence_cnt = 0;
	g_AGC_current_amp = 0;
	
	//gain threshold-----------------------
	g_AGC_max_gain = AGC_gain_max;
	g_AGC_min_gain = AGC_gain_min;	
	g_AGC_default_gain = AGC_gain_default;
			
		
	//noise floor tracking-----------------
	EngMin = 500;
	EngMinTmp = 500;
	update_frmcnt = 0;
	tracking_frmcnt = 0;	
	
	
	//Dynamic Default Gain ----------------
	g_AGC_dynamic_default_gain = AGC_gain_default;
	g_AGC_dynamic_up_control_frmcnt = 0;
	g_AGC_dynamic_down_control_frmcnt = 0;

	g_AGC_dynamic_min_gain = AGC_Dynamic_gain_min;
	g_AGC_dynamic_max_gain = AGC_Dynamic_gain_max;

	g_AGC_dynamic_targetLv_lowbound = AGC_Dynamic_Target_Level_Low;
	g_AGC_dynamic_targetLv_highbound = AGC_Dynamic_Target_Level_High; 
	g_AGC_dynamic_targetLv_speed = AGC_Dynamic_updateSpeed;


	//---------------------------------------convert 16khz parameters to current sample rate. 
	g_silencecnt_thres = (int)( (  (float)(AGC_SILCNT_THRS)*256.0/16000.0 )*(float)sample_rate/(float)AGC_bufsize );

	noise_update_lv1 =  (int) ( (  (float)100.0*256.0/16000.0 )*(float)sample_rate/(float)AGC_bufsize );
	noise_update_lv2 =  (int) ( (  (float)500.0*256.0/16000.0 )*(float)sample_rate/(float)AGC_bufsize );

	tracking_update_init =  (int) ( (  (float)15.0*256.0/16000.0 )*(float)sample_rate/(float)AGC_bufsize );
	noise_tracking_update_lv1 = (int) ( (  (float)50.0*256.0/16000.0 )*(float)sample_rate/(float)AGC_bufsize );
	noise_tracking_update_lv2 = (int) ( (  (float)300.0*256.0/16000.0 )*(float)sample_rate/(float)AGC_bufsize );

	if(noise_update_lv1 <= 0 )
		noise_update_lv1 = 1;

	if(noise_update_lv2 <= 0 )
		noise_update_lv2 = 1;

	if(tracking_update_init<=0)
		tracking_update_init = 1;

	if(noise_tracking_update_lv1<=0)
		noise_tracking_update_lv1 = 1;

	if(noise_tracking_update_lv2<=0)
		noise_tracking_update_lv2 = 1;

	if(g_silencecnt_thres<=0)
		g_silencecnt_thres = 1;

}


short saturation_protect(int current_amp,int peak_cnt,int peak_cnt2, int noise_floor)
{
	int noise_floor_temp;
	static short follow_adc_gain_cnt=0;
	short silence_flag=0;
	
	if(noise_floor<AGC_CONCERT_ENV_AMP_CASE_NOISE)
		noise_floor_temp = noise_floor + (noise_floor>>1);//1.5
	else 
		noise_floor_temp = noise_floor;
	
	if (  ( peak_cnt > g_AGC_peakcnt_thres ) || peak_cnt2>g_AGC_peakcnt_thres2)
		g_AGC_saturation_alarm=0;
					
	if (  ( current_amp > noise_floor_temp && peak_cnt > g_AGC_peakcnt_thres ) || peak_cnt2>g_AGC_peakcnt_thres2)
		g_AGC_saturation_alarm = 1;		

		
	if (current_amp < noise_floor_temp)//silence detection
	{
		//g_silence_tag=-1;
		if (g_AGC_silence_cnt<32767)
			g_AGC_silence_cnt ++;
	}
	else
	{		
		//g_silence_tag=1;
		g_AGC_silence_cnt = 0;
	}
		
	if (g_AGC_silence_cnt>g_silencecnt_thres) //silence case, back to default gain
	{
		silence_flag=1;
		if (g_AGC_response_down_frmcnt == 0)
			if (g_AGC_gain>g_AGC_dynamic_default_gain)
				g_AGC_gain -= AGC_GAIN_DOWN;
			
		if (g_AGC_response_up_frmcnt == 0 && current_amp < g_AGC_dynamic_targetLv_lowbound/*AGC_CONCERT_ENV_AMP_CASE*/)//********************		
			if(g_AGC_gain < g_AGC_dynamic_default_gain)
				g_AGC_gain += AGC_GAIN_UP;				

	}
	else// speech active case, ------------------------------------
	{
			silence_flag=0;
			if(g_AGC_saturation_alarm != 0 && g_AGC_TV_flag==0){//saturation protect trigger point.
				//g_AGC_response_up_frmcnt--;
				if (g_AGC_response_down_frmcnt == 0 ){
					g_AGC_gain -= AGC_GAIN_DOWN;
					follow_adc_gain_cnt++;
					if(follow_adc_gain_cnt==2){
						follow_adc_gain_cnt=0;
						g_AGC_response_up_frmcnt=0;
						g_AGC_dynamic_default_gain--;
						if(g_AGC_dynamic_default_gain<g_AGC_dynamic_min_gain)
							g_AGC_dynamic_default_gain=g_AGC_dynamic_min_gain;
					}
				}
			}				

	}

	//AGC response speed control, increase gain control
	if (g_AGC_response_up_frmcnt == g_AGC_response_up_speed)
		g_AGC_response_up_frmcnt = 0;
	else
		g_AGC_response_up_frmcnt++;
	
	//AGC response speed control, decrease gain control
	if (g_AGC_response_down_frmcnt == g_AGC_response_down_speed)
		g_AGC_response_down_frmcnt = 0;
	else
		g_AGC_response_down_frmcnt++;

	

	g_AGC_saturation_alarm = 0;	
	return silence_flag;
}


void dynamic_default_gain(int current_amp , int noise_floor)
{
	int noise_floor_temp;
		
		//if(noise_floor!=AGC_NOISEFLOOR_MIN)
			noise_floor_temp = noise_floor+(noise_floor>>1);//noise_floor*2; //1.5
		//else
		//	noise_floor_temp = noise_floor;
			


	//if(current_amp > noise_floor)
	{
		
		if(update_frmcnt>noise_tracking_update_lv2 &&  current_amp>noise_floor_temp/* g_AGC_amp_localmax>noise_floor_temp*/)//initial framecnt threshold
		{ //Speech signal
			//------------------			
			if(current_amp < g_AGC_dynamic_targetLv_lowbound){
				g_AGC_dynamic_up_control_frmcnt++;
				g_AGC_dynamic_down_control_frmcnt--;
			}else if (current_amp > g_AGC_dynamic_targetLv_highbound){
				g_AGC_dynamic_up_control_frmcnt--;
				g_AGC_dynamic_down_control_frmcnt++;
			}else{
				g_AGC_dynamic_up_control_frmcnt--;
				g_AGC_dynamic_down_control_frmcnt--;
			}
			if(g_AGC_dynamic_down_control_frmcnt<0)g_AGC_dynamic_down_control_frmcnt=0;
			if(g_AGC_dynamic_up_control_frmcnt<0)g_AGC_dynamic_up_control_frmcnt=0;
						
			//------------------
			
			if(!g_AGC_TV_flag && g_AGC_dynamic_up_control_frmcnt > g_AGC_dynamic_targetLv_speed)
			{
					g_AGC_dynamic_up_control_frmcnt = 0;
					g_AGC_dynamic_default_gain ++;	
					if(g_AGC_gain<g_AGC_dynamic_default_gain)
						g_AGC_gain++;
			}
			
			if(!g_AGC_TV_flag && g_AGC_dynamic_down_control_frmcnt > g_AGC_dynamic_targetLv_speed )
			{
					g_AGC_dynamic_down_control_frmcnt = 0;
					g_AGC_dynamic_default_gain --;	
			}		
		}	
			
	}

	//default gain index boundary
	if (g_AGC_dynamic_default_gain>g_AGC_dynamic_max_gain)
		g_AGC_dynamic_default_gain = g_AGC_dynamic_max_gain;
	else if  (g_AGC_dynamic_default_gain<g_AGC_dynamic_min_gain)
		g_AGC_dynamic_default_gain = g_AGC_dynamic_min_gain;	
}


