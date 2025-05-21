#!/bin/bash

script_dir=$(dirname "$(realpath "$0")")

# Read Python version required by CrowNet from Makefile
python_from_makefile=$(grep -oP '(?<=PYTHON := python).*' $script_dir/../Makefile)

# Read currently used pyhton version
python3_version=$(python3 --version 2>&1 | awk '{print $2}')
# remove patch from version string
python3_version=$(echo "$python3_version" | cut -d '.' -f 1-2)

echo "Python3 version required by CrowNet (see Makefile): $python_from_makefile"
echo "Python3 version used on this system: $python3_version"

# Compare and Warn
if [[ "$python_from_makefile" != "$python3_version" ]]; then
  echo "Error: Python version required by Makefile ($python_from_makefile) does not match Python version on this system ($python3_version)."
  exit -1
fi