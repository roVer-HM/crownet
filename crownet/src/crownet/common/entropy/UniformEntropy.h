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

#include "crownet/common/entropy/entropy_m.h"
#include "omnetpp/crandom.h"
#include <memory>

namespace crownet {

class UniformEntropy :  public UniformEntropy_Base

{
public:
    virtual void initialize(cRNG* rng) override;
    virtual double getValue(inet::Coord position, simtime_t time) override;
    virtual bool selectCell(const int x, const int y, simtime_t time) override;

private:
    std::shared_ptr<cUniform> rnd;

private:
  void copy(const UniformEntropy& other);
public:
    virtual ~UniformEntropy() = default;
    UniformEntropy(const char *name=nullptr): UniformEntropy_Base(name){};
    UniformEntropy(const UniformEntropy& other) : UniformEntropy_Base(other){ copy(other);}
    virtual UniformEntropy *dup() const override {return new UniformEntropy(*this);}




};

}
