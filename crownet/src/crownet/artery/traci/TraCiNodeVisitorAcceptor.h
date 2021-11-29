/*
 * TraciNodeVisitor.h
 *
 *  Created on: Nov 29, 2021
 *      Author: vm-sts
 */

#pragma once
#include <traci/NodeManager.h>


namespace crownet {

class ITraCiNodeVisitorAcceptor {
public:
    virtual ~ITraCiNodeVisitorAcceptor() = default;
    virtual void acceptTraciVisitor(traci::ITraciNodeVisitor* v) = 0;
};


};
