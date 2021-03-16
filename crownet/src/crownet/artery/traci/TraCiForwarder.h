/*
 * TraCiForwarder.h
 *
 *  Created on: Mar 2, 2021
 *      Author: vm-sts
 */

#pragma once

namespace tcpip {
    class Storage;
}

namespace crownet {


class TraCiForwarder {
public:
    virtual ~TraCiForwarder() = default;
    virtual void forward(tcpip::Storage& msgIn,  tcpip::Storage& msgResponse) = 0;
};

}// namespace crownet
