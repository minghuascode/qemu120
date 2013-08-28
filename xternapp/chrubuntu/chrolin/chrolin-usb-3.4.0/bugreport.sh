#!/bin/sh
# -------------------------------------
#   Chromebook GNU/Linux USB generator 
# -------------------------------------
#   http://www.chromebook-linux.com
#
#   by: Jordi <jordi@chromebook-linux.com>
#
# This script sends a bug report to developer with a remote PHP script
# 
# Anyway, you can send manually a bug report via email to: jordi@chromebook-linux.com
#

CHROLIN_VERSION=`cat VERSION` || exit $?
CHROLIN_BUGREPORT=""
CHROLIN_SENDBUG=""
CHROLIN_YOUREMAIL=""

echo "\n-- Chromebook Linux USB $CHROLIN_VERSION --\n\thttp://www.chromebook-linux.com\n"

read -p "- [Bug report] Describe a bug you have found on chrolin-usb: " CHROLIN_BUGREPORT

if [ -n "$CHROLIN_BUGREPORT" ];
then
    read -p "- [Optional] Enter your email address if you want a response related to this bug: " CHROLIN_YOUREMAIL
    read -p "- [Send bug report] Send this bug report now? [y/n]: " CHROLIN_SENDBUG
    case $CHROLIN_SENDBUG in
      [Yy])
	    echo "- Sending bug report, please wait..."
#
# Replace spaces and ampersand symbol to %20 as a quick fix to make a HTTP GET
#
	    CHROLIN_BUGREPORT=`echo $CHROLIN_BUGREPORT | sed 's/[ \&]/%20/g'`
	    CHROLIN_POSTDATA="contact=$CHROLIN_YOUREMAIL&bug=$CHROLIN_BUGREPORT"

	    curl --user-agent "Chrolin/$CHROLIN_VERSION" -d "$CHROLIN_POSTDATA" http://files.chromebook-linux.com/chrolin-usb/bugreport.php || exit $?
#
# Next, we receive a response from [files.chromebook-linux.com] server if email to developer was sent successfully or not.
# So, our job here is finished
#
	    ;;
	 *) ;;
    esac
fi
