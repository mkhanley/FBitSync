FBit Sync
===================

**Not working as of 12/11/16 due to update in Fitbit API**

FBit Sync is a client to synchonise Fitbit devices with a Fitbit account using the supplied Fitbit dongle on Linux systems.

This project was a learning project to port [galileo](https://bitbucket.org/benallard/galileo) from Python to C++


Dependencies
------------
[Boost](https://boost.org)

[libusb](https://libusb.info)

[cpr](https://github.com/whoshuu/cpr) 


  - Required Libraries for cpr 
  
  libssl-dev
  
	libssh2-dev

Execution
---------
Build and compile the project with cmake and make

Copy the 99-FBit.rules file to /etc/udev/rules.d

Run `sudo service restart udev`

Unplug and replug the Fitbit dongle

Execute with `./FBitSync`



Supported Devices
-----------------
Charge HR


Credit
------
All credit for the original source code, udev rule, and reverse engineering the synchronisation protocol go to the guys over at [galileo](https://bitbucket.org/benallard/galileo)
