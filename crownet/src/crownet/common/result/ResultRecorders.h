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
#include "inet/common/geometry/common/Coord.h"

using namespace inet;

namespace crownet {

class IncidentRecorder : public cResultRecorder {
 protected:
  typedef std::map<opp_string, void *> IncidentVectors;
  //  std::map<opp_string, IncidentRecord> incidentCount;
  std::map<opp_string, IncidentVectors> vectorMap;

 protected:
  virtual void subscribedTo(cResultFilter *prev) override;
  virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, bool b,
                             cObject *details) override;
  virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, intval_t l,
                             cObject *details) override;
  virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, uintval_t l,
                             cObject *details) override;
  virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, double d,
                             cObject *details) override;
  virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t,
                             const SimTime &v, cObject *details) override;
  virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t,
                             const char *s, cObject *details) override;
  virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t,
                             cObject *obj, cObject *details) override;

 public:
  IncidentRecorder() {}
  ~IncidentRecorder();
  virtual void finish(cResultFilter *prev) override;

 private:
  virtual void registerVectors(opp_string incident);
  virtual void unregisterVectors();
  static std::vector<opp_string> resultNames;
};

}  // namespace crownet
