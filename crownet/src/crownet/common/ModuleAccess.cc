/*
 * ModuleAccess.cc
 *
 *  Created on: Jan 20, 2022
 *      Author: vm-sts
 */


#include "crownet/common/ModuleAccess.h"

using namespace inet;

namespace crownet {

inline bool _hasProperty(const cModule *mod, const char *propName){
    cProperties *props = mod->getProperties();
    return props && props->getAsBool(propName);
}


cModule *findByPropertyUp(const cModule *from, const char *propName){

    for (cModule *curmod = const_cast<cModule *>(from); curmod; curmod = curmod->getParentModule()) {
        if (_hasProperty(curmod, propName))
            return curmod;
    }
    return nullptr;
}


} // namespace crownet
