#!/bin/bash
# Note: Requires python3.8
# apt install python3.8 python3.8-venv

BASEDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
VENV="$BASEDIR/venv"

# check if virtual environment needs to be created
if [ ! -d "$BASEDIR/venv" ]
then
    echo "venv does not exists - creating new virtual environment"
    python3.8 -m venv "$BASEDIR/venv"
fi

# Activate venv
source $VENV/bin/activate
if [ $? -ne 0 ]; then
    echo "venv activation failed"
    exit 1
fi

# install crownetanalyzer if not available
python3 -c "import crownetanalyzer"
if [ $? -ne 0 ]; then
    echo "crownetanalyzer is not available - installing module..."
    # currently, we use editable mode - remove -e if you do not need to edit the crownetanalyzer
    pip3 install --upgrade pip
    pip3 install -e "$BASEDIR/../../../analysis/crownetanalyzer"
fi


