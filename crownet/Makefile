MAKEMAKE=opp_makemake $(MMOPT)

ARTERY_PROJ=../artery
VANETZA_PROJ=${ARTERY_PROJ}/extern/vanetza
INET_PROJ=../inet4
LTE_PROJ=../simulte
VEINS_PROJ=../veins
VEINS_INET_PROJ=../veins/subprojects/veins_inet

GOOGLE_TEST_LIB = gtest
GOOGLE_TEST_INCLUDE = /usr/local/include


all: checkmakefiles
	cd src && $(MAKE)

clean: checkmakefiles
	cd src && $(MAKE) clean

cleanall: checkmakefiles
	cd src && $(MAKE) MODE=release clean
	cd src && $(MAKE) MODE=debug clean
	rm -f src/Makefile

makefiles-cmdOnly:
		cd src && $(MAKEMAKE)-u Cmdenv --make-so -f --deep -o CROWNET -O out -pCROWNET -KINET_PROJ=../../inet4 -KLTE_PROJ=../../simulte -KVEINS_PROJ=../../veins -KVEINS_INET_PROJ=../../veins/subprojects/veins_inet -DINET_IMPORT -DVEINS_IMPORT -DVEINS_INET_IMPORT -I. -I$$\(INET_PROJ\)/src -I$$\(LTE_PROJ\)/src -I$$\(VEINS_PROJ\)/src -I. -I$$\(VEINS_PROJ\)/subprojects/veins_inet/src -I$$\(VEINS_PROJ\)/src/veins -L$$\(INET_PROJ\)/src -L$$\(LTE_PROJ\)/src -L$$\(VEINS_PROJ\)/src -L$$\(VEINS_PROJ\)/subprojects/veins_inet/src -lINET$$\(D\) -llte$$\(D\) -lveins$$\(D\) -lveins_inet$$\(D\)

makefiles:
	cd src && $(MAKEMAKE) --make-so -f --deep -o CROWNET -O out -pCROWNET \
		-KARTERY_PROJ=../../artery \
		-KINET_PROJ=../../inet4 \
		-KLTE_PROJ=../../simulte \
		-KVEINS_PROJ=../../veins \
		-KVEINS_INET_PROJ=../../veins/subprojects/veins_inet \
		-DINET_IMPORT -DVEINS_IMPORT -DVEINS_INET_IMPORT \
		-I. \
		-I$$\(ARTERY_PROJ\)/src \
		-I$$\(ARTERY_PROJ\)/src/traci/sumo \
		-I$$\(ARTERY_PROJ\)/extern/vanetza \
		-I$$\(ARTERY_PROJ\)/extern/vanetza/vanetza/asn1/support \
		-I$$\(INET_PROJ\)/src \
		-I$$\(LTE_PROJ\)/src \
		-I$$\(VEINS_PROJ\)/src \
		-I$$\(VEINS_PROJ\)/subprojects/veins_inet/src \
		-L$$\(ARTERY_PROJ\)/build/Debug/lib \
		-L$$\(ARTERY_PROJ\)/build/Release/lib \
		-L$$\(ARTERY_PROJ\)/extern/vanetza/build/Debug/lib \
		-L$$\(ARTERY_PROJ\)/extern/vanetza/build/Release/lib \
		-L$$\(INET_PROJ\)/src \
		-L$$\(LTE_PROJ\)/src \
		-L$$\(VEINS_PROJ\)/src \
		-L$$\(VEINS_PROJ\)/subprojects/veins_inet/src \
		-lboost_filesystem \
		-lboost_system \
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
		-lvanetza_security$$\(D\) \
		-lINET$$\(D\) \
		-llte$$\(D\) \
		-lveins$$\(D\) \
		-lveins_inet$$\(D\)


ifeq ("$(MODE)","debug")
	_D=dbg
else
	_D=ddd
endif

test: make-test
	if [ "$(MODE)" = "debug" ]; then cd tests/gtest/src && ./rTests_dbg; else cd tests/gtest/src && ./rTests;fi

clean-test:
	cd tests/gtest/src && $(MAKE) MODE=release clean
	cd tests/gtest/src && $(MAKE) MODE=debug clean
	rm -f tests/gtest/src/Makefile

make-test: makefiles-test
	cd tests/gtest/src && $(MAKE)

makefiles-test:
	cd tests/gtest/src && $(MAKEMAKE) -f --deep -o rTests -O out  \
		-KCROWNET_PORJ=../../../../crownet \
		-KARTERY_PROJ=../../../../artery \
		-KINET_PROJ=../../../../inet4 \
		-KLTE_PROJ=../../../../simulte \
		-KVEINS_PROJ=../../../../veins \
		-KVEINS_INET_PROJ=../../../../veins/subprojects/veins_inet \
		-DINET_IMPORT -DVEINS_IMPORT -DVEINS_INET_IMPORT \
		-I. \
		-I$$\(CROWNET_PORJ\)/src \
		-I$$\(ARTERY_PROJ\)/src \
		-I$$\(ARTERY_PROJ\)/src/traci/sumo \
		-I$$\(ARTERY_PROJ\)/extern/vanetza \
		-I$$\(ARTERY_PROJ\)/extern/vanetza/vanetza/asn1/support \
		-I$$\(INET_PROJ\)/src \
		-I$$\(LTE_PROJ\)/src \
		-I$$\(VEINS_PROJ\)/src \
		-I$$\(VEINS_PROJ\)/subprojects/veins_inet/src \
		-I$(GOOGLE_TEST_INCLUDE) \
		-L$$\(ARTERY_PROJ\)/build/Debug/lib \
		-L$$\(ARTERY_PROJ\)/build/Release/lib \
		-L$$\(ARTERY_PROJ\)/extern/vanetza/build/Debug/lib \
		-L$$\(ARTERY_PROJ\)/extern/vanetza/build/Release/lib \
		-L$$\(CROWNET_PORJ\)/src \
		-L$$\(INET_PROJ\)/src \
		-L$$\(LTE_PROJ\)/src \
		-L$$\(VEINS_PROJ\)/src \
		-L$$\(VEINS_PROJ\)/subprojects/veins_inet/src \
		-L/usr/local/lib \
		-lboost_filesystem \
		-lboost_system \
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
		-lvanetza_security$$\(D\) \
		-lCROWNET$$\(D\) \
		-lINET$$\(D\) \
		-llte$$\(D\) \
		-lveins$$\(D\) \
		-lveins_inet$$\(D\) \
		-l$(GOOGLE_TEST_LIB) \
		-lpthread

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
