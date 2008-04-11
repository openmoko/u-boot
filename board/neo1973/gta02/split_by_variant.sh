#!/bin/sh
# ---------------------------------------------------------
#  Set the core module defines according to Core Module
# ---------------------------------------------------------
# ---------------------------------------------------------
# Set up the GTA02 type define
# ---------------------------------------------------------

CFGINC=${obj}include/config.h
CFGTMP=${obj}board/neo1973/gta02/config.tmp

mkdir -p ${obj}include
if [ "$1" = "" ]
then
	echo "$0:: No parameters - using GTA02Bv1 config"
	echo "#define CONFIG_ARCH_GTA02_v1" > $CFGINC
	echo "#define CONFIG_GTA02_REVISION 1" >> $CFGINC
	echo "CONFIG_USB_DFU_REVISION=0x0310" > $CFGTMP
else
	case "$1" in
	gta02v1_config)
	echo "#define CONFIG_ARCH_GTA02_v1" > $CFGINC
	echo "#define CONFIG_GTA02_REVISION 1" >> $CFGINC
	echo "CONFIG_USB_DFU_REVISION=0x0310" > $CFGTMP
	;;

	gta02v2_config)
	echo "#define CONFIG_ARCH_GTA02_v2" > $CFGINC
	echo "#define CONFIG_GTA02_REVISION 2" >> $CFGINC
	echo "CONFIG_USB_DFU_REVISION=0x0320" > $CFGTMP
	;;

	gta02v3_config)
	echo "#define CONFIG_ARCH_GTA02_v3" > $CFGINC
	echo "#define CONFIG_GTA02_REVISION 3" >> $CFGINC
	echo "CONFIG_USB_DFU_REVISION=0x0330" > $CFGTMP
	;;

	gta02v4_config)
	echo "#define CONFIG_ARCH_GTA02_v4" > $CFGINC
	echo "#define CONFIG_GTA02_REVISION 4" >> $CFGINC
	echo "CONFIG_USB_DFU_REVISION=0x0340" > $CFGTMP
	;;

	gta02v5_config|gta02v6_config)
	echo "#define CONFIG_ARCH_GTA02_v5" > $CFGINC
	echo "#define CONFIG_GTA02_REVISION 5" >> $CFGINC
	echo "CONFIG_USB_DFU_REVISION=0x0350" > $CFGTMP
	;;

	*)
	echo "$0:: Unrecognised config - using GTA02v5 config"
	echo "#define CONFIG_ARCH_GTA02_v5" > $CFGINC
	echo "#define CONFIG_GTA02_REVISION 5" >> $CFGINC
	echo "CONFIG_USB_DFU_REVISION=0x0350" > $CFGTMP
	;;

	esac

fi
sed 's/^/#define /;s/=/ /' <$CFGTMP >>$CFGINC
# ---------------------------------------------------------
# Complete the configuration
# ---------------------------------------------------------
$MKCONFIG -a neo1973_gta02 arm arm920t gta02 neo1973 s3c24x0
