quark-fw-toolkit
================

This project provides tool and library to easily dump, operate and modify
firmware used on Intel Quark processor family.

Build
-----

$ make

Note: use "CROSS_COMPILE=" in command line to specify cross compilation.

Installation
------------

$ sudo make install
$ sudo ldconfig

Clanton Support
---------------

Intel Clanton SoC is the codename of the first generation of Intel Quark
processor family. It is also unknown as Quark X1000 series.

cln_fwtool is the dedicated tool to support the flash image file used for
Clanton SoC. Also, you can embed the PK, KEK and/or DB to a flash image to
enable UEFI secure boot with the keys you own.
