#!/bin/bash


find . -name omnetpp.ini -exec cp -v {} {}.bak \;
echo "created backup of omnetpp.ini files"

echo "replace idStreamType..."
find . -name omnetpp.ini -exec sed --in-place -e  's/orderBySentLastAsc_DistAsc/orderByDistanceAscending/g' {} \;
find . -name omnetpp.ini -exec sed --in-place -e  's/orderBySentLastAsc_DistDesc/orderByDistanceDescending/g' {} \;

echo "done."
