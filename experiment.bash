seq 1 $1 | xargs -n1 -P$2 -I{} sh -c './rl "environment_setting$1.txt" "best_individual$1.dot" "dna_best_individual$1" > "score$1.txt"' -- {}
