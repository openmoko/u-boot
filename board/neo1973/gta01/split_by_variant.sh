#!/bin/sh
# ---------------------------------------------------------
#  Set the core module defines according to Core Module
# ---------------------------------------------------------
# ---------------------------------------------------------
# Set up the GTA01 type define
# ---------------------------------------------------------

CFGINC=${obj}include/config.h
CFGTMP=${obj}board/neo1973/gta01/config.tmp

mkdir -p ${obj}include
if [ "$1" = "" ]
then
	echo "$0:: No parameters - using GTA01Bv3 config"
	echo "#define CONFIG_ARCH_GTA01B_v3" > $CFGINC
	echo "GTA01_BIG_RAM=y" > $CFGTMP
	echo "CONFIG_USB_DFU_REVISION=0x0230" > $CFGTMP
else
	case "$1" in
	gta01v4_config)
	echo "#define CONFIG_ARCH_GTA01_v4" > $CFGINC
	echo "GTA01_BIG_RAM=n" > $CFGTMP
	echo "CONFIG_USB_DFU_REVISION=0x0140" > $CFGTMP
	;;

	gta01v3_config)
	echo "#define CONFIG_ARCH_GTA01_v3" > $CFGINC
	echo "GTA01_BIG_RAM=n" > $CFGTMP
	echo "CONFIG_USB_DFU_REVISION=0x0130" > $CFGTMP
	;;

	gta01bv2_config)
	echo "#define CONFIG_ARCH_GTA01B_v2" > $CFGINC
	echo "GTA01_BIG_RAM=y" > $CFGTMP
	echo "CONFIG_USB_DFU_REVISION=0x0220" > $CFGTMP
	;;

	gta01bv3_config)
	echo "#define CONFIG_ARCH_GTA01B_v3" > $CFGINC
	echo "GTA01_BIG_RAM=y" > $CFGTMP
	echo "CONFIG_USB_DFU_REVISION=0x0230" > $CFGTMP
	;;

	gta01bv4_config)
	echo "#define CONFIG_ARCH_GTA01B_v4" > $CFGINC
	echo "GTA01_BIG_RAM=y" > $CFGTMP
	echo "CONFIG_USB_DFU_REVISION=0x0240" > $CFGTMP
	;;

	*)
	echo "$0:: Unrecognised config - using GTA01Bv4 config"
	echo "#define CONFIG_ARCH_GTA01B_v4" > $CFGINC
	echo "GTA01_BIG_RAM=y" > $CFGTMP
	echo "CONFIG_USB_DFU_REVISION=0x0240" > $CFGTMP
	;;

	esac

fi
sed 's/^/#define /;s/=/ /' <$CFGTMP >>$CFGINC
# ---------------------------------------------------------
# Complete the configuration
# ---------------------------------------------------------
$MKCONFIG -a neo1973_gta01 arm arm920t gta01 neo1973 s3c24x0
