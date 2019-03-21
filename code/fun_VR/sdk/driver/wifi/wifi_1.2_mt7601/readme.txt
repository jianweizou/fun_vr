this is the fork parent scm of all drivers.

	Source code archtecture.

        Keep old api to maintain current code control flow for App.


   ------------------------------------------------------
   |						wifi_api					|
   ------------------------------------------------------
   |													|
   |                 		MLME                        |
   |-----------------------------------------------------
   |  Management   |   nl80211  |   cipher  |    WPA    |
   | Mgmt entry    |            |           |           |
   |-----------------------------------------------------
   |RTMP related control:				   |     Rx     |
   | Initial, control, andes, asic, timer  |            |
   |----------------------------------------            |
   |MT7601 (FW/Register Table/control)     |     Tx     |
   |-----------------------------------------------------
   | 						usb_io						|
   ------------------------------------------------------

 
