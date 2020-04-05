/* Copyright (C) 2011 Circuits At Home, LTD. All rights reserved.

   This software may be distributed and modified under the terms of the GNU
   General Public License version 2 (GPL2) as published by the Free Software
   Foundation and appearing in the file GPL2.TXT included in the packaging of
   this file. Please note that GPL2 Section 2[b] requires that all works based
   on this software must also be made publicly available under the terms of
   the GPL2 ("Copyleft").

   Contact information
   -------------------

   Circuits At Home, LTD
Web      :  http://www.circuitsathome.com
e-mail   :  support@circuitsathome.com
*/
/* USB functions */

#ifndef USB_H_INCLUDED
#define USB_H_INCLUDED
#define _usb_h_

#include <stdint.h>

// Uncomment to enable debug
//#define DEBUG_USB_HOST 1

////////////////////////////////////////////////////////////////////////////////
// MASS STORAGE
////////////////////////////////////////////////////////////////////////////////
// <<<<<<<<<<<<<<<< IMPORTANT >>>>>>>>>>>>>>>
// Set this to 1 to support single LUN devices, and save RAM. -- I.E. thumb drives.
// Each LUN needs ~13 bytes to be able to track the state of each unit.
#ifndef MASS_MAX_SUPPORTED_LUN
#define MASS_MAX_SUPPORTED_LUN  1 ///// 8  WHG
#endif

//#include "Arduino.h"
#include "macros.h"
// None of these should ever be included by a driver, or a user's sketch.

#include "variant.h"
#if (USB_VID==0x2341 && defined(ARDUINO_SAMD_ZERO)) || (USB_VID==0x2a03 && defined(ARDUINO_SAM_ZERO))
  #define USB_HOST_SERIAL SERIAL_PORT_MONITOR
#else
  #define USB_HOST_SERIAL Serial1
#endif
#include "Print.h"
#include "printhex.h"
#include "message.h"
#include "hexdump.h"
#include "sink_parser.h"

#include "address.h"

#include "usb_ch9.h"
//#include "usbhost.h"
#include "UsbCore.h"
#include "parsetools.h"

#include "confdescparser.h"

#endif /* USB_H_INCLUDED */
