parallel --noswap --joblog joblog.txt --bar -j $2 './rl "environment_setting{}.txt" "best_individual{}.dot" "dna_best_individaul{}" > "score{}.txt"' ::: `seq 1 $1`
