# OMNeT++ Simulator (as used within the roVer project)
#
# Within roVer, we use two different containers:
#   1) omnetpp (command line, no GUI, for simulation campaigns)
#   2) omnetpp-ide (all graphical libs, autostart of IDE)
#
# This Dockerfile is used to build the omnetpp-ide container,
# which extends the omnetpp container with a GUI



#FROM sam-dev.cs.hm.edu:5023/rover/crownet/omnetpp:latest
FROM sam-dev.cs.hm.edu:5023/rover/crownet/omnetpp:6.0pre10

WORKDIR /opt/omnetpp/omnetpp

# now install the Eclipse IDE base
ADD ide/ide.tar.gz .

# installation of tools for code formatting and checks (cpplint and cppstyle eclipse plugin)
RUN pip3 install cpplint
# check if PID if is empty. If so this will create an infinte loop at while [ -e /proc/ ]
#RUN /opt/omnetpp/omnetpp/bin/omnetpp -application org.eclipse.equinox.p2.director \
#        -repository http://www.cppstyle.com/mars -installIU org.wangzw.cppstyle.feature.feature.group; \
#        PID=`pidof omnetpp`; \
#        if [ -z "$PID" ]; then echo "error installing plugin. Is ide installed?" && exit 1; fi; \
#        while [ -e /proc/$PID ]; do sleep 0.1 ; done

# update permissions so that the omnetpp directory becomes writable
RUN chmod -R a+rw /opt/omnetpp/omnetpp

# init script (triggered at container start)
COPY init.sh /init.sh
RUN chmod 555 /init.sh
ENTRYPOINT [ "/init.sh" ]

# waitfor script (in order to wait for non-child processes)
COPY waitfor.sh /waitfor.sh
RUN chmod 555 /waitfor.sh
