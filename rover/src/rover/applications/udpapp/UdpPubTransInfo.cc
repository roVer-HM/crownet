/*
 * UdpPubTransInfo.cc
 *
 *  Created on: Nov 19, 2019
 *      Author: stsc
 */

#include "UdpPubTransInfo.h"

#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/FragmentationTag_m.h"

using inet::L3Address;
using inet::L3AddressResolver;
using omnetpp::cStringTokenizer;

namespace rover {

Define_Module(UdpPubTransInfo);

UdpPubTransInfo::UdpPubTransInfo() {
    // TODO Auto-generated constructor stub

}

UdpPubTransInfo::~UdpPubTransInfo() {
    // TODO Auto-generated destructor stub
}



void UdpPubTransInfo::processStart()
{
    socket.setOutputGate(gate("socketOut"));
    const char *localAddress = par("localAddress");
    socket.bind(*localAddress ? L3AddressResolver().resolve(localAddress) : L3Address(), localPort);
    setSocketOptions();

    const char *destAddrs = par("destAddresses");
    cStringTokenizer tokenizer(destAddrs);
    const char *token;

    while ((token = tokenizer.nextToken()) != nullptr) {
        destAddressStr.push_back(token);
        if (strstr(token, "Broadcast") != nullptr){
           destAddresses.push_back(inet::Ipv4Address::ALLONES_ADDRESS);
        } else {
            L3Address result;
            L3AddressResolver().tryResolve(token, result);
            if (result.isUnspecified())
                EV_ERROR << "cannot resolve destination address: " << token << std::endl;
            destAddresses.push_back(result);
        }
    }

    if (!destAddresses.empty()) {
        selfMsg->setKind(SEND);
        processSend();
    }
    else {
        if (stopTime >= SIMTIME_ZERO) {
            selfMsg->setKind(STOP);
            scheduleAt(stopTime, selfMsg);
        }
    }
}

} /* namespace rover */
