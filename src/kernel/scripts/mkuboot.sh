#!/bin/bash

#
# Build U-Boot image when `mkimage' tool is available.
#

MKIMAGENAME=mkimage
# Find out if a custom tool name is provided
if [ "$1" = "-cbin" ]; then
	shift
	if [ "$1" = "" ]; then
		echo "Error: missing arguments"
		exit 1;
	fi
	MKIMAGENAME=$1
	shift
fi

MKIMAGE=$(type -path "${MKIMAGENAME}")

if [ -z "${MKIMAGE}" ]; then
	MKIMAGE=$(type -path mkimage)
	if [ -z "${MKIMAGE}" ]; then
		# Doesn't exist
		echo "'${MKIMAGENAME}' command not found - U-Boot images will not be built" >&2
		exit 0;
	fi
fi

# Call "mkimage" to create U-Boot image
${MKIMAGE} "$@"
