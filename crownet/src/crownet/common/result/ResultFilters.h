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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#include <inet/common/ResultFilters.h>
#pragma GCC diagnostic pop

using namespace inet;

namespace crownet {


/**
 * Only log signals after a given fixed interval. Use the ResultFilter as normal
 * and add the attribute 'interval_par' which points to a double value parameter of
 * the module containing the statistic.
 * This allows the configuration of different intervals for different statistics
 * and there configuration over the ini files.
 *
 * Example:
 * @statistic[posX](source="xCoord(mobilityPos(simTimeIntervalFilter(mobilityStateChanged)))"; interval_par="positionStatInterval"; record=vector?);
 * @statistic[posY](source="yCoord(mobilityPos(simTimeIntervalFilter(mobilityStateChanged)))"; interval_par="positionStatInterval"; record=vector?);
 * double positionStatInterval = default(0.4);
 *
 * Log position every 400ms.
 *
 */
class SimTimeIntervalFilter : public inet::utils::filters::cPointerResultFilter {
public:
 virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t,
                            cObject *object, cObject *details) override;
 virtual void init(Context *ctx) override;
private:
 simtime_t filterInterval = -1.0;
 simtime_t nextLogTime = 0.0;
};

class IncidentAgeFilter : public cObjectResultFilter {
 public:
  virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t,
                             cObject *object, cObject *details) override;
  using cObjectResultFilter::receiveSignal;
};

class LastHopAgeFilter : public cObjectResultFilter {
 public:
  virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t,
                             cObject *object, cObject *details) override;
  using cObjectResultFilter::receiveSignal;
};


class RcvdHostIdFilter : public cObjectResultFilter {
 public:
  virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t,
                             cObject *object, cObject *details) override;
  using cObjectResultFilter::receiveSignal;
};


class HostIdFilter : public cObjectResultFilter {
 public:
  virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t,
                             cObject *object, cObject *details) override;
  using cObjectResultFilter::receiveSignal;
};

class RcvdPerSrcJitter : public cObjectResultFilter {
 public:
  virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t,
                             cObject *object, cObject *details) override;
  using cObjectResultFilter::receiveSignal;
};


class RcvdPerSrcAvgSize : public cObjectResultFilter {
 public:
  virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t,
                             cObject *object, cObject *details) override;
  using cObjectResultFilter::receiveSignal;
};


class RcvdPerSrcCount : public cObjectResultFilter {
 public:
  virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t,
                             cObject *object, cObject *details) override;
  using cObjectResultFilter::receiveSignal;
};

class RcvdPerSrcTotalCount : public cObjectResultFilter {
 public:
  virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t,
                             cObject *object, cObject *details) override;
  using cObjectResultFilter::receiveSignal;
};

class RcvdPerSrcLossCount : public cObjectResultFilter {
 public:
  virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t,
                             cObject *object, cObject *details) override;
  using cObjectResultFilter::receiveSignal;
};


class RcvdCount : public cObjectResultFilter {
 public:
  virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t,
                             cObject *object, cObject *details) override;
  using cObjectResultFilter::receiveSignal;
};

class RcvdSrcCount : public cObjectResultFilter {
 public:
  virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t,
                             cObject *object, cObject *details) override;
  using cObjectResultFilter::receiveSignal;
};

class RcvdAvgSize : public cObjectResultFilter {
 public:
  virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t,
                             cObject *object, cObject *details) override;
  using cObjectResultFilter::receiveSignal;
};

class RcvdSequenceIdFilter : public cObjectResultFilter {
 public:
  virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t,
                             cObject *object, cObject *details) override;
  using cObjectResultFilter::receiveSignal;
};

class SimBBoxFilter : public cObjectResultFilter {
 public:
  virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t,
                             cObject *object, cObject *details) override;
  using cObjectResultFilter::receiveSignal;
};

class SimBoundFilter : public cObjectResultFilter {
 public:
  virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t,
                             cObject *object, cObject *details) override;
  using cObjectResultFilter::receiveSignal;
};

class SimOffsetFilter : public cObjectResultFilter {
 public:
  virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t,
                             cObject *object, cObject *details) override;
  using cObjectResultFilter::receiveSignal;
};

}  // namespace crownet
