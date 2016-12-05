#!/bin/sh
#
# t04-get-led-state-off.sh
#

arg='off'

answer=$(../client/ledctlc client$$ get-led-state) || exit 1
if [ ! -z "$answer" -a $answer = $arg ]
then
	rc=0
else
	rc=1
fi

exit $rc
