#!/bin/sh
#
# t16-parallel-clients.sh
#

rc=0
n=10

sh ./t06-set-led-color-red.sh
rc=$((rc + $?))

for i in $(seq $n)
do
	( sleep 1; sh ./t07-get-led-color-red.sh; ) &
	pids="$pids $!"
done

for pid in $pids
do
	wait $pid
	rc=$((rc + $?))
done

exit $rc
