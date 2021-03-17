#!/bin/bash
python3 run_script.py --experiment-label "experiment-$(date +"%Y-%m-%d-%s")" --delete-existing-containers --create-vadere-container --with-control control.py --control-vadere-only
python3 run_script.py --experiment-label "experiment-$(date +"%Y-%m-%d-%s")" --delete-existing-containers --create-vadere-container
