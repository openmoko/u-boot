#
# (C) Copyright 2002
# Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
# David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
#
# FIC Neo1973 GTA01 board with S3C2410X (ARM920T) cpu
#
# see http://www.samsung.com/ for more information on SAMSUNG
#

# GTA01v3 has 1 bank of 64 MB SDRAM
# GTA01v4 has 1 bank of 64 MB SDRAM
#
# 	3000'0000 to 3400'0000
# we load ourself to 33F8'0000
#
# GTA01Bv2 or later has 1 bank of 128 MB SDRAM
#
# 	3000'0000 to 3800'0000
# we load ourself to 37F8'0000
#
# Linux-Kernel is expected to be at 3000'8000, entry 3000'8000
# optionally with a ramdisk at 3080'0000
#
# download area is 3200'0000 or 3300'0000

CONFIG_USB_DFU_VENDOR=0x1d50
CONFIG_USB_DFU_PRODUCT=0x5119

sinclude $(OBJTREE)/board/$(BOARDDIR)/config.tmp

TEXT_BASE = 0x33F80000
