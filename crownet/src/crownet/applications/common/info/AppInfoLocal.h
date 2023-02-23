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
    AppInfoLocal(const AppInfoLocal& other): AppInfoLocal_Base(other){};

    virtual AppInfoLocal_Base *dup() const override {
        return new AppInfoLocal(*this);
    }


    const uint16_t nextSequenceNumber();

};

} /* namespace crownet */

#endif /* CROWNET_APPLICATIONS_COMMON_INFO_APPINFOLOCAL_H_ */
