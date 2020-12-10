/*
 *******************************************************************************
 * USB-MIDI Interrupt example
 * Copyright (C) 2020 Doug Clauder
 *
 * for use with SAMD21
 *
 * This is sample program. Do not expect perfect behavior.
 *******************************************************************************
 */
/* This test example detects USB events on a connected hub. 
 *  When devices are connected and disconnected, the interrupt fires
 *  displaying the port on the hub with the activity. 
 *  The byte is passed to Hub1.GetHubStatus (custom) which is
 *  intended to enumerate/release the device on the hub.
 *  
 *  The idea is to stop polling the hub using Task().
 *  
 *  Problem is that the library assigns EP 1 to both the hub and the device
 *  connected to it. Polling toggles the use of this EP, so it is not 
 *  possible to wire up event handlers for both the device and the hub
 *  simultaneously. Need to have Midi devices take one higher EP number 
 *  upon enumeration to leave EP 1 for a hub, if present. Not sure how to 
 *  do that yet.
 */

#include <usbh_midi.h>
#include <usbhub.h>

#define SerialDebug Serial1
#ifdef SERIAL_PORT_MONITOR 
#undef SERIAL_PORT_MONITOR
#define SERIAL_PORT_MONITOR Serial1
#endif 

#define Is_uhd_in_received0(p)                    ((USB->HOST.HostPipe[p].PINTFLAG.reg&USB_HOST_PINTFLAG_TRCPT0) == USB_HOST_PINTFLAG_TRCPT0)
#define Is_uhd_in_received1(p)                    ((USB->HOST.HostPipe[p].PINTFLAG.reg&USB_HOST_PINTFLAG_TRCPT1) == USB_HOST_PINTFLAG_TRCPT1)
#define uhd_ack_in_received0(p)                   USB->HOST.HostPipe[p].PINTFLAG.reg = USB_HOST_PINTFLAG_TRCPT0
#define uhd_ack_in_received1(p)                   USB->HOST.HostPipe[p].PINTFLAG.reg = USB_HOST_PINTFLAG_TRCPT1
#define uhd_byte_count0(p)                        usb_pipe_table[p].HostDescBank[0].PCKSIZE.bit.BYTE_COUNT
#define uhd_byte_count1(p)                        usb_pipe_table[p].HostDescBank[1].PCKSIZE.bit.BYTE_COUNT
#define Is_uhd_in_ready0(p)                       ((USB->HOST.HostPipe[p].PSTATUS.reg&USB_HOST_PSTATUS_BK0RDY) == USB_HOST_PSTATUS_BK0RDY)  
#define uhd_ack_in_ready0(p)                       USB->HOST.HostPipe[p].PSTATUSCLR.reg = USB_HOST_PSTATUSCLR_BK0RDY
#define Is_uhd_in_ready1(p)                       ((USB->HOST.HostPipe[p].PSTATUS.reg&USB_HOST_PSTATUS_BK1RDY) == USB_HOST_PSTATUS_BK1RDY)  
#define uhd_ack_in_ready1(p)                       USB->HOST.HostPipe[p].PSTATUSCLR.reg = USB_HOST_PSTATUSCLR_BK1RDY
#define uhd_current_bank(p)                       ((USB->HOST.HostPipe[p].PSTATUS.reg&USB_HOST_PSTATUS_CURBK) == USB_HOST_PSTATUS_CURBK)  
#define Is_uhd_toggle(p)                          ((USB->HOST.HostPipe[p].PSTATUS.reg&USB_HOST_PSTATUS_DTGL) == USB_HOST_PSTATUS_DTGL)  
#define Is_uhd_toggle_error0(p)                   usb_pipe_table[p].HostDescBank[0].STATUS_PIPE.bit.DTGLER
#define Is_uhd_toggle_error1(p)                   usb_pipe_table[p].HostDescBank[1].STATUS_PIPE.bit.DTGLER

USBHost UsbH;
USBHub Hub1(&UsbH);
USBH_MIDI  Midi1(&UsbH);
USBH_MIDI  Midi2(&UsbH);

bool doPipeConfig = true;
bool runTask = true;

//SAMD21 datasheet pg 836. ADDR location needs to be aligned. 
uint8_t bufBk0[64] __attribute__ ((aligned (4))); //Bank0
uint8_t bufBk1[64] __attribute__ ((aligned (4))); //Bank1


void setup()
{
  //bFirst = true;
  //vid = pid = 0;
  SerialDebug.begin(115200);
  if (UsbH.Init()) {
    SerialDebug.println("USB host did not start");
    while (1); //halt
  }
  //USB_SetHandler(&CUSTOM_UHD_Handler);
  delay( 200 );
}

void loop()
{
  
  /*
  Serial1.print("|Task State:");
  Serial1.print(UsbH.getUsbTaskState(),HEX);
  Serial1.print("|Hub1 address:");
  Serial1.print(Hub1.GetAddress(),HEX);
  if (Midi1) {
    Serial1.print("|Midi1 address:");
    Serial1.print(Midi1.GetAddress(),HEX);
    Serial1.print("|Midi1 EP address:");
    Serial1.print(Midi1.GetEpAddress(),HEX);
  }
  if (Midi2) {
    Serial1.print("|Midi2 address:");
    Serial1.print(Midi2.GetAddress(),HEX);
    Serial1.print("|Midi2 EP address:");
    Serial1.print(Midi2.GetEpAddress(),HEX);
  }
  Serial1.println("");
  */
  
  if (runTask) {
    Serial1.println("Calling Usbh.Task()");
    UsbH.Task(); 
  }
  if ( UsbH.getUsbTaskState() == USB_STATE_RUNNING ) {
    if ((Hub1.GetAddress() !=0) && (doPipeConfig)) {
      runTask = false;
      doPipeConfig = false;
      UsbH.Task(); //Sets up EP1 for the Hub
      uint32_t epAddr = 1; //Hub EP.
      usb_pipe_table[epAddr].HostDescBank[0].ADDR.reg = (uint32_t)bufBk0;
      usb_pipe_table[epAddr].HostDescBank[1].ADDR.reg = (uint32_t)bufBk1;
      USB->HOST.HostPipe[epAddr].PSTATUSCLR.reg = USB_HOST_PSTATUSCLR_BK0RDY;
      uhd_unfreeze_pipe(epAddr); 
      USB->HOST.HostPipe[epAddr].PINTENSET.reg = 0x3B; //Enable pipe interrupts
      USB_SetHandler(&CUSTOM_UHD_Handler);
    }
 
  } else {
    USB_SetHandler(&CUSTOM_UHD_Handler);
    USB->HOST.HostPipe[1].PINTENCLR.reg = 0xFF; //Disable pipe interrupts
  }
  
}


void CUSTOM_UHD_Handler(void)
{
  SerialDebug.println("Handler Called");
  uint32_t epAddr = 1;
  if (USB->HOST.INTFLAG.reg == USB_HOST_INTFLAG_DCONN) {
    SerialDebug.println("Connected");
    //doPipeConfig1 = true;
  } else if (USB->HOST.INTFLAG.reg == USB_HOST_INTFLAG_DDISC) {
    SerialDebug.println("Disconnected");
    USB->HOST.HostPipe[epAddr].PINTENCLR.reg = 0xFF; //Disable pipe interrupts
  }
  UHD_Handler();
  uhd_freeze_pipe(epAddr);
  /*
  SerialDebug.print(USB->HOST.INTFLAG.reg,HEX);
  SerialDebug.print(":");
  SerialDebug.print(USB->HOST.HostPipe[epAddr].PINTFLAG.reg,HEX);
  SerialDebug.print(":");
  SerialDebug.print(USB->HOST.HostPipe[epAddr].PSTATUS.reg,HEX);
  SerialDebug.print(":");
  SerialDebug.print("|STATUS0:");
  SerialDebug.print(usb_pipe_table[epAddr].HostDescBank[0].STATUS_PIPE.reg,HEX);
  SerialDebug.print("|STATUS1:");
  SerialDebug.print(usb_pipe_table[epAddr].HostDescBank[1].STATUS_PIPE.reg,HEX);
  SerialDebug.print("|STATUS_BK0:");
  SerialDebug.print(usb_pipe_table[epAddr].HostDescBank[0].STATUS_BK.reg,HEX);
  SerialDebug.print("|STATUS_BK1:");
  SerialDebug.print(usb_pipe_table[epAddr].HostDescBank[1].STATUS_BK.reg,HEX);
  SerialDebug.print("|BYTECOUNT0:");
  SerialDebug.print(uhd_byte_count0(epAddr),HEX);
  SerialDebug.print("|BYTECOUNT1:");
  SerialDebug.print(uhd_byte_count1(epAddr),HEX);
  SerialDebug.print("|TRCPT0:");
  SerialDebug.print(Is_uhd_in_received0(epAddr),HEX);
  SerialDebug.print("|TRCPT1:");
  SerialDebug.print(Is_uhd_in_received1(epAddr),HEX);
  SerialDebug.print("|READY0:");
  SerialDebug.print(Is_uhd_in_ready0(epAddr),HEX);
  SerialDebug.print("|READY1:");
  SerialDebug.print(Is_uhd_in_ready1(epAddr),HEX);
  SerialDebug.print("|CURRBK:");
  SerialDebug.print(uhd_current_bank(epAddr),HEX);
  SerialDebug.print("|TOGGLE:");
  SerialDebug.print(Is_uhd_toggle(epAddr),HEX);
  SerialDebug.print("|TOGGLE_ERROR0:");
  SerialDebug.print(Is_uhd_toggle_error0(epAddr),HEX);
  SerialDebug.print("|TOGGLE_ERROR1:");
  SerialDebug.print(Is_uhd_toggle_error1(epAddr),HEX);
  SerialDebug.print("|NAK:");
  SerialDebug.print(Is_uhd_nak_received(epAddr),HEX);
  SerialDebug.print("|INTSUMMARY:");
  SerialDebug.print(uhd_endpoint_interrupt(),HEX);
  SerialDebug.print("|");
  SerialDebug.println("");
  */
  if (USB->HOST.INTFLAG.reg == USB_HOST_INTFLAG_DCONN) {
    SerialDebug.println("Connected");
    //disconnected = false;
    doPipeConfig = true;
  } else if (USB->HOST.INTFLAG.reg == USB_HOST_INTFLAG_DDISC) {
    //disconnected = true;
    //uhd_unfreeze_pipe(epAddr);
    SerialDebug.println("Disconnected");
    USB->HOST.HostPipe[epAddr].PINTENCLR.reg = 0xFF; //Disable pipe interrupts
  }

  //Both banks full and bank1 is oldest, so process first. 
  if (Is_uhd_in_received0(epAddr) && Is_uhd_in_received1(epAddr) && uhd_current_bank(epAddr)) {
    handleBank1(epAddr);
  }
  if(Is_uhd_in_received0(epAddr)) {
    handleBank0(epAddr);
  } 
  if (Is_uhd_in_received1(epAddr)) {
    handleBank1(epAddr);
  }
  uhd_unfreeze_pipe(epAddr);    
}

void handleBank0(uint32_t epAddr){
    int rcvd = uhd_byte_count0(epAddr);
    String dataString = "";
    if (rcvd == 1) {
     dataString = String(bufBk0[0],HEX);
     SerialDebug.print(dataString);
     SerialDebug.print("|");
     Hub1.CheckHubStatus(bufBk0[0]);
    } else {
     for (int i = 0; i < rcvd; i++) {
      if (bufBk0[i] > 0) {
        dataString = String(bufBk0[i],HEX);
        i++;
        dataString = dataString + String(bufBk0[i],HEX);
        i++;
        dataString = dataString + String(bufBk0[i],HEX);
        i++;
        dataString = dataString + String(bufBk0[i],HEX);
        SerialDebug.print(dataString);
        SerialDebug.print("|");
        }
      }      
    }

    if (dataString != "") {
      SerialDebug.println("");
    }
    uhd_ack_in_received0(epAddr);
    uhd_ack_in_ready0(epAddr);
}

void handleBank1(uint32_t epAddr){
    int rcvd = uhd_byte_count1(epAddr);
    String dataString = "";
    if (rcvd == 1) {
     dataString = String(bufBk0[0],HEX);
     SerialDebug.print(dataString);
     SerialDebug.print("|");
     Hub1.CheckHubStatus(bufBk1[0]);
    } else {
     for (int i = 0; i < rcvd; i++) {
      if (bufBk1[i] > 0) {
        dataString = String(bufBk1[0],HEX);
        i++;
        dataString = dataString + String(bufBk1[i],HEX);
        i++;
        dataString = dataString + String(bufBk1[i],HEX);
        i++;
        dataString = dataString + String(bufBk1[i],HEX);
        SerialDebug.print(dataString);
        SerialDebug.print("|");
        }
      } 
    }

    if (dataString != "") {
      SerialDebug.println("");
    }
    uhd_ack_in_received1(epAddr);
    uhd_ack_in_ready1(epAddr);
}
