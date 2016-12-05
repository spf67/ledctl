#!/bin/sh
#
# t12-set-led-color-bad.sh
#

arg='yellow'

../client/ledctlc client$$ set-led-color $arg
rc=$?

if [ $rc -eq 0 ]
then
	rc=1
else
	rc=0
fi

exit $rc
