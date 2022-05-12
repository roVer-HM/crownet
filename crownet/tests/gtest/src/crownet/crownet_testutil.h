/*
 * crownet_util.h
 *
 *  Created on: Apr 14, 2022
 *      Author: vm-sts
 */

#ifndef GTEST_SRC_CROWNET_CROWNET_TESTUTIL_H_
#define GTEST_SRC_CROWNET_CROWNET_TESTUTIL_H_

#include "crownet/common/Entry.h"
#include "crownet/dcd/regularGrid/RegularCell.h"
#include "crownet/dcd/regularGrid/RegularCellVisitors.h"
#include "crownet/dcd/regularGrid/RegularDcdMap.h"

using namespace crownet;

class DcdFactoryProvider {
public:
    ~DcdFactoryProvider()=default;
    DcdFactoryProvider() : DcdFactoryProvider(inet::Coord(.0, .0), inet::Coord(10.0, 10.0),1.0){}
    DcdFactoryProvider(inet::Coord offset, inet::Coord bound, double cellSize)
        : DcdFactoryProvider(offset, bound, inet::Coord(cellSize, cellSize)) {}
    DcdFactoryProvider(inet::Coord offset, inet::Coord bound, inet::Coord cellSize){
        converter = std::make_shared<OsgCoordinateConverter>(
                offset,
                bound,
                "EPSG:32632");
        converter->setCellSize(cellSize);
        grid = converter->getGridDescription();
        dcdFactory = std::make_shared<RegularDcdMapFactory>(converter);
    }

    std::shared_ptr<OsgCoordinateConverter> converter;
    RegularGridInfo grid;
    std::shared_ptr<RegularDcdMapFactory> dcdFactory;
};



#endif /* GTEST_SRC_CROWNET_CROWNET_TESTUTIL_H_ */
