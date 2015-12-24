quark-fw-toolkit
================

This project provides tools and library to easily dump, modify and generate
firmware image used on Intel Quark processor family.

Build
-----

$ make

Note: use "CROSS_COMPILE=" in command line to specify cross compilation.

Installation
------------

$ sudo make install
$ sudo ldconfig

Help
----

$ cln_fwtool -h

Clanton Support
---------------

Intel Clanton SoC is the codename of the first generation of Intel Quark
processor family. It is also unknown as Intel Quark SoC X1000 series.

cln_fwtool is the dedicated tool to support the firmware image used for
Clanton SoC. Also, you can embed the PK, KEK and/or DB to a firmware image
to enable UEFI secure boot with the keys you own. Furthermore, cln_fwtool
can provide the diagnosis information as a guideline according to the
content of the firmware.
