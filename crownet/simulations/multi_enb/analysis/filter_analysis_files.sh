#!/bin/bash 


GREP_STR="position.h5|\
density_map_count_error.h5|\
enb_rb.h5|\
node_tx_data.h5|\
node_rx_data.h5|\
entropy_map_size.csv|\
entropy_map_measurements_age_over_distance.h5|\
entropy_map_size_and_age_over_distance.h5"

find $1 -path "*Sample*$2*" -type f -exec ls -l {} \; | # create file list
	tr -s ' ' | # collapse multiple strings
	awk -F ' '  '{print $9 " "  $5;}' | # select name and size columns 
	grep -E "${GREP_STR}"  > "analysis_file_list_seed_$2.txt"  # select copied files and save to file 

cat "analysis_file_list_seed_$2.txt" | awk '{sum+=$2}END{print NR" files with total size of " sum/1000000000" GB";}'

cat "analysis_file_list_seed_$2.txt" | awk '{print $1}' > "tmp_analysis_file_list_seed_$2.txt"

mv  "tmp_analysis_file_list_seed_$2.txt" "analysis_file_list_seed_$2.txt"
