# USB Host Library SAMD
USB Host Library for Arduino SAMD boards

This library is based on the [USB Host Shield Library 2.0](https://github.com/felis/USB_Host_Shield_2.0) 
with modifications to work on Arduino SAMD boards. These should work on Zero,
M0, MKR family, etc. Third party SAMD boards may also work but has not been
tested yet.

A USB OTG to host cable or adapter is required.

Changes to the Arduino SAMD board package are required to use the library.

How to create the Arduino build environment using the IDE with board patches.
This works for a Linux system.

```
IDEVER="1.8.6"
SAMDVER="1.6.19"
# Change to home directory
cd
# Create work directory
mkdir arduino_samd_usb_host
cd arduino_samd_usb_host
WORKDIR=`pwd`
# Install Ardino IDE in work directory
wget -O arduino.tar.xz https://downloads.arduino.cc/arduino-${IDEVER}-linux64.tar.xz
tar xf arduino.tar.xz -C ${WORKDIR}
rm arduino.tar.xz
# Create portable sketchbook and library directories
# Using portable prevents these changes from affecting other Arduino projects.
IDEDIR="${WORKDIR}/arduino-${IDEVER}"
LIBDIR="${IDEDIR}/portable/sketchbook/libraries"
mkdir -p "${LIBDIR}"
cd ${IDEDIR}
# Install board package
./arduino --pref "compiler.warning_level=default" --save-prefs
./arduino --install-boards "arduino:samd"
BOARD="arduino:samd:arduino_zero_edbg"
./arduino --board "${BOARD}" --save-prefs
# Install MIDI library for USBH_MIDI examples
./arduino --install-library "MIDI Library"
# Install patches to SAMD board package
cd ${IDEDIR}/portable/packages/arduino/hardware/samd
rm -rf ${SAMDVER}
git clone https://github.com/gdsports/ArduinoCore-samd.git ${SAMDVER}
cd ${SAMDVER}
git checkout patch_bulk_ep
cd ${LIBDIR}
# Install TinyGPS for pl2303 example
git clone https://github.com/mikalhart/TinyGPS.git
# Install USB host library for SAMD
git clone https://github.com/gdsports/USB_Host_Library_SAMD
cd ${IDEDIR}
./arduino &
````
