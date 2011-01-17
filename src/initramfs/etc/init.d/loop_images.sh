#!/bin/sh
#       
# @brief Display forever up to 3 images using <program> waiting <sleep> 
#		seconds between each one.
#       Usage: images_loop.sh <sleep> <program> <image0> <image1> [image2]
#
# Copyright (c) 2010 Duolabs Spa
#
# Author: Pedro Aguilar (pedro@duolabs.com)
#

if [ "$1" == "" -o "$2" == "" -o "$3" == "" -o "$4" == "" ]; then
	exit 1
fi

SLEEP=$1
PROG=$2
IMG_A=$3
IMG_B=$4
IMG_C=$5

while [ "1" ]; do
	$PROG /etc/images/$IMG_A
	sleep $SLEEP
	$PROG /etc/images/$IMG_B
	sleep $SLEEP
	if [ "$IMG_C" != "" ]; then
		$PROG /etc/images/$IMG_C
		sleep $SLEEP
	fi
done


