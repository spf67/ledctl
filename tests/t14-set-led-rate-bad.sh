#!/bin/sh
#
# t14-set-led-rate-bad.sh
#

arg='777'

../client/ledctlc client$$ set-led-rate $arg
rc=$?

if [ $rc -eq 0 ]
then
	rc=1
else
	rc=0
fi

exit $rc
