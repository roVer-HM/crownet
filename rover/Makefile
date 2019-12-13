MAKEMAKE=opp_makemake $(MMOPT)

INET_PROJ=../inet4
LTE_PROJ=../simulte
VEINS_PROJ=../veins
VEINS_INET_PROJ=../veins/subprojects/veins_inet


all: checkmakefiles
	cd src && $(MAKE)

clean: checkmakefiles
	cd src && $(MAKE) clean

cleanall: checkmakefiles
	cd src && $(MAKE) MODE=release clean
	cd src && $(MAKE) MODE=debug clean
	rm -f src/Makefile

makefiles-cmdOnly:
		cd src && $(MAKEMAKE)-u Cmdenv --make-so -f --deep -o ROVER -O out -pROVER -KINET_PROJ=../../inet4 -KLTE_PROJ=../../simulte -KVEINS_PROJ=../../veins -KVEINS_INET_PROJ=/home/sts/repos/rover-main/veins/subprojects/veins_inet -DINET_IMPORT -DVEINS_IMPORT -DVEINS_INET_IMPORT -I. -I$$\(INET_PROJ\)/src -I$$\(LTE_PROJ\)/src -I$$\(VEINS_PROJ\)/src -I. -I$$\(VEINS_PROJ\)/subprojects/veins_inet/src -I$$\(VEINS_PROJ\)/src/veins -L$$\(INET_PROJ\)/src -L$$\(LTE_PROJ\)/src -L$$\(VEINS_PROJ\)/src -L$$\(VEINS_PROJ\)/subprojects/veins_inet/src -lINET$$\(D\) -llte$$\(D\) -lveins$$\(D\) -lveins_inet$$\(D\)

makefiles:
		cd src && $(MAKEMAKE) --make-so -f --deep -o ROVER -O out -pROVER -KINET_PROJ=../../inet4 -KLTE_PROJ=../../simulte -KVEINS_PROJ=../../veins -KVEINS_INET_PROJ=/home/sts/repos/rover-main/veins/subprojects/veins_inet -DINET_IMPORT -DVEINS_IMPORT -DVEINS_INET_IMPORT -I. -I$$\(INET_PROJ\)/src -I$$\(LTE_PROJ\)/src -I$$\(VEINS_PROJ\)/src -I. -I$$\(VEINS_PROJ\)/subprojects/veins_inet/src -I$$\(VEINS_PROJ\)/src/veins -L$$\(INET_PROJ\)/src -L$$\(LTE_PROJ\)/src -L$$\(VEINS_PROJ\)/src -L$$\(VEINS_PROJ\)/subprojects/veins_inet/src -lINET$$\(D\) -llte$$\(D\) -lveins$$\(D\) -lveins_inet$$\(D\)

checkmakefiles:
	@if [ ! -f src/Makefile ]; then \
	echo; \
	echo '======================================================================='; \
	echo 'src/Makefile does not exist. Please use "make makefiles" to generate it!'; \
	echo '======================================================================='; \
	echo; \
	exit 1; \
	fi
