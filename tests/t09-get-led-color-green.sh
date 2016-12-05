#!/bin/sh
#
# t09-get-led-color-green.sh
#

arg='green'

answer=$(../client/ledctlc client$$ get-led-color) || exit 1
if [ ! -z "$answer" -a $answer = $arg ]
then
	rc=0
else
	rc=1
fi

exit $rc
