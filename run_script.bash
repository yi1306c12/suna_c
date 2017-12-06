ID=$1
echo $ID $BASHPID

logname="log"$ID".txt"
timelogname="timelog"$ID".txt"
python3 -m cProfile -s time -o $timelogname main_gym.py > $logname

dot -T jpg best_individual.dot -o "best"$ID".jpg"
mv best_individual.dot "best_individual"$ID".dot"
mv dna_best_individual "dna_best_individual"$ID
