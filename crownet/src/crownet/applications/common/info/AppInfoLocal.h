/*
 * AppInfoLocal.h
 *
 *  Created on: Aug 27, 2021
 *      Author: vm-sts
 */

#ifndef CROWNET_APPLICATIONS_COMMON_INFO_APPINFOLOCAL_H_
#define CROWNET_APPLICATIONS_COMMON_INFO_APPINFOLOCAL_H_

#include "crownet/applications/common/info/AppInfo_m.h"

namespace crownet {

class AppInfoLocal: public AppInfoLocal_Base {
public:
    virtual ~AppInfoLocal() = default;
    AppInfoLocal(): AppInfoLocal_Base() {}

//    virtual std::string str() const override;


    const uint16_t nextSequenceNumber();
    std::string packetName(const std::string& baseName) const;
    void incrSent(){ ++packetsSentCount; }
    void incrReceivd() {++packetsReceivedCount; }

};

} /* namespace crownet */

#endif /* CROWNET_APPLICATIONS_COMMON_INFO_APPINFOLOCAL_H_ */
