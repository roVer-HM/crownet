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

#include "UniformEntropy.h"
#include <omnetpp/regmacros.h>

namespace crownet {

Register_Class(UniformEntropy);

void UniformEntropy::initialize(cRNG* rng) {
    rnd =  std::make_shared<cUniform>(rng, getMinValue(), getMaxValue());
}

double UniformEntropy::getValue(const inet::Coord& position, const simtime_t& time, const double old_value){
    return rnd->draw();
}

double UniformEntropy::getRndValue(){
    return rnd->draw();
}

bool UniformEntropy::selectCell(const int x, const int y, simtime_t time){
    // doubleRand() [0, 1)
    return rnd->getRNG()->doubleRand() < getCellSelectionPropability() ;
}

void UniformEntropy::copy(const UniformEntropy& other){
    this->rnd = other.rnd;
}

}
