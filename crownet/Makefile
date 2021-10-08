MAKEMAKE=opp_makemake $(MMOPT)

#ARTERY_PROJ=../artery
#VANETZA_PROJ=${ARTERY_PROJ}/extern/vanetza
#INET_PROJ=../inet4
#5G_PROJ=../simu5g
#VEINS_PROJ=../veins
#VEINS_INET_PROJ=../veins/subprojects/veins_inet

GOOGLE_TEST_LIB = gtest
GOOGLE_MOCK_LIB = gmock
GOOGLE_TEST_INCLUDE = /usr/local/include
GOOLGE_ARGS = ""

TARGT_NAME := CROWNET

ifeq ("$(MODE)","debug")
	_D=_dbg
endif

ifeq ("$(GFILTER)", "")

else
	GOOGLE_ARGS := $(GOOGLE_ARGS) --gtest_filter=$(GFILTER)
endif

NUM_CPUS := $(shell grep -c ^processor /proc/cpuinfo)

all: checkmakefiles
	cd src && $(MAKE) -j $(NUM_CPUS)

.PHONY: build-lib build-exec

clean: checkmakefiles
	cd src && $(MAKE) clean
	echo clean all build targets ...
	-rm -f src/CROWNET_dbg
	-rm -f src/CROWNET
	-rm -f src/linCROWNET_dbg
	-rm -f src/libCROWNET

cleanall: checkmakefiles
	cd src && $(MAKE) MODE=release clean
	cd src && $(MAKE) MODE=debug clean
	rm -f src/Makefile

PROJECTS_VARS := -KARTERY_PROJ=../../artery \
				 -KINET_PROJ=../../inet4 \
			 	 -K5G_PROJ=../../simu5g \
				 -KVEINS_PROJ=../../veins
				 -KVEINS_INET_PROJ=../../veins/subprojects/veins_inet

ARTERY_ := 	-I$$\(ARTERY_PROJ\)/src \
			-I$$\(ARTERY_PROJ\)/src/traci/sumo \
	   		-I$$\(ARTERY_PROJ\)/extern/vanetza \
	   		-I$$\(ARTERY_PROJ\)/extern/vanetza/vanetza/asn1/support \
	   	    -L$$\(ARTERY_PROJ\)/build/Debug/lib \
		    -L$$\(ARTERY_PROJ\)/build/Release/lib \
		    -L$$\(ARTERY_PROJ\)/extern/vanetza/build/Debug/lib \
		    -L$$\(ARTERY_PROJ\)/extern/vanetza/build/Release/lib \
		    -lartery_core$$\(D\) \
		    -lartery_envmod$$\(D\) \
		   	-lartery_police$$\(D\) \
		    -lartery_storyboard$$\(D\) \
    	    -ltraci$$\(D\) \
		   	-lvanetza_access$$\(D\) \
		    -lvanetza_asn1$$\(D\) \
		    -lvanetza_btp$$\(D\) \
		    -lvanetza_common$$\(D\) \
		    -lvanetza_dcc$$\(D\) \
		    -lvanetza_facilities$$\(D\) \
		    -lvanetza_geonet$$\(D\) \
		    -lvanetza_gnss$$\(D\) \
		    -lvanetza_net$$\(D\) \
		    -lvanetza_security$$\(D\)

VEINS_ := -DVEINS_IMPORT -DVEINS_INET_IMPORT \
		  -I$$\(VEINS_PROJ\)/src \
	  	  -I$$\(VEINS_PROJ\)/subprojects/veins_inet/src \
		  -I$$\(VEINS_PROJ\)/src/veins \
	      -L$$\(VEINS_PROJ\)/src \
	      -L$$\(VEINS_PROJ\)/subprojects/veins_inet/src \
		  -lveins$$\(D\) \
	      -lveins_inet$$\(D\)

INET_5G_ := -DINET_IMPORT \
	  		 -I$$\(INET_PROJ\)/src \
			 -I$$\(5G_PROJ\)/src \
			 -L$$\(INET_PROJ\)/src \
			 -L$$\(5G_PROJ\)/src \
		     -lINET$$\(D\) \
		     -lsimu5g$$\(D\)

MISC_ := -lboost_filesystem \
	     -lboost_system \
	     -losg \
	     -losgEarth

TEST_ := -I$(GOOGLE_TEST_INCLUDE) \
		 -L/usr/local/lib \
		 -l$(GOOGLE_TEST_LIB) \
		 -l$(GOOGLE_MOCK_LIB) \
		 -lpthread

makefiles:
	cd src && $(MAKEMAKE) -f --deep -o CROWNET -O out -pCROWNET \
	   $(PROJECTS_VARS) \
	   -I. \
	   $(ARTERY_) \
	   $(VEINS_) \
	   $(INET_5G_) \
	   $(MISC_)
	rm -f src/__make_sharedlib
	touch src/__make_executable


makefiles-so:
	cd src && $(MAKEMAKE) --make-so -f --deep -o CROWNET -O out -pCROWNET \
		$(PROJECTS_VARS) \
		-I. \
		$(ARTERY_) \
	   	$(VEINS_) \
	   	$(INET_5G_) \
		$(MISC_)
	rm -f src/__make_executable
	touch  src/__make_sharedlib

src/libCROWNET$(_D) : makefiles-so
	cd src &&  $(MAKE) -j $(NUM_CPUS)

src/CROWNET$(D): makefiles
	cd src &&  $(MAKE) -j $(NUM_CPUS)

build-lib: src/libCROWNET$(_D)

build-exec: src/CROWNET$(_D)


opp-test: opp-test-make
	cd tests/omnetpp && ./runtest *.test

opp-test-make: build-lib
	cd tests/omnetpp && ./gentest *.test 

opp-test-clean:
	rm -rf tests/omnetpp/work

# fingerprint tests
test: make-test
	if [ "$(MODE)" = "debug" ]; then cd tests/gtest/src && ./rTests_dbg $(GOOGLE_ARGS); else cd tests/gtest/src && ./rTests $(GOOGLE_ARGS) ;fi

test-clean: 
	rm -f tests/gtest/src/rTests
	rm -f tests/gtest/src/rTests_dbg

clean-fingerprint-test:
	cd tests/gtest/src && $(MAKE) MODE=release clean
	cd tests/gtest/src && $(MAKE) MODE=debug clean
	rm -f tests/gtest/src/Makefile

make-test: makefiles-test
	cd tests/gtest/src && $(MAKE) -j $(NUM_CPUS)

makefiles-test: src/libCROWNET$(_D)
	cd tests/gtest/src && $(MAKEMAKE) -f --deep -o rTests -O out  \
		-KCROWNET_PORJ=../../../../crownet \
        -KARTERY_PROJ=../../../../artery \
		-KINET_PROJ=../../../../inet4 \
		-K5G_PROJ=../../../../simu5g \
		-KVEINS_PROJ=../../../../veins \
		-KVEINS_INET_PROJ=../../../../veins/subprojects/veins_inet \
		-I. \
		-I$$\(CROWNET_PORJ\)/src \
		-L$$\(CROWNET_PORJ\)/src \
		-lCROWNET$$\(D\) \
		$(ARTERY_) \
	   	$(VEINS_) \
	   	$(INET_5G_) \
		$(MISC_) \
		$(TEST_)

checktestmakefiles:
	@if [ ! -f tests/gtest/src/Makefile ]; then \
	echo; \
	echo '======================================================================='; \
	echo 'src/Makefile does not exist. Please use "make makefiles" to generate it!'; \
	echo '======================================================================='; \
	echo; \
	exit 1; \
	fi

checkmakefiles:
	@if [ ! -f src/Makefile ]; then \
	echo; \
	echo '======================================================================='; \
	echo 'src/Makefile does not exist. Please use "make makefiles" to generate it!'; \
	echo '======================================================================='; \
	echo; \
	exit 1; \
	fi
