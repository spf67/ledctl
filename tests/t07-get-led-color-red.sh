#!/bin/sh
#
# t07-get-led-color-red.sh
#

arg='red'

answer=$(../client/ledctlc client$$ get-led-color) || exit 1
if [ ! -z "$answer" -a $answer = $arg ]
then
	rc=0
else
	rc=1
fi

exit $rc
