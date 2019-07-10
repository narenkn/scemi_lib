#! /bin/env bash

if [ "aa" = "aa${REPO_PATH}" ] ; then
	echo "do bootenv and try again";
	exit 1;
fi

## Loop Init and Run
args=("$@");
((n=0));
((skip=0));
for WORD in "$@" ; do
	((n++));
	if [ "0" -eq $skip ] ; then
		case $WORD in
		-testdir)
			testdir="${args[((n))]}";
			((skip=1));
			;;
		*) ## It is a testcase...
			all_tests=("${all_tests[@]}" "${WORD}");
			;;
		esac
	else
		((skip=0));
	fi;
done

\rm -f simv_*.daidir/.vcs.timestamp svc_dut.f svc_tb.f

\rm -rf csrc
vcs -full64 -sverilog -o simv_dut +incdir+../scemi ../scemi/scemi_pipes_dut.sv dut.sv -CFLAGS "-g -I../scemi -DSVC_DUT" ../scemi/scemi_pipes.cc -debug_access+all

\rm -rf csrc
vcs -full64 -sverilog -o simv_tb +incdir+../scemi ../scemi/scemi_pipes_tb.sv tb.sv -CFLAGS "-g -I../scemi" ../scemi/scemi_pipes.cc -debug_access+all

mkfifo svc_dut.f svc_tb.f

if [ ! -f ./simv_dut -o ! -f ./simv_tb ] ; then
	exit;
fi

./simv_dut > dut.log 2>&1 &
./simv_tb > tb.log 2>&1 &

wait

\rm -f svc_dut.f svc_tb.f

