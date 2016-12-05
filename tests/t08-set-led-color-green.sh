#!/bin/sh
#
# t08-set-led-color-green.sh
#

arg='green'

../client/ledctlc client$$ set-led-color $arg
rc=$?

exit $rc
