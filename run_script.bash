ID=$1
SCRIPT=$2
echo $ID $BASHPID

logname="log"$ID".txt"
$SCRIPT > $logname

dot -T jpg best_individual.dot -o "best"$ID".jpg"
mv best_individual.dot "best_individual"$ID".dot"
mv dna_best_individual "dna_best_individual"$ID
