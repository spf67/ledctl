#!/bin/sh
#
# t04-set-led-state-bad.sh
#

arg='bad'

../client/ledctlc client$$ set-led-state $arg
rc=$?

if [ $rc -eq 0 ]
then
	rc=1
else
	rc=0
fi

exit $rc
