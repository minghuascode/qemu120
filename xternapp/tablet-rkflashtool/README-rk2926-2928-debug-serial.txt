
http://wenku.baidu.com/view/431560365727a5e9856a6167.html

RK2928 and RK2926 Debug Serial Port Connection
(RK2928&RK2926调试串口连接说明)

On RK2928 and 2926 the OTG and debug serial port are connected 
to the same set of pins by an internal IO_SWITCH. On RK2926 it is 
ping 64 65, RK2928 is V8 W8. Default the two pins are serial mode. 
When VBUS signal is detected it is switched to USB_DEVICE mode. 
When ID is low, it is switched to USB_HOST mode. 

Serial debug connection: 
      RK2926/2928 OTG_DP  --------  RXD serial board
      RK2926/2928 OTG_DN  --------  TXD serial board
      RK2926/2928 GND     --------  GND serial board
      
