# Eclipse IDE for OMNeT++

After executing the `update_container.sh` script for the omnetpp-ide container, 
this directory contains locally cached versions of the the base IDE (within ide.tar.gz)
that is required in order to run the graphical user interface of OMNeT++.
It is not included in the OMNeT++ repository but delivered as part of the
OMNeT++ source tar archive on the website. 

If you compile OMNeT++ based on the sources in the roVer gitlab repository, the
files in the ide.tar.gz archive need to be extracted to `./ide` in the omnetpp
directory. This is done automatically by the `update_container.sh` script.

The files are downloaded and cached locally and not part of the repo. 
See `../update_container.sh` for currently used version.
