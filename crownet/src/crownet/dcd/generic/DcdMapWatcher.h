//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#pragma once

#include <omnetpp/cstlwatch.h>
#include "crownet/dcd/generic/DcdMap.h"

using namespace omnetpp;

namespace crownet {

/*
 * Display compact information about each cell in a DcdMap.
 *
 */
template <typename C, typename N, typename T>
class DcdMapWatcher : public cStdVectorWatcherBase{
public:
    virtual ~DcdMapWatcher() = default;
    DcdMapWatcher(const char *name, std::shared_ptr<DcDMap<C, N,T>> map)
        : cStdVectorWatcherBase(name), _map(map) {}

    const char *getClassName() const override {return "DcdMap";}
    virtual const char *getElemTypeName() const override;
    virtual int size() const override;
    virtual std::string at(int i) const override;
    virtual std::string atIt() const;

private:
    std::shared_ptr<DcDMap<C, N,T>> _map;
    mutable typename DcDMap<C, N,T>::map_t::iterator it;
    mutable int itPos;
};

#include "DcdMapWatcher.tcc"


}

