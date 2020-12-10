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

 
 /* Note this example should work with one MIDI device connected without a hub. 
  *  
  */

#include <usbh_midi.h>
#include <usbhub.h>

#define SerialDebug Serial1

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
USBHub Hub(&UsbH);
USBH_MIDI  Midi(&UsbH);

bool doPipeConfig = false;
bool runTask = true;

//SAMD21 datasheet pg 836. ADDR location needs to be aligned. 
uint8_t bufMidiBk0[64] __attribute__ ((aligned (4))); //Bank0
uint8_t bufMidiBk1[64] __attribute__ ((aligned (4))); //Bank1

void setup()
{
  //bFirst = true;
  //vid = pid = 0;
  SerialDebug.begin(115200);
  if (UsbH.Init()) {
    SerialDebug.println("USB host did not start");
    while (1); //halt
  }
  USB_SetHandler(&CUSTOM_UHD_Handler);
  delay( 200 );
}

void loop()
{
  UsbH.Task();
  
  if ( UsbH.getUsbTaskState() == USB_STATE_RUNNING ) {
    if ( Midi ) {
      if (doPipeConfig) {
        uint32_t epAddr = Midi.GetEpAddress();
        doPipeConfig = false;
        uint16_t rcvd;
        while (USB->HOST.HostPipe[Midi.GetEpAddress()].PCFG.bit.PTYPE != 0x03) {
          UsbH.Task(); 
          Midi.RecvData(&rcvd,  bufMidiBk0);
        }
        USB->HOST.HostPipe[epAddr].BINTERVAL.reg = 0x01;//Zero here caused bus resets.
        usb_pipe_table[epAddr].HostDescBank[0].ADDR.reg = (uint32_t)bufMidiBk0;
        usb_pipe_table[epAddr].HostDescBank[1].ADDR.reg = (uint32_t)bufMidiBk1;
        USB->HOST.HostPipe[epAddr].PCFG.bit.PTOKEN = tokIN;
        USB->HOST.HostPipe[epAddr].PSTATUSCLR.reg = USB_HOST_PSTATUSCLR_BK0RDY; 
        uhd_unfreeze_pipe(epAddr); //launch the transfer
        USB->HOST.HostPipe[epAddr].PINTENSET.reg = 0x3B; //Enable pipe interrupts

        SerialDebug.println("Pipe Started");
        SerialDebug.print("Dump:");
        SerialDebug.print("ADDR0:");
        SerialDebug.print(usb_pipe_table[epAddr].HostDescBank[0].ADDR.reg,HEX);
        SerialDebug.print(":");
        SerialDebug.print("ADDR1:");
        SerialDebug.print(usb_pipe_table[epAddr].HostDescBank[1].ADDR.reg,HEX);
        SerialDebug.print(":");
        SerialDebug.print(USB->HOST.INTFLAG.reg,HEX);
        SerialDebug.print(":");
        SerialDebug.print(USB->HOST.HostPipe[epAddr].PINTFLAG.reg,HEX);
        SerialDebug.print(":");
        SerialDebug.println(USB->HOST.HostPipe[epAddr].PSTATUS.reg,HEX);
      }
    }
  } else {
    USB_SetHandler(&CUSTOM_UHD_Handler);
    USB->HOST.HostPipe[Midi.GetEpAddress()].PINTENCLR.reg = 0xFF; //Disable pipe interrupts
  }
}


void CUSTOM_UHD_Handler(void)
{
  uint32_t epAddr = Midi.GetEpAddress();
  if (USB->HOST.INTFLAG.reg == USB_HOST_INTFLAG_DCONN) {
    SerialDebug.println("Connected");
    doPipeConfig = true;
  } else if (USB->HOST.INTFLAG.reg == USB_HOST_INTFLAG_DDISC) {
    SerialDebug.println("Disconnected");
    USB->HOST.HostPipe[epAddr].PINTENCLR.reg = 0xFF; //Disable pipe interrupts
  }
  UHD_Handler();
  uhd_freeze_pipe(epAddr);
  //SerialDebug.print(USB->HOST.INTFLAG.reg,HEX);
  //SerialDebug.print(":");
  //SerialDebug.print(USB->HOST.HostPipe[epAddr].PINTFLAG.reg,HEX);
  //SerialDebug.print(":");
  //SerialDebug.print(USB->HOST.HostPipe[epAddr].PSTATUS.reg,HEX);
  //SerialDebug.print(":");
  //SerialDebug.print("|STATUS0:");
  //SerialDebug.print(usb_pipe_table[epAddr].HostDescBank[0].STATUS_PIPE.reg,HEX);
  //SerialDebug.print("|STATUS1:");
  //SerialDebug.print(usb_pipe_table[epAddr].HostDescBank[1].STATUS_PIPE.reg,HEX);
  //SerialDebug.print("|STATUS_BK0:");
  //SerialDebug.print(usb_pipe_table[epAddr].HostDescBank[0].STATUS_BK.reg,HEX);
  //SerialDebug.print("|STATUS_BK1:");
  //SerialDebug.print(usb_pipe_table[epAddr].HostDescBank[1].STATUS_BK.reg,HEX);
  //SerialDebug.print("|BYTECOUNT0:");
  //SerialDebug.print(uhd_byte_count0(epAddr),HEX);
  //SerialDebug.print("|BYTECOUNT1:");
  //SerialDebug.print(uhd_byte_count1(epAddr),HEX);
  //SerialDebug.print("|TRCPT0:");
  //SerialDebug.print(Is_uhd_in_received0(epAddr),HEX);
  //SerialDebug.print("|TRCPT1:");
  //SerialDebug.print(Is_uhd_in_received1(epAddr),HEX);
  //SerialDebug.print("|READY0:");
  //SerialDebug.print(Is_uhd_in_ready0(epAddr),HEX);
  //SerialDebug.print("|READY1:");
  //SerialDebug.print(Is_uhd_in_ready1(epAddr),HEX);
  //SerialDebug.print("|CURRBK:");
  //SerialDebug.print(uhd_current_bank(epAddr),HEX);
  //SerialDebug.print("|TOGGLE:");
  //SerialDebug.print(Is_uhd_toggle(epAddr),HEX);
  //SerialDebug.print("|TOGGLE_ERROR0:");
  //SerialDebug.print(Is_uhd_toggle_error0(epAddr),HEX);
  //SerialDebug.print("|TOGGLE_ERROR1:");
  //SerialDebug.print(Is_uhd_toggle_error1(epAddr),HEX);
  //SerialDebug.print("|NAK:");
  //SerialDebug.print(Is_uhd_nak_received(epAddr),HEX);
  //SerialDebug.print("|INTSUMMARY:");
  //SerialDebug.print(uhd_endpoint_interrupt(),HEX);
  //SerialDebug.print("|");

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

/*
void pipeConfig(uint32_t addr, uint32_t epAddr) {
    SerialDebug.println("pipeConfig called");
    //UHD_Pipe_Alloc Pipe Configuration Datasheet 31.8.6
    //Note usb_pipe_table and DESCADDR done by UHD_Init() already.
    USB->HOST.HostPipe[epAddr].PCFG.reg = 0; //clear
    USB->HOST.HostPipe[epAddr].PCFG.bit.PTYPE = 0x3; //pipe enabled as BULK and frozen
    USB->HOST.HostPipe[epAddr].PCFG.bit.BK = USB_HOST_NB_BK_1; //dual bank
    USB->HOST.HostPipe[epAddr].BINTERVAL.reg = 0x01;//Zero here caused bus resets.
    USB->HOST.HostPipe[epAddr].PCFG.bit.PTOKEN = USB_HOST_PCFG_PTOKEN_IN;
    USB->HOST.HostPipe[epAddr].PSTATUSSET.reg  = USB_HOST_PSTATUSSET_BK0RDY;
    //Not sure why this is here. It's in UHD_Init() as well. Maybe to clear it.
    memset((uint8_t *)&usb_pipe_table[epAddr], 0, sizeof(usb_pipe_table[epAddr]));
    usb_pipe_table[epAddr].HostDescBank[0].CTRL_PIPE.bit.PDADDR = addr;
    usb_pipe_table[epAddr].HostDescBank[0].CTRL_PIPE.bit.PEPNUM = epAddr;
    usb_pipe_table[epAddr].HostDescBank[0].PCKSIZE.bit.SIZE     = USB_PCKSIZE_SIZE_64_BYTES;
    //End Pipe Configuration
}
*/

void handleBank0(uint32_t epAddr){
    int rcvd = uhd_byte_count0(epAddr);
    String dataString = "";
    for (int i = 0; i < rcvd; i++) {
      if (bufMidiBk0[i] > 0) {
        dataString = String(bufMidiBk0[i],HEX);
        i++;
        dataString = dataString + String(bufMidiBk0[i],HEX);
        i++;
        dataString = dataString + String(bufMidiBk0[i],HEX);
        i++;
        dataString = dataString + String(bufMidiBk0[i],HEX);
        SerialDebug.print(dataString);
        SerialDebug.print("|");
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
    for (int i = 0; i < rcvd; i++) {
      if (bufMidiBk1[i] > 0) {
        dataString = String(bufMidiBk1[i],HEX);
        i++;
        dataString = dataString + String(bufMidiBk1[i],HEX);
        i++;
        dataString = dataString + String(bufMidiBk1[i],HEX);
        i++;
        dataString = dataString + String(bufMidiBk1[i],HEX);
        SerialDebug.print(dataString);
        SerialDebug.print("|");
        }
      } 
    if (dataString != "") {
      SerialDebug.println("");
    }
    uhd_ack_in_received1(epAddr);
    uhd_ack_in_ready1(epAddr);
}
