#!/bin/sh
# ----------------------------------------------
#   Chromebook GNU/Linux Ubuntu 11.04 downloader
# ----------------------------------------------
#   http://www.chromebook-linux.com
#
#   by: Jordi <jordi@chromebook-linux.com>
#
# This script downloads from Internet: Ubuntu 11.04 rootfs (for Chromebooks)
#
# Note: the Ubuntu 11.04 image was created by Jay Lee: http://chromeos-cr48.blogspot.com/2011/04/ubuntu-1104-for-cr-48-is-ready.html
#

VERSION='cat VERSION'
DEST_PATH="builds"

echo "\n-- Chromebook Linux USB $VERSION --\n\thttp://www.chromebook-linux.com\n"

if [ ! -d $DEST_PATH ];
then
    mkdir -p $DEST_PATH || exit $?
fi

# Download and copy Ubuntu 11.04 root filesystem, keep track of successful parts so we can resume

for one in a b
do
    for two in a b c d e f g h i j k l m n o p q r s t u v w x y z
    do
	FILENAME="ubuntu.bin$one$two.bz2"
	if [ ! -f $DEST_PATH/ubuntu-$FILENAME-finished.txt ]
	then
            if [ -f $DEST_PATH/$FILENAME ]
            then
                rm $DEST_PATH/$FILENAME
            fi

	    echo "- Downloading $FILENAME\n"
	    wget -P $DEST_PATH http://cr-48-ubuntu.googlecode.com/files/$FILENAME
	    touch $DEST_PATH/ubuntu-$FILENAME-finished.txt
	fi
    done
done

cd $DEST_PATH
echo "- Merging parts..."
cat ubuntu.bin* > ubuntu_rootfs-11.04.bin.bz2
echo "- Unzipping parts..."
bunzip2 ubuntu_rootfs-11.04.bin.bz2
echo "- Cleaning temporary files..."
rm ubuntu-ubuntu.bin*.bz2-finished.txt
rm ubuntu.bina*
rm ubuntu.binb*
cd -

echo "\n\n- Done! Type: sudo sh chrolin-usb.sh -u <your usb path> -i builds/ubuntu_rootfs-11.04.bin to install Ubuntu 11.04 in your USB.\n\n"
