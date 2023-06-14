
# Individually building the models
Instead of using the top-level Makefile, the models can also be built individually. Therefore, one must
first switch to the model subdirectory and than execute make locally.

*Example: Building the INET and Simu5G Frameworks*

Assuming that you have added the "omnetpp" script to your search path for executables and cloned the CrowNet project with all its submodules (see installation instructions above), you can build the INET and Simu5G models by the following commands:
```
cd inet
omnetpp exec make makefiles
omnetpp exec make -j4 MODE=release
omnetpp exec make -j4 MODE=debug
cd ..

cd simu5g
omnetpp exec make makefiles
omnetpp exec make -j4

```

*Example: Building the veins (vehicles in network simulation) projects*

The veins framework itself can be built similar to the previously mentioned components. However, it is recommended to create
the makefiles by calling the *configure* script:
```
cd veins
omnetpp exec ./configure
omnetpp exec make -j4
```

Veins includes of several subprojects - for CrowNet the veins_inet project is required. It needs to have access to the folder where INET 4 is located. The location is specified by the option `--with-inet=../../../inet4` of the *configure* script:

```
cd subprojects/veins_inet
omnetpp exec ./configure --with-inet=../../../inet4
omnetpp exec make -j4
cd ../../..
```
