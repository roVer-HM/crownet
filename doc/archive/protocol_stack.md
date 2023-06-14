# roVer Protocol Stack and Services

## Requirements
Provide *robust information dissemination* for location-dependent information
for the local area of mobile persons/pedestrians (refer to application model
for examples):
* scalability
* Provide information dissemination in local area even if network
  infrastructure is not available (i.e. overloaded, destroyed or in case of a
  power surge)
* dynamic adaptation to the locally available communication technologies and
  data rates


## Motivation

**State-of-the-Art does not provide robust information dissemination.**

The current state of the art is based on the classical TCP/IP protocol stack
and a client-server based architecture: The mobile person uses a smartphone-app
to connect via the cellular (2G/3G/4G) infrastructure to a server within the
Internet. Location-dependent information is usually requested on-demand by the
smartphone app from the central server. Alternatively, the information is
pushed to all users with an area where the information is relevant - however,
this implies a periodic update of the local position of a user at the central
server.

TODO: diagram of classic stack and system structure

*Known Problems of the classical TCP/IP stack in roVer scenarios:*
* Overload within the cellular networks: Depending on the local density of
  users, individual cells or the core-network components can be overloaded.
* Overload at the central server(s) within the Internet, examples include:
    * assassination in Munich in July 2016 [Greiner2016]
    * nationwide alert day 2020 (https://warnung-der-bevoelkerung.de/en/)
* Unicast TCP does not scale well and suffers drastically in case of high
  packet error rates or only partially connected networks with short
  link-lifetimes.
* Routing is based on (IP-)addresses - for local mobility information, neither
  the IP-address of the source nor of the destination are usually known.
  Therefore, a (centralized) server (whose address is known beforehand) is used
  to store and disseminate the information.
* Geographic routing protocols (as known from numerous works in the scientific
  community) suffers in situations with high mobility. Furthermore, the
  geographic position of potentially interested receivers is often not known.
* Locally available data-rate varies by several orders of magnitude due to the
  location-dependent availability of communication networks (2G...5G),
  adaptive modulation, etc. The state-of-the-art TCP/IP network protocol stack
  does not support a dynamic adaptation of the information dissemination to the
  situation within the local area.
* Locally available communication technologies (cellular/direct,
  2G/3G/4G/5G/WLAN/Bluetooth) vary significantly - in these heterogeneous
  networks the state-of-the-art protocol stack does not support a dynamic
  adaption to the locally available technologies in these heterogeneous
  networks.

## Overview of roVer Protocol Stack

TODO: explanation and figure with roVer protocol stack, highlighting the
modifications (e.g. AID-service)

## References

* [Greiner2016]
* Lena Greiner, „So funktioniert Katwarn - oder auch nicht“,
  Spiegel Online, 23-Juli-2016. [Online]. Verfügbar unter: http://www.spiegel.de/netzwelt/apps/katwarn-so-funktioniert-das-warnsystem-und-das-sind-die-probleme-a-1104421.html. [Zuge-griffen: 16-Feb-2017].
