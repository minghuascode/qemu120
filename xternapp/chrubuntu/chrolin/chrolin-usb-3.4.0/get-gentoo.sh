#!/bin/sh
# ---------------------------------------------
#   Chromebook GNU/Linux USB Gentoo downloader
# ---------------------------------------------
#   http://www.chromebook-linux.com
#
#   by: Jordi <jordi@chromebook-linux.com>
#
# This script downloads from Internet: Gentoo rootfs (for Chromebooks)
#

VERSION=`cat VERSION`
DEST_PATH="builds"

echo "\n-- Chromebook Linux USB $VERSION --\n\thttp://www.chromebook-linux.com\n"

if [ ! -d $DEST_PATH ];
then
    mkdir -p $DEST_PATH || exit $?
fi

# Download and copy gentoo root filesystem, keep track of successful parts so we can resume

for one in a b
do
    for two in a b c d e f g h i j k l m n o p q r s t u v w x y z
    do
	FILENAME="gentoo_rootfs.bin$one$two.bz2"
	if [ ! -f $DEST_PATH/gentoo-$FILENAME-finished.txt ]
	then
	    if [ -f $DEST_PATH/$FILENAME ]
 	    then
                rm $DEST_PATH/$FILENAME
            fi

	    echo "- Downloading $FILENAME\n"
	    wget -P $DEST_PATH http://files.chromebook-linux.com/gentoo/$FILENAME
	    touch $DEST_PATH/gentoo-$FILENAME-finished.txt
	fi
    done
done

cd $DEST_PATH
echo "- Unzipping parts..."
bunzip2 gentoo_rootfs.bin*.bz2
echo "- Merging parts..."
cat gentoo_rootfs.bin* > gentoo_rootfs.bin
echo "- Cleaning temporary files..."
rm gentoo-gentoo_rootfs.bin*.bz2-finished.txt
rm gentoo_rootfs.bina*
rm gentoo_rootfs.binb*
cd -

echo "\n\n- Done! Type: sudo sh chrolin-usb.sh -u <your usb path> -i builds/gentoo_rootfs.bin to install Gentoo in your USB.\n\n"
