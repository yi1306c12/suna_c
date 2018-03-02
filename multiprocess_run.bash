SCRIPT=$1
HOWMANY=$2
MULTI=$3

seq 1 $HOWMANY | xargs -P $MULTI -n 1 -IXXX bash run_script.bash XXX "$SCRIPT"