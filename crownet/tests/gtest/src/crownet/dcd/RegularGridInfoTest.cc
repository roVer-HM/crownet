/*
 * RegularGridInfoTest.cc
 *
 *  Created on: Sep 28, 2022
 *      Author: vm-sts
 */

#include <functional>
#include <memory>
#include <string>

#include "main_test.h"
#include "crownet/crownet_testutil.h"
#include "crownet/common/Entry.h"
#include "crownet/dcd/regularGrid/RegularCell.h"
#include "crownet/dcd/regularGrid/RegularCellVisitors.h"
#include "crownet/dcd/regularGrid/RegularDcdMap.h"
#include "inet/common/geometry/common/Coord.h"

using namespace crownet;

class RegularGridInfoTest_F : public BaseOppTest {
public:
    RegularGridInfoTest_F(){
        DcdFactoryProvider provider_s = DcdFactoryProvider(
                inet::Coord(0.0, 0.0),
                inet::Coord(100.0, 100.0),
                inet::Coord(10.0, 10.0)
        );
        s = provider_s.converter;

        DcdFactoryProvider provider_r = DcdFactoryProvider(
                inet::Coord(0.0, 0.0),
                inet::Coord(100.0, 200.0),
                inet::Coord(10.0, 20.0)
        );
        r = provider_r.converter;

    }

    protected:
    std::shared_ptr<OsgCoordinateConverter> s;
    std::shared_ptr<OsgCoordinateConverter> r;
};

TEST_F(RegularGridInfoTest_F, inCellCenter){
    auto grid = s->getGridDescription();
    // contains lower and left edge but not right and upper
    EXPECT_TRUE(grid.posInCenteredCell(inet::Coord(5.0, 5.0), inet::Coord(0.0, 2.0)));
    EXPECT_TRUE(grid.posInCenteredCell(inet::Coord(5.0, 5.0), inet::Coord(2.0, 0.0)));
    EXPECT_FALSE(grid.posInCenteredCell(inet::Coord(5.0, 5.0), inet::Coord(10.0, 2.0)));
    EXPECT_FALSE(grid.posInCenteredCell(inet::Coord(5.0, 5.0), inet::Coord(2.0, 10.0)));
    // contains center
    EXPECT_TRUE(grid.posInCenteredCell(inet::Coord(5.0, 5.0), inet::Coord(5.0, 5.0)));

    grid = r->getGridDescription();
    // contains lower and left edge but not right and upper
    EXPECT_TRUE(grid.posInCenteredCell(inet::Coord(5.0, 10.0), inet::Coord(0.0, 15.0)));
    EXPECT_TRUE(grid.posInCenteredCell(inet::Coord(5.0, 5.0), inet::Coord(7.0, 0.0)));
    EXPECT_FALSE(grid.posInCenteredCell(inet::Coord(5.0, 5.0), inet::Coord(7.0, 20.0)));
    EXPECT_FALSE(grid.posInCenteredCell(inet::Coord(5.0, 5.0), inet::Coord(10.0, 15.0)));
    // contains center
    EXPECT_TRUE(grid.posInCenteredCell(inet::Coord(5.0, 10.0), inet::Coord(5.0, 10.0)));

}




