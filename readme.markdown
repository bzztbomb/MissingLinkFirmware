# Intro

This is the bootloader and firmware for the AVR chip used with The Missing Link (http://wifimidi.com/).  It is Copyright 2011 Jabrudian Industries LLC.

# License

This code is licensed under GPL 3, see COPYING for more information.

# Getting the code onto the device

See bootloader/README for more details on compiling and using the bootloader.  There is a Makefile setup for use with avrdude and a AVRMKII ISP programmer in bootloader/avr_code.  The target is program.

To compile/burn the main firmware, first make sure that the uploader program under bootloader/uploader has been compiled.  Then go to the MissingLink directory and type make program.

# General overview

The AVR chip communicates to two other modules.  One is a WIFI module and the other is an USB MIDI module.  The WIFI module is controlled via SPI.  The USB MIDI module is controlled via I2C.  OSC packets are received via UDP and processed by the uIP/WiShield TCP/IP stack and code in udpapp.cpp.  This is then fed to the OSC library (http://recotana.com).  From here the message is dispatched to the code that responds to it (MissingLink.cpp).  For a normal OSC->MIDI message, handleTouchOSCMidi (HandleMidi.cpp) is called and uses code from Parse.cpp to parse the OSC address and send data to MIDI.  For MIDI input, the avrmidi library (http://www.x37v.info) is used to parse incoming midi and routed out to the appropiate destination.  Finally, I2C support is provided by the twi/arduino library and user code is at the bottom of HandleMidi.cpp.  (More to come here..)