/*
 * AppInfoLocal.cc
 *
 *  Created on: Aug 27, 2021
 *      Author: vm-sts
 */

#include "crownet/applications/common/info/AppInfoLocal.h"

namespace crownet {

Register_Class(AppInfoLocal);

//std::string AppInfoLocal::str() const{
//    return "foo";
//}

const uint16_t AppInfoLocal::nextSequenceNumber(){
    uint16_t next = ++sequencenumber;
    // check sequencenumber wraping
    if (next < sequencenumber) ++sequencecycle;
    return next;
}

std::string AppInfoLocal::packetName(const std::string& baseName) const{
    std::ostringstream str;
    str << baseName << "-" << nodeId << "#" << sequencenumber;
    return str.str();
}

} /* namespace crownet */
