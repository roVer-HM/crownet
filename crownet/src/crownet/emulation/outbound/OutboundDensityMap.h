/*
 * OutboundDensitzMap.h
 *
 *  Created on: Jul 14, 2021
 *      Author: matthias
 */

#ifndef CROWNET_EMULATION_OUTBOUND_OUTBOUNDDENSITYMAP_H_
#define CROWNET_EMULATION_OUTBOUND_OUTBOUNDDENSITYMAP_H_

#undef NaN

#include "../definitions.pb.h"
#include <map>

using com::example::peopledensitymeasurementprototype::model::proto::LocationMessageWrapper;
using com::example::peopledensitymeasurementprototype::model::proto::SingleLocationData;
using com::example::peopledensitymeasurementprototype::model::proto::DensityMap;

class OutboundDensityMap {
public:
    OutboundDensityMap();
    virtual ~OutboundDensityMap();

    virtual void addSingle(SingleLocationData const * const data);
    virtual void reset();
    virtual void writeToDensityMap(DensityMap* map);

private:
    std::map<int, SingleLocationData> nodeDensityMap;
};

#endif /* CROWNET_EMULATION_OUTBOUND_OUTBOUNDDENSITYMAP_H_ */
