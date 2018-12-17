parallel --noswap --joblog joblog.txt --bar -j $2 './rl "environment_setting{1}.txt" "best_individual{1}.dot" "dna_best_individaul{1}" --neat_parameters {2} > "score{1}.txt"' ::: `seq 1 $1` ::: $3
