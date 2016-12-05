#!/bin/sh
#
# t15-invalid-cmd.sh
#

../client/ledctlc client$$ cmd invalid
rc=$?

if [ $rc -eq 0 ]
then
	rc=1
else
	rc=0
fi

exit $rc
