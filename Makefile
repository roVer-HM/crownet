# Makefile for the roVer simulation models
#
# This top-level Makefile builds all components required for running coupled pedestrian mobility
# and communication simulations. Therefore, it invokes make in all relevant subfolders.
#
# make makefiles     creates all the Makefiles in the subfolders
# make               builds all models  (optionally specify MODE=debug or MODE=release)
# make clean         cleans all models  (optionally specify MODE=debug or MODE=release)

mod_crownet    := crownet
mod_inet       := inet4
mod_simulte    := simulte
mod_veins      := veins
mod_veins_inet := veins/subprojects/veins_inet
mod_artery     := artery
mod_roveranalyzer   := analysis/roveranalyzer
mod_flowcontrol		:= flowcontrol
mod_suqc		:= analysis/suq-controller

# In order to simplify handling the dependencies between the models,
# the models are ordered in three different levels:
# Level 1: These models do only depend on the OMNeT++ core libraries.
# Level 2: These models depend on Level 1 models, e.g. inet.
# Level 3: These models depend on Level 2 models, e.g. simuLTE.
models_l1 := $(mod_inet) $(mod_veins)
models_l2 := $(mod_simulte) $(mod_veins_inet)
models_l3 := $(mod_crownet)
models := $(models_l3) $(mod_artery) $(models_l2) $(models_l1)

NUM_CPUS := $(shell grep -c ^processor /proc/cpuinfo)
PYTHON := python3.8

# check if omnetpp is found
ifeq (, $(shell which opp_configfilepath))
 $(error opp_configfilepath not found. In order to run the command within the roVer docker container, try "omnetpp exec make")
endif

# python build variables
roveranalyzer_sdist := $(shell cd $(mod_roveranalyzer); $(PYTHON) setup.py --fullname).tar.gz
flowcontrol_sdist := $(shell cd $(mod_flowcontrol); $(PYTHON) setup.py --fullname).tar.gz
suqc_sdist := $(shell cd $(mod_suqc); $(PYTHON) setup.py --fullname).tar.gz
python_dists := $(addprefix out/, $(roveranalyzer_sdist) $(flowcontrol_sdist) $(suqc_sdist))
venv_user := crownet_user
venv_dev  := crownet_dev
python_venvs := $(addprefix out/, $(venv_user) $(venv_dev))

# In case the user did not specify a MODE, use the default mode which is "debug".
MODE ?= debug

target clean : TARGET = clean
target cleanall : TARGET = cleanall
target makefiles : TARGET = makefiles

.PHONY: all $(models)

all: $(models_l3) $(mod_artery) $(models_l2) $(models_l1)

$(models_l3): $(models_l2) $(mod_artery)

$(models_l2): $(models_l1)

clean: all

cleanall: all

oppfeatures:
	cd $(mod_inet); opp_featuretool enable VisualizationOsg

# - for all standard models we recursively call make with the makefiles target
# - for veins, we need to call ./configure
makefiles: makefiles_std configure_veins configure_veins_inet

makefiles_std: oppfeatures
	$(MAKE) --directory=$(mod_inet) makefiles
	$(MAKE) --directory=$(mod_simulte) makefiles
	$(MAKE) --directory=$(mod_crownet) makefiles

configure_veins:
	cd $(mod_veins); ./configure

configure_veins_inet:
	cd $(mod_veins_inet); ./configure --with-inet=../../../$(mod_inet)

$(models_l3) $(models_l2) $(models_l1):
	$(MAKE) -j$$(($(NUM_CPUS)*2)) --directory=$@ $(TARGET) MODE=$(MODE)

$(mod_artery): $(models_l2) $(models_l1)
	$(MAKE) -j$$(($(NUM_CPUS)*2)) --directory=$@ $(TARGET) MODE=$(MODE)

#Python analysis setup##########################################################

analysis-all: analysis-build venv-build

analysis-build: $(python_dists)

analysis-clean:
	rm -f $(python_dists)

venv-build: $(python_venvs)

out:
	mkdir -v out/

#-create source distributions of all python packages---------------------------

out/$(roveranalyzer_sdist): | out
	cd $(mod_roveranalyzer) && \
		$(PYTHON) setup.py sdist
	cp -v $(mod_roveranalyzer)/dist/$(roveranalyzer_sdist) out

out/$(flowcontrol_sdist): | out
	cd $(mod_flowcontrol) && \
   	   $(PYTHON) setup.py sdist
	cp -v $(mod_flowcontrol)/dist/$(flowcontrol_sdist) out

out/$(suqc_sdist): | out
	cd $(mod_suqc) && \
	   $(PYTHON) setup.py sdist
	cp -v $(mod_suqc)/dist/$(suqc_sdist) out

#-build virtual enviroments----------------------------------------------------

out/$(venv_user): analysis-build | out
	cd out && \
		$(PYTHON) -m  venv $(venv_user) && \
		. ./$(venv_user)/bin/activate && \
		pip3 install --upgrade pip && \
		pip3 install wheel && \
		pip3 install $(flowcontrol_sdist) && \
		pip3 install $(roveranalyzer_sdist) && \
		pip3 install $(suqc_sdist)

out/$(venv_dev): analysis-build | out
	cd out && \
		$(PYTHON) -m  venv $(venv_dev) && \
		. ./$(venv_dev)/bin/activate && \
		pip3 install --upgrade pip && \
		pip3 install wheel && \
		pip3 install -r ../$(mod_flowcontrol)/requirements.txt && \
		pip3 install -r ../$(mod_roveranalyzer)/requirements-dev.txt && \
		pip3 install -r ../$(mod_suqc)/requirements.txt

#------------------------------------------------------------------------------

