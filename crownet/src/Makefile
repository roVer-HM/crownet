#
# OMNeT++/OMNEST Makefile for $(LIB_PREFIX)ROVER
#
# This file was generated with the command:
#  opp_makemake --make-so -f --deep -o ROVER -O out -pROVER -KARTERY_PROJ=../../artery -KINET_PROJ=../../inet4 -KLTE_PROJ=../../simulte -KVEINS_PROJ=../../veins -KVEINS_INET_PROJ=/home/sts/repos/crownet/veins/subprojects/veins_inet -DINET_IMPORT -DVEINS_IMPORT -DVEINS_INET_IMPORT -I. -I$$\(ARTERY_PROJ\)/src -I$$\(ARTERY_PROJ\)/src/traci/sumo -I$$\(ARTERY_PROJ\)/extern/vanetza -I$$\(ARTERY_PROJ\)/extern/vanetza/vanetza/asn1/support -I$$\(INET_PROJ\)/src -I$$\(LTE_PROJ\)/src -I$$\(VEINS_PROJ\)/src -I$$\(VEINS_PROJ\)/subprojects/veins_inet/src -I$$\(VEINS_PROJ\)/src/veins -L$$\(ARTERY_PROJ\)/build/Debug/lib -L$$\(ARTERY_PROJ\)/build/Release/lib -L$$\(ARTERY_PROJ\)/extern/vanetza/build/Debug/lib -L$$\(ARTERY_PROJ\)/extern/vanetza/build/Release/lib -L$$\(INET_PROJ\)/src -L$$\(LTE_PROJ\)/src -L$$\(VEINS_PROJ\)/src -L$$\(VEINS_PROJ\)/subprojects/veins_inet/src -lboost_filesystem -lartery_core$$\(D\) -lartery_envmod$$\(D\) -lartery_police$$\(D\) -lartery_storyboard$$\(D\) -ltraci$$\(D\) -lvanetza_access$$\(D\) -lvanetza_asn1$$\(D\) -lvanetza_btp$$\(D\) -lvanetza_common$$\(D\) -lvanetza_dcc$$\(D\) -lvanetza_facilities$$\(D\) -lvanetza_geonet$$\(D\) -lvanetza_gnss$$\(D\) -lvanetza_net$$\(D\) -lvanetza_security$$\(D\) -lINET$$\(D\) -llte$$\(D\) -lveins$$\(D\) -lveins_inet$$\(D\)
#

# Name of target to be created (-o option)
TARGET = $(LIB_PREFIX)ROVER$(D)$(SHARED_LIB_SUFFIX)
TARGET_DIR = .

# C++ include paths (with -I)
INCLUDE_PATH = \
    -I. \
    -I$(ARTERY_PROJ)/src \
    -I$(ARTERY_PROJ)/src/traci/sumo \
    -I$(ARTERY_PROJ)/extern/vanetza \
    -I$(ARTERY_PROJ)/extern/vanetza/vanetza/asn1/support \
    -I$(INET_PROJ)/src \
    -I$(LTE_PROJ)/src \
    -I$(VEINS_PROJ)/src \
    -I$(VEINS_PROJ)/subprojects/veins_inet/src \
    -I$(VEINS_PROJ)/src/veins

# Additional object and library files to link with
EXTRA_OBJS =

# Additional libraries (-L, -l options)
LIBS = $(LDFLAG_LIBPATH)$(ARTERY_PROJ)/build/Debug/lib $(LDFLAG_LIBPATH)$(ARTERY_PROJ)/build/Release/lib $(LDFLAG_LIBPATH)$(ARTERY_PROJ)/extern/vanetza/build/Debug/lib $(LDFLAG_LIBPATH)$(ARTERY_PROJ)/extern/vanetza/build/Release/lib $(LDFLAG_LIBPATH)$(INET_PROJ)/src $(LDFLAG_LIBPATH)$(LTE_PROJ)/src $(LDFLAG_LIBPATH)$(VEINS_PROJ)/src $(LDFLAG_LIBPATH)$(VEINS_PROJ)/subprojects/veins_inet/src  -lboost_filesystem -lartery_core$(D) -lartery_envmod$(D) -lartery_police$(D) -lartery_storyboard$(D) -ltraci$(D) -lvanetza_access$(D) -lvanetza_asn1$(D) -lvanetza_btp$(D) -lvanetza_common$(D) -lvanetza_dcc$(D) -lvanetza_facilities$(D) -lvanetza_geonet$(D) -lvanetza_gnss$(D) -lvanetza_net$(D) -lvanetza_security$(D) -lINET$(D) -llte$(D) -lveins$(D) -lveins_inet$(D)

# Output directory
PROJECT_OUTPUT_DIR = ../out
PROJECTRELATIVE_PATH = src
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

# Object files for local .cc, .msg and .sm files
OBJS = \
    $O/rover/rover.o \
    $O/rover/aid/Aid.o \
    $O/rover/aid/Aid_test.o \
    $O/rover/aid/AidConnection.o \
    $O/rover/aid/AidConnection_test.o \
    $O/rover/aid/AidHeader.o \
    $O/rover/aid/AidSocket.o \
    $O/rover/aid/AidSocket_test.o \
    $O/rover/aid/algorithms/AidAlgorithm.o \
    $O/rover/aid/algorithms/AidAlgorithm_test.o \
    $O/rover/applications/aid/ArteryDensityMapApp.o \
    $O/rover/applications/aid/VruAid.o \
    $O/rover/applications/common/AidBaseApp.o \
    $O/rover/applications/common/AidBaseApp_test.o \
    $O/rover/applications/common/BaseApp.o \
    $O/rover/applications/common/BaseApp_test.o \
    $O/rover/applications/common/UdpBaseApp.o \
    $O/rover/applications/common/UdpBaseApp_test.o \
    $O/rover/applications/udpapp/UdpPubTransInfo.o \
    $O/rover/applications/udpapp/detour/UdpDetourApp.o \
    $O/rover/applications/udpapp/detour/UdpDetourApp_test.o \
    $O/rover/applications/udpapp/detour/UdpDetourAppVadere.o \
    $O/rover/applications/udpapp/vru/VruUdp.o \
    $O/rover/applications/udpapp/vru/VruUdp_test.o \
    $O/rover/artery/PedestrianMiddelwareEmpyt.o \
    $O/rover/artery/PedestrianMiddleware.o \
    $O/rover/artery/lte/LteRadioDriver.o \
    $O/rover/artery/traci/MobilityRoverArtery.o \
    $O/rover/artery/traci/PedestrianPositionProvider.o \
    $O/rover/artery/traci/VadereApi.o \
    $O/rover/artery/traci/VadereCore.o \
    $O/rover/artery/traci/VadereLauchner.o \
    $O/rover/artery/traci/VadereNodeManager.o \
    $O/rover/artery/traci/VaderePersonController.o \
    $O/rover/artery/traci/VadereSubscriptionManager.o \
    $O/rover/artery/traci/VadereUtils.o \
    $O/rover/artery/traci/VariableCache.o \
    $O/rover/common/GlobalDensityMap.o \
    $O/rover/common/SocketHandler.o \
    $O/rover/common/SocketHandler_test.o \
    $O/rover/common/converter/OsgCoordConverter.o \
    $O/rover/common/converter/OsgCoordinateConverter.o \
    $O/rover/common/result/ResultFilters.o \
    $O/rover/common/result/ResultRecorders.o \
    $O/rover/common/result/Simsignals.o \
    $O/rover/common/util/FileWriter.o \
    $O/rover/common/util/NedFunctions.o \
    $O/rover/common/util/rover_util.o \
    $O/rover/dcd/identifier/CellKeyProvider.o \
    $O/rover/dcd/identifier/Identifiers.o \
    $O/rover/dcd/regularGrid/RegularCell.o \
    $O/rover/dcd/regularGrid/RegularCellVisitors.o \
    $O/rover/dcd/regularGrid/RegularDcdMap.o \
    $O/rover/dcd/regularGrid/RegularDcdMapPrinter.o \
    $O/rover/simulte/corenetwork/lteip/Geo2Lte.o \
    $O/rover/aid/AidCommand_m.o \
    $O/rover/aid/AidHeader_m.o \
    $O/rover/applications/PositionMapPacket_m.o \
    $O/rover/applications/common/AppCommon_m.o \
    $O/rover/applications/udpapp/detour/DetourAppPacket_m.o \
    $O/rover/artery/lte/GeoNetTag_m.o \
    $O/rover/common/ItsPdu_m.o \
    $O/rover/common/util/rover_util_m.o

# Message files
MSGFILES = \
    rover/aid/AidCommand.msg \
    rover/aid/AidHeader.msg \
    rover/applications/PositionMapPacket.msg \
    rover/applications/common/AppCommon.msg \
    rover/applications/udpapp/detour/DetourAppPacket.msg \
    rover/artery/lte/GeoNetTag.msg \
    rover/common/ItsPdu.msg \
    rover/common/util/rover_util.msg

# SM files
SMFILES =

# Other makefile variables (-K)
ARTERY_PROJ=../../artery
INET_PROJ=../../inet4
LTE_PROJ=../../simulte
VEINS_PROJ=../../veins
VEINS_INET_PROJ=/home/sts/repos/crownet/veins/subprojects/veins_inet

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
OMNETPP_LIBS = -loppenvir$D $(KERNEL_LIBS) $(SYS_LIBS)
ifneq ($(TOOLCHAIN_NAME),clangc2)
LIBS += -Wl,-rpath,$(abspath $(ARTERY_PROJ)/build/Debug/lib) -Wl,-rpath,$(abspath $(ARTERY_PROJ)/build/Release/lib) -Wl,-rpath,$(abspath $(ARTERY_PROJ)/extern/vanetza/build/Debug/lib) -Wl,-rpath,$(abspath $(ARTERY_PROJ)/extern/vanetza/build/Release/lib) -Wl,-rpath,$(abspath $(INET_PROJ)/src) -Wl,-rpath,$(abspath $(LTE_PROJ)/src) -Wl,-rpath,$(abspath $(VEINS_PROJ)/src) -Wl,-rpath,$(abspath $(VEINS_PROJ)/subprojects/veins_inet/src)
endif

COPTS = $(CFLAGS) $(IMPORT_DEFINES) -DINET_IMPORT -DVEINS_IMPORT -DVEINS_INET_IMPORT -DROVER_EXPORT $(INCLUDE_PATH) -I$(OMNETPP_INCL_DIR)
MSGCOPTS = $(INCLUDE_PATH) -PROVER_API
SMCOPTS =

# we want to recompile everything if COPTS changes,
# so we store COPTS into $COPTS_FILE and have object
# files depend on it (except when "make depend" was called)
COPTS_FILE = $O/.last-copts
ifneq ("$(COPTS)","$(shell cat $(COPTS_FILE) 2>/dev/null || echo '')")
$(shell $(MKPATH) "$O" && echo "$(COPTS)" >$(COPTS_FILE))
endif

#------------------------------------------------------------------------------
# User-supplied makefile fragment(s)
# >>>
# inserted from file 'makefrag':
#
# Use the new message compiler introduced in OMNeT++ 5.3
#
MSGC:=$(MSGC) --msg6

#
# compiler warning for c-style cast
#
#CFLAGS += -Wold-style-cast
#CFLAGS += -save-temps

# <<<
#------------------------------------------------------------------------------

# Main target
all: $(TARGET_DIR)/$(TARGET)

$(TARGET_DIR)/% :: $O/%
	@mkdir -p $(TARGET_DIR)
	$(Q)$(LN) $< $@
ifeq ($(TOOLCHAIN_NAME),clangc2)
	$(Q)-$(LN) $(<:%.dll=%.lib) $(@:%.dll=%.lib)
endif

$O/$(TARGET): $(OBJS)  $(wildcard $(EXTRA_OBJS)) Makefile $(CONFIGFILE)
	@$(MKPATH) $O
	@echo Creating shared library: $@
	$(Q)$(SHLIB_LD) -o $O/$(TARGET) $(OBJS) $(EXTRA_OBJS) $(AS_NEEDED_OFF) $(WHOLE_ARCHIVE_ON) $(LIBS) $(WHOLE_ARCHIVE_OFF) $(OMNETPP_LIBS) $(LDFLAGS)
	$(Q)$(SHLIB_POSTPROCESS) $O/$(TARGET)

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
	$(Q)-rm -f $(TARGET_DIR)/$(TARGET)
	$(Q)-rm -f $(TARGET_DIR)/$(TARGET:%.dll=%.lib)
	$(Q)-rm -f $(call opp_rwildcard, . , *_m.cc *_m.h *_sm.cc *_sm.h)

cleanall:
	$(Q)$(MAKE) -s clean MODE=release
	$(Q)$(MAKE) -s clean MODE=debug
	$(Q)-rm -rf $(PROJECT_OUTPUT_DIR)

# include all dependencies
-include $(OBJS:%=%.d) $(MSGFILES:%.msg=$O/%_m.h.d)