/*
 * Bar.h
 *
 *  Created on: Feb 7, 2023
 *      Author: vm-sts
 */

#ifndef CROWNET_APPLICATIONS_COMMON_INFO_BAR_H_
#define CROWNET_APPLICATIONS_COMMON_INFO_BAR_H_

#include "AppRxInfo.h"
#include "AppRxInfoPerSource_m.h"

namespace crownet {

class AppRxInfoPerSource : public AppRxInfoPerSource_Base{
public:
    virtual ~AppRxInfoPerSource();
    AppRxInfoPerSource(const char *name=nullptr): AppRxInfoPerSource_Base(name){};

    virtual void computeMetrics(Packet *packetIn) override;
    virtual void calcPacketLoss();
    virtual void checkOutOfOrder();

};

} /* namespace crownet */

#endif /* CROWNET_APPLICATIONS_COMMON_INFO_BAR_H_ */
