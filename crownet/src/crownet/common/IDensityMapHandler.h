/*
 * IDensityMap.h
 *
 *  Created on: Oct 9, 2020
 *      Author: sts
 */

#pragma once

#include <memory>
#include "crownet/common/util/FileWriter.h"

namespace crownet {

class GridCellDistance;
class OsgCoordinateConverter;
class RegularDcdMapFactory;


template <typename GRID>
class IDensityMapHandlerBase{
   public:
    virtual ~IDensityMapHandlerBase() = default;
    virtual std::shared_ptr<GRID> getMap() = 0;
};


template <typename GRID>
class IDensityMapHandler : public IDensityMapHandlerBase<GRID> {
 public:
  virtual ~IDensityMapHandler() = default;
  virtual void updateLocalMap() = 0;
  virtual void computeValues() = 0;
  // FIXME: allow global fileWriter object to pass in.
  virtual void writeMap() = 0;
  //  FIXME: make mergeMap independent from Packet (see ArteryDensityMapApp.cc)
  virtual void setMapFactory(std::shared_ptr<RegularDcdMapFactory>) = 0;
  virtual void setCoordinateConverter(std::shared_ptr<OsgCoordinateConverter> converter) = 0;
};

}  // namespace crownet
