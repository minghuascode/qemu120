#!/bin/sh
# --------------------------------------------------
#   Chromebook GNU/Linux USB Debian 6.0.3 downloader
# --------------------------------------------------
#   http://www.chromebook-linux.com
#
#   by: Jordi <jordi@chromebook-linux.com>
#
# This script downloads from Internet: Debian 6.0.3 rootfs (for Chromebooks)
#

VERSION=`cat VERSION`
DEST_PATH="builds"

echo "\n-- Chromebook Linux USB $VERSION --\n\thttp://www.chromebook-linux.com\n"

if [ ! -d $DEST_PATH ];
then
    mkdir -p $DEST_PATH || exit $?
fi

# Download and copy debian root filesystem, keep track of successful parts so we can resume

for one in a b
do
    for two in a b c d e f g h i j k l m n o p q r s t u v w x y z
    do
	FILENAME="debian_rootfs.bin$one$two.bz2"
	if [ ! -f $DEST_PATH/debian-$FILENAME-finished.txt ]
	then
	    if [ -f $DEST_PATH/$FILENAME ]
 	    then
                rm $DEST_PATH/$FILENAME
            fi

	    echo "- Downloading $FILENAME\n"
	    wget -P $DEST_PATH http://files.chromebook-linux.com/debian-6.0.3/$FILENAME
	    touch $DEST_PATH/debian-$FILENAME-finished.txt
	fi
    done
done

cd $DEST_PATH
echo "- Unzipping parts..."
bunzip2 debian_rootfs.bin*.bz2
echo "- Merging parts..."
cat debian_rootfs.bin* > debian_rootfs-6.0.3.bin
echo "- Cleaning temporary files..."
rm debian-debian_rootfs.bin*.bz2-finished.txt
rm debian_rootfs.bina*
rm debian_rootfs.binb*
cd -

echo "\n\n- Done! Type: sudo sh chrolin-usb.sh -u <your usb path> -i builds/debian_rootfs-6.0.3.bin to install Debian 6.0.3 in your USB.\n\n"
