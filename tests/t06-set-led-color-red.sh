#!/bin/sh
#
# t06-set-led-color-red.sh
#

arg='red'

../client/ledctlc client$$ set-led-color $arg
rc=$?

exit $rc
