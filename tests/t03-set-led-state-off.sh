#!/bin/sh
#
# t03-set-led-state-off.sh
#

arg='off'

../client/ledctlc client$$ set-led-state $arg
rc=$?

exit $rc
