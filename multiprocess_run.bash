HOWMANY=$1
MULTI=$2

seq 1 $HOWMANY | xargs -P $MULTI -n 1 bash run_script.bash

curdir=$(dirname $(readlink -f $0))
basename1=$(basename $(dirname $curdir))
basename2=$(basename $curdir)
csvname="${basename1}_${basename2}.csv"
echo $csvname

bash ../../serial_number.bash log $1 .txt | xargs -n 1 bash ../../add_csv_column.bash > $csvname
