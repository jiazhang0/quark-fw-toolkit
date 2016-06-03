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

Usage
-----

- Show the information of firmware (if current machine is quark-based)
# cln_fwtool show

- Show the diagnosis information of firmware (if current machine is quark-based)
# cln_fwtool diagnosis

- Show the information of firmware image
$ cln_fwtool show test/Flash-crosshill-8M-secure.bin

- Embed UEFI secure boot keys to a firmware image
$ cln_fwtool sbembed Flash-crosshill-8M-secure.bin \
	--pk=owner-cert.cer --kek=vendor-cert.cer --db=vendor-cert.cer

- Convert the firmware image to an unsigned capsule image
$ cln_fwtool capsule test/Flash-crosshill-8M-secure.bin \
	-o output_unsigned_8M.cap

Clanton Support
---------------

Intel Clanton SoC is the codename of the first generation of Intel Quark
processor family. It is also unknown as Intel Quark SoC X1000 series.

cln_fwtool is the dedicated tool to support the firmware image used for
Clanton SoC. Also, you can embed the PK, KEK and/or DB to a firmware image
to enable UEFI secure boot with the keys you own. Furthermore, cln_fwtool
can provide the diagnosis information as a guideline according to the
content of the firmware.

The sub-commands show and diagnosis can work on a real clanton-based board.
In this case, instead of loading a firmware file, cln_fwtool can detect
the physical memory region where the flash chip resides on.
