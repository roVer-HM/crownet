/*
 * IAppScheduler.h
 *
 *  Created on: Sep 16, 2021
 *      Author: vm-sts
 */

#ifndef CROWNET_APPLICATIONS_COMMON_SCHEDULER_IAPPSCHEDULER_H_
#define CROWNET_APPLICATIONS_COMMON_SCHEDULER_IAPPSCHEDULER_H_

#include "inet/common/INETDefs.h"

namespace crownet {

class IAppScheduler {
public:
    virtual ~IAppScheduler() = default;
    /**
     *  Called when scheduler is triggerd with some self message.
     */
    virtual void scheduleApp(inet::cMessage *message) = 0;

    /**
     * Called when scheduler receives message on configIn gate
     */
    virtual void scheduleEvent(inet::cMessage *message) = 0;

    /**
     * Called when scheduler should schedule a completely assembled packet.
     */
    virtual void schedulePacket(inet::Packet *packet) = 0;

};

}



#endif /* CROWNET_APPLICATIONS_COMMON_SCHEDULER_IAPPSCHEDULER_H_ */
