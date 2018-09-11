#include <adk.h>
#include <usbhub.h>

// Satisfy IDE, which only needs to see the include statment in the ino.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

USBHost UsbH;
//USBHub     Hub(&UsbH);

ADK adk(&UsbH, "Circuits@Home, ltd.",
              "USB Host Shield",
              "Arduino Terminal for Android",
              "1.0",
              "http://www.circuitsathome.com",
              "0000000000000001");

void setup()
{
  Serial.begin(115200);
#if !defined(__MIPSEL__)
  while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
  Serial.println("\r\nADK demo start");

  if (UsbH.Init()) {
    Serial.println("USB host failed to assert");
    while (1); //halt
  }//if (UsbH.Init() == -1...
}

void loop()
{
  uint8_t rcode;
  uint8_t msg[64] = { 0x00 };
  const char* recv = "Received: ";

  UsbH.Task();

  if ( adk.isReady() == false ) {
    return;
  }
  uint16_t len = 64;

  rcode = adk.RcvData(&len, msg);
  if ( rcode & ( rcode != USB_ERRORFLOW )) {
    USBTRACE2("Data rcv. :", rcode );
  }
  if (len > 0) {
    USBTRACE("\r\nData Packet.");

    for ( uint8_t i = 0; i < len; i++ ) {
      Serial.print((char)msg[i]);
    }
    /* sending back what was received */
    rcode = adk.SndData( strlen( recv ), (uint8_t *)recv );
    if (rcode && rcode != USB_ERRORFLOW) {
      Serial.print(F("\r\nData send: "));
      Serial.print(rcode, HEX);
    }
    rcode = adk.SndData( strlen(( char * )msg ), msg );
    if (rcode && rcode != USB_ERRORFLOW) {
      Serial.print(F("\r\nData send: "));
      Serial.print(rcode, HEX);
    }

  }//if( len > 0 )...

  delay( 1000 );
}

