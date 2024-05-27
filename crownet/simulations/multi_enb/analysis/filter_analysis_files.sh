#!/bin/bash 


GREP_STR="position.h5|\
density_map_count_error.h5|\
enb_rb.h5|\
node_tx_data.h5|\
node_rx_data.h5|\
entropy_map_size.csv|\
entropy_map_size_global.csv|\
density_map_size.csv|\
density_map_size_global.csv|\
entropy_map_measurements_age_over_distance.h5|\
entropy_map_size_and_age_over_distance.h5|\
entropy_map_size_global_by_rsd.csv|\
density_map_size_global_by_rsd|\
DENSITY.cfg|\
ENTROPY.cfg"

GREP_STR="entropy_map_size.csv|\
entropy_map_size_global.csv|\
density_map_size.csv|\
density_map_size_global.csv"

_tmp=$(mktemp --suffix "seed_$2")


find $1 -path "*Sample*$2_0*" -type f -exec ls -l {} \; | # create file list
	tr -s ' ' | # collapse multiple strings
	awk -F ' '  '{print $9 " "  $5;}' | # select name and size columns 
	grep -E "${GREP_STR}"  > $_tmp  # select copied files and save to file 


cat $_tmp | awk '{print $1}'

if [ -t 1 ] ; then
    cat $_tmp | awk '{sum+=$2}END{print NR" files with total size of " sum/1000000000" GB";}'
fi

rm $_tmp

