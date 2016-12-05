#!/bin/sh
#
# t11-get-led-color-blue.sh
#

arg='blue'

answer=$(../client/ledctlc client$$ get-led-color) || exit 1
if [ ! -z "$answer" -a $answer = $arg ]
then
	rc=0
else
	rc=1
fi

exit $rc
