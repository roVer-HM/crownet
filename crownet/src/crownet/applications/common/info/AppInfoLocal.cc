/*
 * AppInfoLocal.cc
 *
 *  Created on: Aug 27, 2021
 *      Author: vm-sts
 */

#include "crownet/applications/common/info/AppInfoLocal.h"

namespace crownet {

Register_Class(AppInfoLocal);

const uint16_t AppInfoLocal::nextSequenceNumber(){
    uint16_t next = ++sequencenumber;
    // check sequencenumber wraping
    if (next < sequencenumber) ++sequencecycle;
    return next;
}


} /* namespace crownet */
