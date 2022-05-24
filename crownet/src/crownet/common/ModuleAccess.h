/*
 * ModuleAccess.h
 *
 *  Created on: Jan 20, 2022
 *      Author: vm-sts
 */

#pragma once

#include "inet/common/ModuleAccess.h"

using namespace inet;

namespace crownet {


bool _hasProperty(const cModule *mod, const char *propName);


/**
 * Find the node containing the given module.
 * Returns nullptr, if no containing node.
 */
cModule *findByPropertyUp(const cModule *from, const char *propName);


} // namespace crownet
