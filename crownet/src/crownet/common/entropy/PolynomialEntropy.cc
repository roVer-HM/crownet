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

#include "PolynomialEntropy.h"
#include <omnetpp/regmacros.h>

namespace crownet {

Register_Class(PolynomialEntropy);

void PolynomialEntropy::initialize(cRNG* rng) {
    rnd =  std::make_shared<cUniform>(rng, 0.0, 1.0);
}

double PolynomialEntropy::getValue(inet::Coord position, simtime_t time){
    double ret = 0.0;
    double t=time.dbl(); // in seconds
    for(int n=0; n < getCoefficientsArraySize(); n++){
        double alpha_n= getCoefficients(n);
        if(n==0){
            ret += alpha_n;
        } else if (alpha_n != 0.0){
            ret += alpha_n*pow(t, n);
        }
    }
    return ret;
}

bool PolynomialEntropy::selectCell(const int x, const int y, simtime_t time){
    // doubleRand() [0, 1)
    return rnd->getRNG()->doubleRand() < getCellSelectionPropability() ;
}

void PolynomialEntropy::copy(const PolynomialEntropy& other){
    this->rnd = other.rnd;
}

}
