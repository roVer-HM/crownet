# Makefile for the roVer simulation models
#
# This top-level Makefile builds all components required for running coupled pedestrian mobility 
# and communication simulations. Therefore, it invokes make in all relevant subfolders.
#
# make makefiles     creates all the Makefiles in the subfolders
# make               builds all models  (optionally specify MODE=debug or MODE=release)
# make clean         cleans all models  (optionally specify MODE=debug or MODE=release)

mod_rover      := rover
mod_inet       := inet4
mod_simulte    := simulte
mod_veins      := veins
mod_veins_inet := veins/subprojects/veins_inet

# In order to simplify handling the dependencies between the models, 
# the models are ordered in three different levels:
# Level 1: These models do only depend on the OMNeT++ core libraries.
# Level 2: These models depend on Level 1 models, e.g. inet.
# Level 3: These models depend on Level 2 models, e.g. simuLTE.
models_l1 := $(mod_inet) $(mod_veins)
models_l2 := $(mod_simulte) $(mod_veins_inet) 
models_l3 := $(mod_rover) 
models := $(models_l3) $(models_l2) $(models_l1)

NUM_CPUS := $(shell grep -c ^processor /proc/cpuinfo)

target clean : TARGET = clean
target makefiles : TARGET = makefiles

.PHONY: all $(models)

all: $(models_l3) $(models_l2) $(models_l1)

$(models_l3): $(models_l2)

$(models_l2): $(models_l1)

clean: all

# - for all standard models we recursively call make with the makefiles target
# - for veins, we need to call ./configure
makefiles: makefiles_std configure_veins configure_veins_inet

makefiles_std:
	$(MAKE) --directory=$(mod_inet) makefiles
	$(MAKE) --directory=$(mod_simulte) makefiles
	$(MAKE) --directory=$(mod_rover) makefiles

configure_veins:
	cd $(mod_veins); ./configure

configure_veins_inet:
	cd $(mod_veins_inet); ./configure --with-inet=../../../$(mod_inet)

$(models_l3) $(models_l2) $(models_l1):
	$(MAKE) -j$$(($(NUM_CPUS)*2)) --directory=$@ $(TARGET)


