/*
 *******************************************************************************
 * USB-MIDI dump utility
 * Copyright (C) 2013-2017 Yuuichi Akagawa
 *
 * for use with USB Host Shield 2.0 from Circuitsathome.com
 * https://github.com/felis/USB_Host_Shield_2.0
 *
 * This is sample program. Do not expect perfect behavior.
 *******************************************************************************
 */

#include <usbh_midi.h>
#include <usbhub.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#include <SPI.h>
#endif

USBHost UsbH;
USBHub Hub(&UsbH);
USBH_MIDI  Midi(&UsbH);

void MIDI_poll();
void doDelay(uint32_t t1, uint32_t t2, uint32_t delayTime);

boolean bFirst;
uint16_t pid, vid;

void setup()
{
  bFirst = true;
  vid = pid = 0;
  SERIAL_PORT_MONITOR.begin(115200);

  if (UsbH.Init()) {
    Serial.println("USB host did not start");
    while (1); //halt
  }
  delay( 200 );
}

void loop()
{
  UsbH.Task();
  //uint32_t t1 = (uint32_t)micros();
  if ( UsbH.getUsbTaskState() == USB_STATE_RUNNING )
  {
    uint8_t midimsg[4] = { 0x01, 0x02 , 0x03, 0x04 };
    //Midi.SendRawData(sizeof(midimsg), midimsg);
    MIDI_poll();
  }
  //delay(1ms)
  //doDelay(t1, (uint32_t)micros(), 1000);
}

// Poll USB MIDI Controler and send to serial MIDI
void MIDI_poll()
{
  char buf[20];
  uint8_t bufMidi[64];
  uint8_t  rcvd;

  if (Midi.vid != vid || Midi.pid != pid) {
    sprintf(buf, "VID:%04X, PID:%04X", Midi.vid, Midi.pid);
    SERIAL_PORT_MONITOR.println(buf);
    vid = Midi.vid;
    pid = Midi.pid;
  }
  if (Midi.RecvData(&rcvd,  bufMidi) == 0 ) {
    uint32_t time = (uint32_t)millis();
    sprintf(buf, "%04X%04X: ", (uint16_t)(time >> 16), (uint16_t)(time & 0xFFFF)); // Split variable to prevent warnings on the ESP8266 platform
    SERIAL_PORT_MONITOR.print(buf);
    SERIAL_PORT_MONITOR.print(rcvd);
    SERIAL_PORT_MONITOR.print(':');
    for (int i = 0; i < 64; i++) {
      sprintf(buf, " %02X", bufMidi[i]);
      SERIAL_PORT_MONITOR.print(buf);
    }
    SERIAL_PORT_MONITOR.println();
  }
}

// Delay time (max 16383 us)
void doDelay(uint32_t t1, uint32_t t2, uint32_t delayTime)
{
  uint32_t t3;

  if ( t1 > t2 ) {
    t3 = (0xFFFFFFFF - t1 + t2);
  } else {
    t3 = t2 - t1;
  }

  if ( t3 < delayTime ) {
    delayMicroseconds(delayTime - t3);
  }
}
