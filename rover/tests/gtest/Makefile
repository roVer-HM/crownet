GOOGLE_TEST_LIB = gtest
GOOGLE_TEST_INCLUDE = /usr/local/include

# Projects
ROVER_PROJ=../../../rover
ARTERY_PROJ=../../../artery
INET_PROJ=../../../inet4
LTE_PROJ=../../../simulte
VEINS_PROJ=../../../veins
VEINS_INET_PROJ=../../../veins/subprojects/veins_inet


G++ = g++
G++_FLAGS = -c -Wall -I $(GOOGLE_TEST_INCLUDE) 
G++_FLAGS_DBG = -g -O0 -DDEBUG
LD_FLAGS = \
	-L /usr/local/lib \
	-l $(GOOGLE_TEST_LIB) \
	-l pthread $(LDFLAG_LIBPATH)$(ARTERY_PROJ)/build/Debug/lib $(LDFLAG_LIBPATH)$(ARTERY_PROJ)/build/Release/lib $(LDFLAG_LIBPATH)$(ARTERY_PROJ)/extern/vanetza/build/Debug/lib $(LDFLAG_LIBPATH)$(ARTERY_PROJ)/extern/vanetza/build/Release/lib $(LDFLAG_LIBPATH)$(INET_PROJ)/src $(LDFLAG_LIBPATH)$(LTE_PROJ)/src $(LDFLAG_LIBPATH)$(VEINS_PROJ)/src $(LDFLAG_LIBPATH)$(VEINS_PROJ)/subprojects/veins_inet/src  -lboost_filesystem -lartery_core$(D) -lartery_envmod$(D) -lartery_police$(D) -lartery_storyboard$(D) -ltraci$(D) -lvanetza_access$(D) -lvanetza_asn1$(D) -lvanetza_btp$(D) -lvanetza_common$(D) -lvanetza_dcc$(D) -lvanetza_facilities$(D) -lvanetza_geonet$(D) -lvanetza_gnss$(D) -lvanetza_net$(D) -lvanetza_security$(D) -lINET$(D) -llte$(D) -lveins$(D) -lveins_inet$(D)
  
# C++ include paths (with -I)
INCLUDE_PATH = \
  -I $(ROVER_PROJ)/src


ifeq ("$(MODE)","debug")
MODE_PATH = Debug
G++_FLAGS += $(G++_FLAGS_DBG)
else
MODE_PATH = Release
endif


OBJECTS = main_test.o grid_test.o
TARGET = rover_uTests



#------------------------------------------------------------------------------

# Pull in OMNeT++ configuration (Makefile.inc)

ifneq ("$(OMNETPP_CONFIGFILE)","")
CONFIGFILE = $(OMNETPP_CONFIGFILE)
else
ifneq ("$(OMNETPP_ROOT)","")
CONFIGFILE = $(OMNETPP_ROOT)/Makefile.inc
else
CONFIGFILE = $(shell opp_configfilepath)
endif
endif

ifeq ("$(wildcard $(CONFIGFILE))","")
$(error Config file '$(CONFIGFILE)' does not exist -- add the OMNeT++ bin directory to the path so that opp_configfilepath can be found, or set the OMNETPP_CONFIGFILE variable to point to Makefile.inc)
endif

include $(CONFIGFILE)

# Simulation kernel and user interface libraries
OMNETPP_LIBS = -L/opt/omnetpp/omnetpp-5.6/lib -loppsim$D -loppenvir$D $(KERNEL_LIBS) $(SYS_LIBS)
ifneq ($(TOOLCHAIN_NAME),clangc2)
LD_FLAGS += -Wl,-rpath,/opt/omnetpp/omnetpp-5.6/lib \
	-Wl,-rpath,$(abspath $(ARTERY_PROJ)/build/$(MODE_PATH)/lib) \
	-Wl,-rpath,$(abspath $(ARTERY_PROJ)/extern/vanetza/build/$(MODE_PATH)/lib) \
	-Wl,-rpath,$(abspath $(INET_PROJ)/src) \
	-Wl,-rpath,$(abspath $(LTE_PROJ)/src) \
	-Wl,-rpath,$(abspath $(VEINS_PROJ)/src) \
	-Wl,-rpath,$(abspath $(VEINS_PROJ)/subprojects/veins_inet/src)
endif

G++_FLAGS += $(IMPORT_DEFINES) -DINET_IMPORT -DVEINS_IMPORT -DVEINS_INET_IMPORT -DROVER_EXPORT $(INCLUDE_PATH) -I$(OMNETPP_INCL_DIR)
MSGCOPTS = $(INCLUDE_PATH)
SMCOPTS =

# we want to recompile everything if COPTS changes,
# so we store COPTS into $COPTS_FILE and have object
# files depend on it (except when "make depend" was called)
COPTS_FILE = $O/.last-copts
ifneq ("$(COPTS)","$(shell cat $(COPTS_FILE) 2>/dev/null || echo '')")
$(shell $(MKPATH) "$O" && echo "$(COPTS)" >$(COPTS_FILE))
endif

#------------------------------------------------------------------------------


all: $(TARGET)

$(TARGET): $(OBJECTS)
	@echo ">>>: $(CONFIGFILE)"
	g++ -o $(TARGET) $(OBJECTS) $(LD_FLAGS)  $(OMNETPP_LIBS)

%.o : %.cc
	$(G++) $(G++_FLAGS) $<

clean:
	rm -f $(TARGET) $(OBJECTS)
                    
.PHONY: all clean