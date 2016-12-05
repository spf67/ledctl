#!/bin/sh
#
# t10-set-led-color-blue.sh
#

arg='blue'

../client/ledctlc client$$ set-led-color $arg
rc=$?

exit $rc
