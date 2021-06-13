echo %~dp0
cd /d %~dp0
../nssm.exe CPOS_RTSPClient CposRtspDemo.cmd
../nssm.exe CPOS_RTSPObjectRegister socket_image_receive.cmd