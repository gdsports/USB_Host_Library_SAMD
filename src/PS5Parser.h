/* Copyright (C) 2014 Kristian Lauszus, TKJ Electronics. All rights reserved.

 This software may be distributed and modified under the terms of the GNU
 General Public License version 2 (GPL2) as published by the Free Software
 Foundation and appearing in the file GPL2.TXT included in the packaging of
 this file. Please note that GPL2 Section 2[b] requires that all works based
 on this software must also be made publicly available under the terms of
 the GPL2 ("Copyleft").

 Contact information
 -------------------

 Kristian Lauszus, TKJ Electronics
 Web      :  http://www.tkjelectronics.com
 e-mail   :  kristianl@tkjelectronics.com

 PS5 support by Joseph Duchesne
 */

#ifndef _ps5parser_h_
#define _ps5parser_h_

#include "Usb.h"
#include "controllerEnums.h"
#include "PS5Trigger.h"

/** Buttons on the controller */
const uint8_t PS5_BUTTONS[] PROGMEM = {
        UP, // UP
        RIGHT, // RIGHT
        DOWN, // DOWN
        LEFT, // LEFT

        0x0C, // SHARE
        0x0D, // OPTIONS
        0x0E, // L3
        0x0F, // R3

        0x0A, // L2
        0x0B, // R2
        0x08, // L1
        0x09, // R1

        0x07, // TRIANGLE
        0x06, // CIRCLE
        0x05, // CROSS
        0x04, // SQUARE

        0x10, // PS
        0x11, // TOUCHPAD
};

union PS5Buttons {
        struct {
                uint8_t dpad : 4;
                uint8_t square : 1;
                uint8_t cross : 1;
                uint8_t circle : 1;
                uint8_t triangle : 1;

                uint8_t l1 : 1;
                uint8_t r1 : 1;
                uint8_t l2 : 1;
                uint8_t r2 : 1;
                uint8_t share : 1;
                uint8_t menu : 1;
                uint8_t left_stick : 1;
                uint8_t right_stick : 1;
                

                uint8_t ps : 1;
                uint8_t touchpad : 1;
                uint8_t mic : 1;
                uint8_t dummy : 5;
        } __attribute__((packed));
        uint32_t val : 24;
} __attribute__((packed));

struct touchpadXY {
        struct {
                uint8_t counter : 7; // Increments every time a finger is touching the touchpad
                uint8_t touching : 1; // The top bit is cleared if the finger is touching the touchpad
                uint16_t x : 12;
                uint16_t y : 12;
        } __attribute__((packed)) finger[2]; // 0 = first finger, 1 = second finger
} __attribute__((packed));

struct PS5Status {
        // first byte
        uint8_t headphone : 1;
        uint8_t dummy : 2;
        uint8_t usb : 1;  // charging
        uint8_t dummy2: 4;

        // second byte
        uint8_t battery : 4;
        uint8_t dummy3 : 1;
        uint8_t battery_full : 1;
        uint8_t dummy4 : 2;
} __attribute__((packed));

struct PS5Data {
        /* Button and joystick values */
        uint8_t hatValue[4];  // 0-3 bytes
        uint8_t trigger[2];  // 4-5
        
        uint8_t dummy;  // 6 unknown
        
        PS5Buttons btn;      // 7-9

        uint8_t dummy2[5];   // 0xA-0xD unknown

        /* Gyro and accelerometer values */
        int16_t gyroX, gyroZ, gyroY;   // 0x0F - 0x14
        int16_t accX, accZ, accY; // 0x15-0x1A

        uint8_t dummy3[5];  // 0x1B - 0x1F unknown

        // 0x20 - 0x23 touchpad point 1
        // 0x24 - 0x27 touchpad point 2
        touchpadXY xy;

        uint8_t dummy4;  //0x28 unknown
        
        uint8_t rightTriggerFeedback; // 0x29
        uint8_t leftTriggerFeedback;  // 0x2A

        uint8_t dummy5[10];  // 0x2B - 0x34 unknown

        // status bytes 0x35-0x36
        PS5Status status;
        
} __attribute__((packed));

struct PS5Output {
        uint8_t bigRumble, smallRumble; // Rumble
        uint8_t microphoneLed;
        uint8_t disableLeds;
        uint8_t playerLeds;
        uint8_t r, g, b; // RGB for lightbar
        bool reportChanged; // The data is send when data is received from the controller
} __attribute__((packed));

/** This class parses all the data sent by the PS5 controller */
class PS5Parser {
public:
        PS5Trigger leftTrigger;
        PS5Trigger rightTrigger;

        /** Constructor for the PS5Parser class. */
        PS5Parser() : leftTrigger(), rightTrigger() {
                Reset();
        };

        /** @name PS5 Controller functions */
        /**
         * getButtonPress(ButtonEnum b) will return true as long as the button is held down.
         *
         * While getButtonClick(ButtonEnum b) will only return it once.
         *
         * So you instance if you need to increase a variable once you would use getButtonClick(ButtonEnum b),
         * but if you need to drive a robot forward you would use getButtonPress(ButtonEnum b).
         * @param  b          ::ButtonEnum to read.
         * @return            getButtonPress(ButtonEnum b) will return a true as long as a button is held down, while getButtonClick(ButtonEnum b) will return true once for each button press.
         */
        bool getButtonPress(ButtonEnum b);
        bool getButtonClick(ButtonEnum b);
        /**@}*/
        /** @name PS5 Controller functions */
        /**
         * Used to get the analog value from button presses.
         * @param  b The ::ButtonEnum to read.
         * The supported buttons are:
         * ::UP, ::RIGHT, ::DOWN, ::LEFT, ::L1, ::L2, ::R1, ::R2,
         * ::TRIANGLE, ::CIRCLE, ::CROSS, ::SQUARE, and ::T.
         * @return   Analog value in the range of 0-255.
         */
        uint8_t getAnalogButton(ButtonEnum b);

        /**
         * Used to read the analog joystick.
         * @param  a ::LeftHatX, ::LeftHatY, ::RightHatX, and ::RightHatY.
         * @return   Return the analog value in the range of 0-255.
         */
        uint8_t getAnalogHat(AnalogHatEnum a);

        /**
         * Get the x-coordinate of the touchpad. Position 0 is in the top left.
         * @param  finger 0 = first finger, 1 = second finger. If omitted, then 0 will be used.
         * @param  xyId   The controller sends out three packets with the same structure.
         *                The third one will contain the last measure, but if you read from the controller then there is only be data in the first one.
         *                For that reason it will be set to 0 if the argument is omitted.
         * @return        Returns the x-coordinate of the finger.
         */
        uint16_t getX(uint8_t finger = 0, uint8_t xyId = 0) {
                return ps5Data.xy[xyId].finger[finger].x;
        };

        /**
         * Get the y-coordinate of the touchpad. Position 0 is in the top left.
         * @param  finger 0 = first finger, 1 = second finger. If omitted, then 0 will be used.
         * @param  xyId   The controller sends out three packets with the same structure.
         *                The third one will contain the last measure, but if you read from the controller then there is only be data in the first one.
         *                For that reason it will be set to 0 if the argument is omitted.
         * @return        Returns the y-coordinate of the finger.
         */
        uint16_t getY(uint8_t finger = 0, uint8_t xyId = 0) {
                return ps5Data.xy[xyId].finger[finger].y;
        };

        /**
         * Returns whenever the user is toucing the touchpad.
         * @param  finger 0 = first finger, 1 = second finger. If omitted, then 0 will be used.
         * @param  xyId   The controller sends out three packets with the same structure.
         *                The third one will contain the last measure, but if you read from the controller then there is only be data in the first one.
         *                For that reason it will be set to 0 if the argument is omitted.
         * @return        Returns true if the specific finger is touching the touchpad.
         */
        bool isTouching(uint8_t finger = 0, uint8_t xyId = 0) {
                return !(ps5Data.xy[xyId].finger[finger].touching); // The bit is cleared when a finger is touching the touchpad
        };

        /**
         * This counter increments every time a finger touches the touchpad.
         * @param  finger 0 = first finger, 1 = second finger. If omitted, then 0 will be used.
         * @param  xyId   The controller sends out three packets with the same structure.
         *                The third one will contain the last measure, but if you read from the controller then there is only be data in the first one.
         *                For that reason it will be set to 0 if the argument is omitted.
         * @return        Return the value of the counter, note that it is only a 7-bit value.
         */
        uint8_t getTouchCounter(uint8_t finger = 0, uint8_t xyId = 0) {
                return ps5Data.xy[xyId].finger[finger].counter;
        };

        /**
         * Get the angle of the controller calculated using the accelerometer.
         * @param  a Either ::Pitch or ::Roll.
         * @return   Return the angle in the range of 0-360.
         */
        float getAngle(AngleEnum a) {
                if (a == Pitch)
                        return (atan2f(ps5Data.accY, ps5Data.accZ) + PI) * RAD_TO_DEG;
                else
                        return (atan2f(ps5Data.accX, ps5Data.accZ) + PI) * RAD_TO_DEG;
        };

        /**
         * Used to get the raw values from the 3-axis gyroscope and 3-axis accelerometer inside the PS5 controller.
         * @param  s The sensor to read.
         * @return   Returns the raw sensor reading.
         */
        int16_t getSensor(SensorEnum s) {
                switch(s) {
                        case gX:
                                return ps5Data.gyroX;
                        case gY:
                                return ps5Data.gyroY;
                        case gZ:
                                return ps5Data.gyroZ;
                        case aX:
                                return ps5Data.accX;
                        case aY:
                                return ps5Data.accY;
                        case aZ:
                                return ps5Data.accZ;
                        default:
                                return 0;
                }
        };

        /**
         * Return the battery level of the PS5 controller.
         * @return The battery level in the range 0-15.
         */
        uint8_t getBatteryLevel() {
                return ps5Data.status.battery;
        };

        /**
         * Use this to check if an USB cable is connected to the PS5 controller.
         * @return Returns true if an USB cable is connected.
         */
        bool getUsbStatus() {
                return ps5Data.status.usb;
        };

        /**
         * Use this to check if an audio jack cable is connected to the PS5 controller.
         * @return Returns true if an audio jack cable is connected.
         */
        bool getAudioStatus() {
                return ps5Data.status.headphone;
        };

        /**
         * Use this to check if a microphone is connected to the PS5 controller.
         * @return Returns true if a microphone is connected.
         */
        bool getMicStatus() {
                return false;  // TODO: fix this. return ps5Data.status.mic;
        };

        /** Turn both rumble and the LEDs off. */
        void setAllOff() {
                setRumbleOff();
                setLedOff();
        };

        /** Set rumble off. */
        void setRumbleOff() {
                setRumbleOn(0, 0);
        };

        /**
         * Turn on rumble.
         * @param mode Either ::RumbleHigh or ::RumbleLow.
         */
        void setRumbleOn(RumbleEnum mode) {
                if (mode == RumbleLow)
                        setRumbleOn(0x00, 0xFF);
                else
                        setRumbleOn(0xFF, 0x00);
        };

        /**
         * Turn on rumble.
         * @param bigRumble   Value for big motor.
         * @param smallRumble Value for small motor.
         */
        void setRumbleOn(uint8_t bigRumble, uint8_t smallRumble) {
                ps5Output.bigRumble = bigRumble;
                ps5Output.smallRumble = smallRumble;
                ps5Output.reportChanged = true;
        };


        /** Turn all LEDs off. */
        void setLedOff() {
                setLed(0, 0, 0);
        };

        /**
         * Use this to set the color using RGB values.
         * @param r,g,b RGB value.
         */
        void setLed(uint8_t r, uint8_t g, uint8_t b) {
                ps5Output.r = r;
                ps5Output.g = g;
                ps5Output.b = b;
                ps5Output.reportChanged = true;
        };

        /**
         * Use this to set the color using the predefined colors in ::ColorsEnum.
         * @param color The desired color.
         */
        void setLed(ColorsEnum color) {
                setLed((uint8_t)(color >> 16), (uint8_t)(color >> 8), (uint8_t)(color));
        };

        uint16_t getMessageCounter(){
          return message_counter;
        }

protected:
        /**
         * Used to parse data sent from the PS5 controller.
         * @param len Length of the data.
         * @param buf Pointer to the data buffer.
         */
        void Parse(uint8_t len, uint8_t *buf);

        /** Used to reset the different buffers to their default values */
        void Reset();

        /**
         * Send the output to the PS5 controller. This is implemented in PS5BT.h and PS5USB.h.
         * @param output Pointer to PS5Output buffer;
         */
        virtual void sendOutputReport(PS5Output *output) = 0;


private:
        bool checkDpad(ButtonEnum b); // Used to check PS5 DPAD buttons

        PS5Data ps5Data;
        PS5Buttons oldButtonState, buttonClickState;
        PS5Output ps5Output;
        uint8_t oldDpad;
        uint16_t message_counter = 0;
};
#endif
