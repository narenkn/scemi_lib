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

\rm simv_*.daidir/.vcs.timestamp

\rm -rf csrc
vcs -sverilog -o simv_dut +incdir+$REPO_PATH/scemi $REPO_PATH/scemi/scemi_pipes_dut.sv $testdir/*dut.sv -CFLAGS "-g -I$REPO_PATH/scemi -DSVC_DUT" $REPO_PATH/scemi/scemi_pipes.cc -debug_all

\rm -rf csrc
vcs -sverilog -o simv_tb +incdir+$REPO_PATH/scemi $REPO_PATH/scemi/scemi_pipes_tb.sv $testdir/*tb.sv -CFLAGS "-g -I$REPO_PATH/scemi" $REPO_PATH/scemi/scemi_pipes.cc -debug_all

mkfifo svc_dut.f svc_tb.f

if [ ! -f ./simv_dut -o ! -f ./simv_tb ] ; then
	exit;
fi

./simv_tb > tb.log 2>&1 &
./simv_dut > dut.log 2>&1 &

wait

\rm -f svc_dut.f svc_tb.f

