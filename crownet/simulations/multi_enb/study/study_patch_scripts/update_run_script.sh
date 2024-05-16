#!/bin/bash

echo "backup run_scrip.py"
find . -name run_script.py -exec cp -v {} {}.bak2 \;

echo "update run_script.py"
find . -name run_script.py -exec cp ../additional_rover_files/run_script.py {} \;

echo "done."

