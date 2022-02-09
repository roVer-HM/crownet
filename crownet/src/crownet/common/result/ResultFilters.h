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
#include "inet/common/INETDefs.h"

using namespace inet;

namespace crownet {

class IncidentAgeFilter : public cObjectResultFilter {
 public:
  virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t,
                             cObject *object, cObject *details) override;
};

class LastHopAgeFilter : public cObjectResultFilter {
 public:
  virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t,
                             cObject *object, cObject *details) override;
};


class RcvdHostIdFilter : public cObjectResultFilter {
 public:
  virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t,
                             cObject *object, cObject *details) override;
};


class HostIdFilter : public cObjectResultFilter {
 public:
  virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t,
                             cObject *object, cObject *details) override;
};

class RcvdSequenceIdFilter : public cObjectResultFilter {
 public:
  virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t,
                             cObject *object, cObject *details) override;
};

class SimBoundFilter : public cObjectResultFilter {
 public:
  virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t,
                             cObject *object, cObject *details) override;
};

class SimOffsetFilter : public cObjectResultFilter {
 public:
  virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t,
                             cObject *object, cObject *details) override;
};

}  // namespace crownet
