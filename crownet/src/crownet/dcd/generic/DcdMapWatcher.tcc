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
#include <omnetpp/simutil.h>
#include "crownet/dcd/generic/DcdMapWatcher.h"

template <typename C, typename N, typename T>
const char* DcdMapWatcher<C, N, T>::getElemTypeName() const {
    return omnetpp::opp_typename(typeid(DcdMapWatcher<C, N, T>));
}

template <typename C, typename N, typename T>
int DcdMapWatcher<C, N, T>::size() const{
    return this->_map->getCells().size();
}

template <typename C, typename N, typename T>
std::string DcdMapWatcher<C, N, T>::at(int i) const {

    if (i==0){
       this->it = this->_map->getCells().begin();
       itPos = 0;
    } else if (i==itPos+1 && it != this->_map->getCells().end()){
        ++this->it;
        ++this->itPos;
    } else {
        this->it = this->_map->getCells().begin();
        for (int k=0; k<i && it!=this->_map->getCells().end(); k++){
            ++it;
        }
        itPos = i;
    }
    if (it==this->_map->getCells().end()){
        return std::string("out of bounds");
    }
    return atIt();
}

template <typename C, typename N, typename T>
std::string DcdMapWatcher<C, N, T>::atIt() const{
    std::stringstream out;
    out << it->first << ": " << it->second.infoCompact();
    return out.str();
}


