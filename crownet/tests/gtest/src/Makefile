#
# OMNeT++/OMNEST Makefile for rTests
#
# This file was generated with the command:
#  opp_makemake -f --deep -o rTests -O out -KCROWNET_PORJ=../../../../crownet -KARTERY_PROJ=../../../../artery -KINET_PROJ=../../../../inet4 -KLTE_PROJ=../../../../simulte -KVEINS_PROJ=../../../../veins -KVEINS_INET_PROJ=../../../../veins/subprojects/veins_inet -DINET_IMPORT -DVEINS_IMPORT -DVEINS_INET_IMPORT -I. -I$(CROWNET_PORJ)/src -I$(ARTERY_PROJ)/src -I$(ARTERY_PROJ)/src/traci/sumo -I$(ARTERY_PROJ)/extern/vanetza -I$(ARTERY_PROJ)/extern/vanetza/vanetza/asn1/support -I$(INET_PROJ)/src -I$(LTE_PROJ)/src -I$(VEINS_PROJ)/src -I$(VEINS_PROJ)/subprojects/veins_inet/src -I/usr/local/include -L$(ARTERY_PROJ)/build/Debug/lib -L$(ARTERY_PROJ)/build/Release/lib -L$(ARTERY_PROJ)/extern/vanetza/build/Debug/lib -L$(ARTERY_PROJ)/extern/vanetza/build/Release/lib -L$(CROWNET_PORJ)/src -L$(INET_PROJ)/src -L$(LTE_PROJ)/src -L$(VEINS_PROJ)/src -L$(VEINS_PROJ)/subprojects/veins_inet/src -L/usr/local/lib -lboost_filesystem -lboost_system -lartery_core$(D) -lartery_envmod$(D) -lartery_police$(D) -lartery_storyboard$(D) -ltraci$(D) -lvanetza_access$(D) -lvanetza_asn1$(D) -lvanetza_btp$(D) -lvanetza_common$(D) -lvanetza_dcc$(D) -lvanetza_facilities$(D) -lvanetza_geonet$(D) -lvanetza_gnss$(D) -lvanetza_net$(D) -lvanetza_security$(D) -lCROWNET$(D) -lINET$(D) -llte$(D) -lveins$(D) -lveins_inet$(D) -lgtest -lpthread
#

# Name of target to be created (-o option)
TARGET_DIR = .
TARGET_NAME = rTests$(D)
TARGET = $(TARGET_NAME)$(EXE_SUFFIX)
TARGET_IMPLIB = $(TARGET_NAME)$(IMPLIB_SUFFIX)
TARGET_IMPDEF = $(TARGET_NAME)$(IMPDEF_SUFFIX)
TARGET_FILES = $(TARGET_DIR)/$(TARGET)

# User interface (uncomment one) (-u option)
USERIF_LIBS = $(ALL_ENV_LIBS) # that is, $(QTENV_LIBS) $(CMDENV_LIBS)
#USERIF_LIBS = $(CMDENV_LIBS)
#USERIF_LIBS = $(QTENV_LIBS)

# C++ include paths (with -I)
INCLUDE_PATH = \
    -I. \
    -I$(CROWNET_PORJ)/src \
    -I$(ARTERY_PROJ)/src \
    -I$(ARTERY_PROJ)/src/traci/sumo \
    -I$(ARTERY_PROJ)/extern/vanetza \
    -I$(ARTERY_PROJ)/extern/vanetza/vanetza/asn1/support \
    -I$(INET_PROJ)/src \
    -I$(LTE_PROJ)/src \
    -I$(VEINS_PROJ)/src \
    -I$(VEINS_PROJ)/subprojects/veins_inet/src \
    -I/usr/local/include

# Additional object and library files to link with
EXTRA_OBJS =

# Additional libraries (-L, -l options)
LIBS = $(LDFLAG_LIBPATH)$(ARTERY_PROJ)/build/Debug/lib $(LDFLAG_LIBPATH)$(ARTERY_PROJ)/build/Release/lib $(LDFLAG_LIBPATH)$(ARTERY_PROJ)/extern/vanetza/build/Debug/lib $(LDFLAG_LIBPATH)$(ARTERY_PROJ)/extern/vanetza/build/Release/lib $(LDFLAG_LIBPATH)$(CROWNET_PORJ)/src $(LDFLAG_LIBPATH)$(INET_PROJ)/src $(LDFLAG_LIBPATH)$(LTE_PROJ)/src $(LDFLAG_LIBPATH)$(VEINS_PROJ)/src $(LDFLAG_LIBPATH)$(VEINS_PROJ)/subprojects/veins_inet/src $(LDFLAG_LIBPATH)/usr/local/lib  -lboost_filesystem -lboost_system -lartery_core$(D) -lartery_envmod$(D) -lartery_police$(D) -lartery_storyboard$(D) -ltraci$(D) -lvanetza_access$(D) -lvanetza_asn1$(D) -lvanetza_btp$(D) -lvanetza_common$(D) -lvanetza_dcc$(D) -lvanetza_facilities$(D) -lvanetza_geonet$(D) -lvanetza_gnss$(D) -lvanetza_net$(D) -lvanetza_security$(D) -lCROWNET$(D) -lINET$(D) -llte$(D) -lveins$(D) -lveins_inet$(D) -lgtest -lpthread

# Output directory
PROJECT_OUTPUT_DIR = ../../../out
PROJECTRELATIVE_PATH = tests/gtest/src
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

# Object files for local .cc, .msg and .sm files
OBJS = \
    $O/main_test.o \
    $O/crownet/common/IEntryTest.o \
    $O/crownet/dcd/CellTest.o \
    $O/crownet/dcd/DcDMapFileWriterTest.o \
    $O/crownet/dcd/DcDMapIteratorTest.o \
    $O/crownet/dcd/DcDMapTest.o \
    $O/crownet/dcd/IdentifierTest.o

# Message files
MSGFILES =

# SM files
SMFILES =

# Other makefile variables (-K)
CROWNET_PORJ=../../../../crownet
ARTERY_PROJ=../../../../artery
INET_PROJ=../../../../inet4
LTE_PROJ=../../../../simulte
VEINS_PROJ=../../../../veins
VEINS_INET_PROJ=../../../../veins/subprojects/veins_inet

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
OMNETPP_LIBS = $(OPPMAIN_LIB) $(USERIF_LIBS) $(KERNEL_LIBS) $(SYS_LIBS)
ifneq ($(PLATFORM),win32.x86_64)
LIBS += -Wl,-rpath,$(abspath $(ARTERY_PROJ)/build/Debug/lib) -Wl,-rpath,$(abspath $(ARTERY_PROJ)/build/Release/lib) -Wl,-rpath,$(abspath $(ARTERY_PROJ)/extern/vanetza/build/Debug/lib) -Wl,-rpath,$(abspath $(ARTERY_PROJ)/extern/vanetza/build/Release/lib) -Wl,-rpath,$(abspath $(CROWNET_PORJ)/src) -Wl,-rpath,$(abspath $(INET_PROJ)/src) -Wl,-rpath,$(abspath $(LTE_PROJ)/src) -Wl,-rpath,$(abspath $(VEINS_PROJ)/src) -Wl,-rpath,$(abspath $(VEINS_PROJ)/subprojects/veins_inet/src) -Wl,-rpath,$(abspath /usr/local/lib)
endif

COPTS = $(CFLAGS) $(IMPORT_DEFINES) -DINET_IMPORT -DVEINS_IMPORT -DVEINS_INET_IMPORT $(INCLUDE_PATH) -I$(OMNETPP_INCL_DIR)
MSGCOPTS = $(INCLUDE_PATH)
SMCOPTS =

# we want to recompile everything if COPTS changes,
# so we store COPTS into $COPTS_FILE (if COPTS has changed since last build)
# and make the object files depend on it
COPTS_FILE = $O/.last-copts
ifneq ("$(COPTS)","$(shell cat $(COPTS_FILE) 2>/dev/null || echo '')")
  $(shell $(MKPATH) "$O")
  $(file >$(COPTS_FILE),$(COPTS))
endif

#------------------------------------------------------------------------------
# User-supplied makefile fragment(s)
#------------------------------------------------------------------------------

# Main target
all: $(TARGET_FILES)

$(TARGET_DIR)/% :: $O/%
	@mkdir -p $(TARGET_DIR)
	$(Q)$(LN) $< $@
ifeq ($(TOOLCHAIN_NAME),clang-msabi)
	-$(Q)-$(LN) $(<:%.dll=%.lib) $(@:%.dll=%.lib) 2>/dev/null
endif

$O/$(TARGET): $(OBJS)  $(wildcard $(EXTRA_OBJS)) Makefile $(CONFIGFILE)
	@$(MKPATH) $O
	@echo Creating executable: $@
	$(Q)$(CXX) $(LDFLAGS) -o $O/$(TARGET) $(OBJS) $(EXTRA_OBJS) $(AS_NEEDED_OFF) $(WHOLE_ARCHIVE_ON) $(LIBS) $(WHOLE_ARCHIVE_OFF) $(OMNETPP_LIBS)

.PHONY: all clean cleanall depend msgheaders smheaders

.SUFFIXES: .cc

$O/%.o: %.cc $(COPTS_FILE) | msgheaders smheaders
	@$(MKPATH) $(dir $@)
	$(qecho) "$<"
	$(Q)$(CXX) -c $(CXXFLAGS) $(COPTS) -o $@ $<

%_m.cc %_m.h: %.msg
	$(qecho) MSGC: $<
	$(Q)$(MSGC) -s _m.cc -MD -MP -MF $O/$(basename $<)_m.h.d $(MSGCOPTS) $?

%_sm.cc %_sm.h: %.sm
	$(qecho) SMC: $<
	$(Q)$(SMC) -c++ -suffix cc $(SMCOPTS) $?

msgheaders: $(MSGFILES:.msg=_m.h)

smheaders: $(SMFILES:.sm=_sm.h)

clean:
	$(qecho) Cleaning $(TARGET)
	$(Q)-rm -rf $O
	$(Q)-rm -f $(TARGET_FILES)
	$(Q)-rm -f $(call opp_rwildcard, . , *_m.cc *_m.h *_sm.cc *_sm.h)

cleanall:
	$(Q)$(CLEANALL_COMMAND)
	$(Q)-rm -rf $(PROJECT_OUTPUT_DIR)

help:
	@echo "$$HELP_SYNOPSYS"
	@echo "$$HELP_TARGETS"
	@echo "$$HELP_VARIABLES"
	@echo "$$HELP_EXAMPLES"

# include all dependencies
-include $(OBJS:%=%.d) $(MSGFILES:%.msg=$O/%_m.h.d)
