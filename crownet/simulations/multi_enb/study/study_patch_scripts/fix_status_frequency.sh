#!/bin/bash

echo "set cmdenv-status-frequency to 600 seconds."

find . -name omnetpp.ini -exec sed --in-place=.bak2  -e 's/cmdenv-status-frequency = 10s/cmdenv-status-frequency = 600s/' {} \;

echo "done."
