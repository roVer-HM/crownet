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

#include "RndOffsetPolynomialEntropy.h"
#include <omnetpp/regmacros.h>

namespace crownet {

Register_Class(RndOffsetPolynomialEntropy);

void RndOffsetPolynomialEntropy::initialize(cRNG* rng) {
    rnd =  std::make_shared<cUniform>(rng, getMinValue(), getMaxValue());
}

double RndOffsetPolynomialEntropy::getValue(const inet::Coord& position, const simtime_t& time, const double old_value){
    auto id = std::make_pair(position.x, position.y);
    if (rndOffset.find(id) == rndOffset.end()){
        rndOffset[id] = getRndValue();
    }
    double alpha_0 = rndOffset[id];
    double ret = 0.0;
    double t=time.dbl(); // in seconds
    for(int n=0; n < getCoefficientsArraySize(); n++){
        double alpha_n= getCoefficients(n);
        if(n==0){
            ret += alpha_n + alpha_0;
        } else if (alpha_n != 0.0){
            ret += alpha_n*pow(t, n);
        }
    }
    return ret;
}

bool RndOffsetPolynomialEntropy::selectCell(const int x, const int y, simtime_t time){
    // doubleRand() [0, 1)
    return rnd->getRNG()->doubleRand() < getCellSelectionPropability() ;
}

double RndOffsetPolynomialEntropy::getRndValue(){
    return rnd->draw();
}

void RndOffsetPolynomialEntropy::copy(const RndOffsetPolynomialEntropy& other){
    this->rnd = other.rnd;
    this->rndOffset = other.rndOffset;
}

}
