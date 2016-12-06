#!/bin/sh
#
# Тестирование сервера и клиента ledctl.
#

LEDCTL_DIR=$(pwd)/tmp;		export LEDCTL_DIR
mkdir $LEDCTL_DIR || exit 1

tests=$(cat <<-EOF
	t01-set-led-state-on.sh		set-led-state on
	t02-get-led-state-on.sh		get-led-state == on ?
	t03-set-led-state-off.sh	set-led-state off
	t04-get-led-state-off.sh	get-led-state == off ?
	t05-set-led-state-bad.sh	set-led-state bad
	t06-set-led-color-red.sh	set-led-color red
	t07-get-led-color-red.sh	get-led-color == red ?
	t08-set-led-color-green.sh	set-led-color green
	t09-get-led-color-green.sh	get-led-color == green ?
	t10-set-led-color-blue.sh	set-led-color blue
	t11-get-led-color-blue.sh	get-led-color == blue ?
	t12-set-led-color-bad.sh	set-led-color bad
	t13-led-rate.sh			led-rate
	t14-set-led-rate-bad.sh		set-led-rate bad
	t15-invalid-cmd.sh		invalid command
	t16-parallel-clients.sh		parallel clients
EOF
)

trap quit 1 2 3 15

quit()
{
	[ ! -z "$ledctld_pid" ] && kill -HUP $ledctld_pid
	rm -rf $LEDCTL_DIR
	exit
}

[ -x ../server/ledctld ] && {
	../server/ledctld &
	sleep 1
	ledctld_pid=$(cat $LEDCTL_DIR/ledctld.pid)

	echo "$tests" |
	{
		test_n=0
		test_ok=0
		test_fail=0

		while IFS='	' read test desc
		do
			./$test
			ret=$?
			if [ $ret -eq 0 ]
			then
				test_ok=$((test_ok + 1))
				echo -n '[OK] - '
			else
				test_fail=$((test_fail + 1))
				echo -n '[FAIL] - '
			fi
			echo "$test: " $desc

			test_n=$((test_n + 1))
		done

		echo
		echo "Total: $test_n," "ok: $test_ok", "fail: $test_fail"
		echo
	}
}

quit
