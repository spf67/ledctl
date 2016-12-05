#!/bin/sh
#
# t01-set-led-state-on.sh
#

arg='on'

../client/ledctlc client$$ set-led-state $arg
rc=$?

exit $rc
