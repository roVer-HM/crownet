## CrowNet - **Crow**d **Net**work

A central element of intelligent transport infrastructures as well as innovative mobility concepts is networking: intelligent vehicles and traffic control systems exchange information to make traffic safer, more efficient and more environmentally friendly. New forms of communication, e.g. via smartphones, enable new mobility concepts such as car and bike sharing. Here, road users communicate with each other as well as with mobility service providers and the transport infrastructure. The information is distributed via mobile radio and influences the behaviour of road users. At the same time, however, user behaviour and mobility also influence the communication infrastructure - up to a complete breakdown of communication, e.g. during major events or in extreme situations, such as the Munich attack in July 2016. With the increasing spread of intelligent transport systems, these interactions between transport infrastructure, user mobility and communication infrastructure will continue to increase. However, a particularly high density of active, communicating road users often arises from pedestrians travelling in groups - e.g. after a major event - or changing modes of transport. Simulations can be used to investigate whether networking and information dissemination are robust in these mobility scenarios. Existing simulation systems mainly focus on car traffic and vehicle mobility simulation, a detailed microscopic pedestrian mobility model is often not available.

<p align="center"> 
  <img src="ped_antenne.png" alt="Dense crowd in front of a bottleneck" width="50%"/>
  </p>


### The open-source framework to simulate interactions between pedestrians and mobile networks and vice-versa.

CrowNet (for crowd network) is an open-source simulation environment which couples state-of-the art pedestrian locomotion models with wireless
communication models. It can be used to evaluate pedestrian communication in urban and rural environments.


<p align="center"> 
 <img src="coverage_szenarien_dense_ITS_en.png" alt="Communication for different types of coverage" width="100%"/>
</p>


CrowNet is a novel simulation framework based on open-source components that allows to simulate interactions of crowd motion and mobile networks. Most other simulation frameworks simulate pedestrian locomotion separately and before they simulate the communication in the network or do not support a detailed microscopic modelling of pedestrian mobility. Therefore, interactions cannot be observed. Using CrowNet, interactions between locomotion and information dissemination can be investigated systematically in simulation studies.

The source code can be found [here](https://github.com/roVer-HM/crownet).

### Research project
CrowNet has been developed as part of the [roVer](https://www.hm.edu/allgemein/forschung_entwicklung/forschungsprojekte/projektdetails/wischhof/wischhof_koester_rover.de.html) research project at [Munich University of Applied Sciences](https://www.hm.edu/en/index.en.html).
roVer is a project funded by the [Federal Ministry of Education and Research](https://www.bmbf.de/) (Grant number: 13FH669IX6).


### Acknowledgments
CrowNet is based on a number of other software components which are developed by the research community:

* [Vadere](http://www.vadere.org/)
* [SimuLTE](https://simulte.com/) / [Simu5G](http://simu5g.org/)
* [Artery](http://artery.v2x-research.eu/)
* [Veins](http://veins.car2x.org/)
* [sumo](https://www.eclipse.org/sumo/)
* [INET Framework](https://inet.omnetpp.org/)
* [OMNeT++](https://omnetpp.org/)

We would like to thank all organizations and individuals involved for making these great projects publicly available. Without these, CrowNet could not be implemented.

### Documentation
The documentation can be found in our [Github project](https://github.com/roVer-HM/crownet/blob/master/doc/README.md)

### Contributions

We welcome contributions from anyone, who is interested in extending or improving the software framework. Please submit a pull-request via github.

###  Contact

Having trouble with CrowNet? Please feel free to contact [us](https://github.com/orgs/roVer-HM/people).
