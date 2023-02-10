/*
 * AppRxInfoProvider.h
 *
 *  Created on: Feb 7, 2023
 *      Author: vm-sts
 */

#ifndef CROWNET_APPLICATIONS_COMMON_INFO_APPRXINFOPROVIDER_H_
#define CROWNET_APPLICATIONS_COMMON_INFO_APPRXINFOPROVIDER_H_

namespace crownet {


class AppRxInfo;

class AppRxInfoProvider {
public:
    virtual ~AppRxInfoProvider() = default;
    virtual const AppRxInfo* getAppRxInfo(const int id = -1) const = 0;
    virtual const int getNumberOfSenders() const = 0;
};
}


#endif /* CROWNET_APPLICATIONS_COMMON_INFO_APPRXINFOPROVIDER_H_ */
