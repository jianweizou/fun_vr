		AREA |.ARM.__at_0x6004E000|, DATA, READONLY
	
	
	
	
	EXPORT light_hi_master_table
	EXPORT light_hi_master_table_end
	EXPORT light_yellow_table
	EXPORT light_yellow_table_end
	EXPORT light_white_table
	EXPORT light_white_table_end
	EXPORT light_middle_table	
	EXPORT light_middle_table_end
	EXPORT light_night_table	
	EXPORT light_night_table_end
	EXPORT light_up_top_table	
	EXPORT light_up_top_table_end
	EXPORT light_down_bottom_table
	EXPORT light_down_bottom_table_end
	EXPORT light_up_table
	EXPORT light_up_table_end
	EXPORT light_down_table
	EXPORT light_down_table_end
		
	EXPORT light_has_bottom_table
	EXPORT light_has_bottom_table_end
	EXPORT light_has_top_table
	EXPORT light_has_top_table_end
	
	EXPORT light_no_talk_table
	EXPORT light_no_talk_table_end
	EXPORT light_opend_table
	EXPORT light_opend_table_end
	EXPORT light_sleep_table
	EXPORT light_sleep_table_end
	EXPORT light_talk_mode_table
	EXPORT light_talk_mode_table_end
	
		
			
;===============================================	
light_hi_master_table
		INCBIN .\vr\Audio32_File\mbe\light_hi_master.mbe
light_hi_master_table_end
		ALIGN 4	
			
light_yellow_table
		INCBIN .\vr\Audio32_File\mbe\light_yellow.mbe
light_yellow_table_end
		ALIGN 4	

light_white_table
		INCBIN .\vr\Audio32_File\mbe\light_white.mbe
light_white_table_end
		ALIGN 4		

light_night_table
        INCBIN .\vr\Audio32_File\mbe\light_night.mbe
light_night_table_end
		ALIGN 4		
			
light_middle_table
		INCBIN .\vr\Audio32_File\mbe\light_middle.mbe
light_middle_table_end
		ALIGN 4	

				
light_up_top_table
		INCBIN .\vr\Audio32_File\mbe\light_up_top.mbe
light_up_top_table_end
		ALIGN 4		
light_down_bottom_table
		INCBIN .\vr\Audio32_File\mbe\light_down_bottom.mbe
light_down_bottom_table_end
		ALIGN 4	
light_up_table
		INCBIN .\vr\Audio32_File\mbe\light_up.mbe
light_up_table_end
		ALIGN 4			
light_has_top_table
		INCBIN .\vr\Audio32_File\mbe\light_has_top.mbe
light_has_top_table_end
		ALIGN 4					
light_down_table
		INCBIN .\vr\Audio32_File\mbe\light_down.mbe
light_down_table_end
		ALIGN 4
light_has_bottom_table
		INCBIN .\vr\Audio32_File\mbe\light_has_bottom.mbe
light_has_bottom_table_end
		ALIGN 4			
light_no_talk_table
        INCBIN .\vr\Audio32_File\mbe\light_no_talk.mbe
light_no_talk_table_end
		ALIGN 4		
			
;===============================================	
light_hi_bai_table
        INCBIN .\vr\Audio32_File\mbe\light_hi_bai.mbe
light_hi_bai_table_end
		ALIGN 4					
				
light_talk_mode_table
        INCBIN .\vr\Audio32_File\mbe\light_talk_mode.mbe
light_talk_mode_table_end
		ALIGN 4					
light_sleep_table
		INCBIN .\vr\Audio32_File\mbe\light_sleep.mbe
light_sleep_table_end
		ALIGN 4						
;===============================================		  
				
light_opend_table
        INCBIN .\vr\Audio32_File\mbe\light_opend.mbe
light_opend_table_end
		ALIGN 4					
			
		
;===============================================	
				
				
					
AREA END	
	
	
		;EXPORT A32_32K_1_START
		;EXPORT A32_32K_1_END
		;EXPORT A32_32K_2_START
		;EXPORT A32_32K_2_END			
		;EXPORT A32_24K_3_START
		;EXPORT A32_24K_3_END
		;EXPORT A32_24K_4_START
		;EXPORT A32_24K_4_END
		;EXPORT A32_16K_5_START
		;EXPORT A32_16K_5_END
		;EXPORT A32_16K_6_START
		;EXPORT A32_16K_6_END
		;EXPORT A32_16K_7_START
		;EXPORT A32_16K_7_END
		;EXPORT A32_16K_8_START
		;EXPORT A32_16K_8_END
		;EXPORT A32_16K_9_START
		;EXPORT A32_16K_9_END
		;EXPORT A32_16K_10_START
		;EXPORT A32_16K_10_END
		;EXPORT A32_16K_11_START
		;EXPORT A32_16K_11_END

	;ALIGN 4			
;A32_32K_1_START
		;;INCBIN pattern\16kbps\zai.mbe
;A32_32K_1_END

	;ALIGN 4			
;A32_32K_2_START
		;;INCBIN pattern\32kbps\1129.a32
;A32_32K_2_END

	;ALIGN 4			
;A32_24K_3_START
		;;INCBIN pattern\24kbps\1113.a32
;A32_24K_3_END

	;ALIGN 4			
;A32_24K_4_START
		;;INCBIN pattern\24kbps\1129.a32
;A32_24K_4_END

	;ALIGN 4			
;A32_16K_5_START
		;;INCBIN pattern\16kbps\1129.a32
;A32_16K_5_END

	;ALIGN 4			
;A32_16K_6_START
		;;INCBIN pattern\16kbps\1124.a32
;A32_16K_6_END
	;ALIGN 4			
;A32_16K_7_START
		;INCBIN pattern\16kbps\Wav001.mbe
;A32_16K_7_END
	;ALIGN 4			
;A32_16K_8_START
		;INCBIN pattern\16kbps\WAV004.mbe
;A32_16K_8_END
	;ALIGN 4			
;A32_16K_9_START
		;INCBIN pattern\16kbps\mp31.mbe
;A32_16K_9_END

;A32_16K_10_START
		;INCBIN pattern\16kbps\nihao.bin
;A32_16K_10_END
	;ALIGN 4			
;A32_16K_11_START
		;INCBIN pattern\16kbps\hello.bin
;A32_16K_11_END
;AREA END
	
