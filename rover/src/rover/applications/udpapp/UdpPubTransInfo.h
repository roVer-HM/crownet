/*
 * UdpPubTransInfo.h
 *
 *  Created on: Nov 19, 2019
 *      Author: stsc
 */

#pragma once

#include "inet/applications/udpapp/UdpBasicApp.h"

namespace rover {

class UdpPubTransInfo : public inet::UdpBasicApp {
public:
    UdpPubTransInfo();
    virtual ~UdpPubTransInfo();

protected:
    void processStart() override;

};

} /* namespace rover */

