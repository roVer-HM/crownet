# OMNeT++ Simulator (as used within the roVer project)
# To be discussed: Split this in two containers:
#   1) omnetpp-base (command line, no GUI, for simulation campaigns)
#   2) omnetpp-ide (all graphical libs, autostart of IDE)


FROM sam-dev.cs.hm.edu:5023/rover/rover-main/omnetpp:latest

# Specify the version/tag of omnetpp to be put in the container:
ENV OMNETRELEASE omnetpp-5.6
# Note: Remember to update the description below when changing OMNETRELEASE


LABEL description "OMNeT++ Simulation Framework (as used in roVer, tag omnetpp-5.6)"
LABEL maintainer  "wischhof@ieee.org"


WORKDIR /opt/omnetpp/omnetpp

# now install the Eclipse IDE base
ADD ide/ide.tar.gz .

# installation of tools for code formatting and checks (cpplint and cppstyle eclipse plugin)
RUN pip3 install cpplint
RUN /opt/omnetpp/omnetpp/bin/omnetpp -application org.eclipse.equinox.p2.director -repository http://www.cppstyle.com/mars -installIU org.wangzw.cppstyle.feature.feature.group ; PID=`pidof omnetpp`; while [ -e /proc/$PID ]; do sleep 0.1; done

# update permissions so that the omnetpp directory becomes writable
RUN chmod -R a+rw /opt/omnetpp/omnetpp

# init script (triggered at container start)
COPY init.sh /init.sh
RUN chmod 555 /init.sh
ENTRYPOINT [ "/init.sh" ]

# waitfor script (in order to wait for non-child processes)
COPY waitfor.sh /waitfor.sh
RUN chmod 555 /waitfor.sh