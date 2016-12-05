#!/bin/sh
#
# t13-led-rate.sh
#

rate=0
rc=0

while [ $rate -le 5 ]
do
	../client/ledctlc client$$ set-led-rate $rate
	rc=$((rc + $?))

	answer=$(../client/ledctlc client$$ get-led-rate)
	if [ ! -z "$answer" -a $answer -eq $rate ]
	then
		:
	else
		rc=$((rc + 1))
	fi

	rate=$((rate + 1))
done

exit $rc
