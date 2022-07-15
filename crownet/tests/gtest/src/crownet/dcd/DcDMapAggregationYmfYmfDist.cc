/*
 * DcDMapTest.cc
 *
 *  Created on: Nov 24, 2020
 *      Author: sts
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
#include <iostream>
#include <fstream>
#include <boost/predef/other/endian.h>
#include <boost/endian/conversion.hpp>
#include <boost/algorithm/hex.hpp>
#include <boost/range/iterator_range_core.hpp>
#include <boost/filesystem.hpp>
#include <boost/uuid/name_generator_sha1.hpp>
namespace fs = boost::filesystem;

using namespace crownet;

namespace {

DcdFactoryProvider f = DcdFactoryProvider(
        inet::Coord(.0, .0),
        inet::Coord(10.0, 10.0),
        1.0
);
RegularGridInfo  grid = f.grid;
std::shared_ptr<RegularDcdMapFactory> dcdFactory = f.dcdFactory;

}

class RegularDcDMapAggregationTest : public BaseOppTest {
 public:
  using Entry = IEntry<IntIdentifer, omnetpp::simtime_t>;
  RegularDcDMapAggregationTest()
      :  mapFull(dcdFactory->create(3)) {}

  void update(RegularDcdMap& map, int x, int y, int id, int count, omnetpp::simtime_t t, double dist) {
    EntryDist distEntry = {0., dist, 0.};
    auto e = std::make_shared<Entry>(count, t, t, IntIdentifer(id), distEntry);
    map.setEntry(GridCellID(x, y), std::move(e));
  }
  void update(RegularDcdMap& map, GridCellID cell, int id, int count, omnetpp::simtime_t t, double dist) {
    EntryDist distEntry = {0., dist, 0.};
    auto e = std::make_shared<Entry>(count, t, t, IntIdentifer(id), distEntry);
    map.setEntry(cell, std::move(e));
  }

  void SetUp() override {
    setSimTime(0.0);
    clearMap();
  }

  void clearMap() {
      ClearVisitor v{simTime()};
      mapFull.visitCells(v);
  }

 protected:
  RegularDcdMap mapFull;
};

TEST_F(RegularDcDMapAggregationTest, ymf_one_element) {
    clearMap();
    setSimTime(20.0);
    GridCellID cell{5, 5};
    update(mapFull, cell, 100, 5, simTime() , 100);
    auto entry = mapFull.getCell(cell).getData()[100];
    YmfVisitor v {simTime()};
    mapFull.computeValues(&v);
    auto selected_value = mapFull.getCell(cell).val();

    EXPECT_EQ(entry, selected_value);

    EXPECT_EQ(selected_value->getSelectionRank(), 0.0);
    EXPECT_EQ(selected_value->getCount(), 5);
}

TEST_F(RegularDcDMapAggregationTest, ymfDist_one_element_must_have_rank_0) {
    clearMap();
    setSimTime(20.0);
    GridCellID cell{5, 5};
    update(mapFull, cell, 100, 5, simTime() , 100);
    auto entry = mapFull.getCell(cell).getData()[100];
    YmfPlusDistVisitor v {0.5, simTime()};
    mapFull.computeValues(&v);
    auto selected_value = mapFull.getCell(cell).val();

    EXPECT_EQ(entry, selected_value);

    EXPECT_EQ(selected_value->getSelectionRank(), 0.0);
    EXPECT_EQ(selected_value->getCount(), 5);
}


TEST_F(RegularDcDMapAggregationTest, ymfDist_two_element_must_have_rank_0_and_1) {
    clearMap();
    setSimTime(20.0);
    GridCellID cell{5, 5};
    update(mapFull, cell, 100, 5, simTime() , 110); // older and farer away entry
    setSimTime(30.0);
    update(mapFull, cell, 101, 5, simTime() , 100); // younger and closer (<-- select this one)
    auto newerEntry = mapFull.getCell(cell).getData()[101];
    auto olderEntry = mapFull.getCell(cell).getData()[100];
    YmfPlusDistVisitor v {0.5, simTime()};
    mapFull.computeValues(&v);
    auto selected_value = mapFull.getCell(cell).val();

    EXPECT_EQ(newerEntry, selected_value);

    EXPECT_EQ(olderEntry->getSelectionRank(), 1.0);
    EXPECT_EQ(selected_value->getSelectionRank(), 0.0);
    EXPECT_EQ(selected_value->getCount(), 5);
}


TEST_F(RegularDcDMapAggregationTest, ymfDistStep_one_element_must_have_rank_0_a) {
    // with distance greater than stepDist
    clearMap();
    setSimTime(20.0);
    GridCellID cell{5, 5};
    update(mapFull, cell, 100, 5, simTime() , 100);
    auto entry = mapFull.getCell(cell).getData()[100];
    YmfPlusDistStepVisitor v {0.5, simTime(), 50};
    mapFull.computeValues(&v);
    auto selected_value = mapFull.getCell(cell).val();

    EXPECT_EQ(entry, selected_value);

    EXPECT_EQ(selected_value->getSelectionRank(), 0.0);
    EXPECT_EQ(selected_value->getCount(), 5);
}

TEST_F(RegularDcDMapAggregationTest, ymfDistStep_one_element_must_have_rank_0_b) {
    // with distance smaller than stepDist
    clearMap();
    setSimTime(20.0);
    GridCellID cell{5, 5};
    update(mapFull, cell, 100, 5, simTime() , 20);
    auto entry = mapFull.getCell(cell).getData()[100];
    YmfPlusDistStepVisitor v {0.5, simTime(), 50};
    mapFull.computeValues(&v);
    auto selected_value = mapFull.getCell(cell).val();

    EXPECT_EQ(entry, selected_value);

    EXPECT_EQ(selected_value->getSelectionRank(), 0.0);
    EXPECT_EQ(selected_value->getCount(), 5);
}


TEST_F(RegularDcDMapAggregationTest, ymfDistStep_two_element_with_rank_aggrement_differ_in_both_ranks_must_have_rank_0_and_1) {
    // with distance greater than stepDist for both
    clearMap();
    setSimTime(20.0);
    GridCellID cell{5, 5};
    update(mapFull, cell, 100, 5, simTime() , 110); // older and farer away entry
    setSimTime(30.0);
    update(mapFull, cell, 101, 5, simTime() , 100); // younger and closer (<-- select this one)
    auto newerEntry = mapFull.getCell(cell).getData()[101];
    auto olderEntry = mapFull.getCell(cell).getData()[100];
    YmfPlusDistStepVisitor v {0.5, simTime(), 50};
    mapFull.computeValues(&v);
    auto selected_value = mapFull.getCell(cell).val();

    EXPECT_EQ(newerEntry, selected_value);

    EXPECT_EQ(olderEntry->getSelectionRank(), 1.0);
    EXPECT_EQ(selected_value->getSelectionRank(), 0.0);
    EXPECT_EQ(selected_value->getCount(), 5);
}

TEST_F(RegularDcDMapAggregationTest, ymfDistStep_two_element_with_rank_aggrement_below_step_dist_must_have_rank_0_and_dot_5) {
    /**
     * with distance smaller than stepDist for both --> only time will define rank
     *
     * In this case the second value will reach a rank of at most 0.5 given the alpha
     * value is set to 0.5. Because  both values are ranked the same for the distance
     * (same -> rank = 0.0, both are equally good based on the distance rank) and only
     * the age is used. For the age the second value got the worst rank = 1.0, combined
     * with alpha this will lead to a rank of 0.5
     */
    clearMap();
    setSimTime(20.0);
    GridCellID cell{5, 5};
    update(mapFull, cell, 100, 5, simTime() , 110); // older and farer away entry
    setSimTime(30.0);
    update(mapFull, cell, 101, 5, simTime() , 100); // younger and closer (<-- select this one)
    auto newerEntry = mapFull.getCell(cell).getData()[101];
    auto olderEntry = mapFull.getCell(cell).getData()[100];
    YmfPlusDistStepVisitor v {0.5, simTime(), 300};
    mapFull.computeValues(&v);
    auto selected_value = mapFull.getCell(cell).val();

    EXPECT_EQ(newerEntry, selected_value);

    EXPECT_EQ(olderEntry->getSelectionRank(), .5);
    EXPECT_EQ(selected_value->getSelectionRank(), 0.0);
    EXPECT_EQ(selected_value->getCount(), 5);
}


TEST_F(RegularDcDMapAggregationTest, ymfDistStep_two_element_with_rank_aggrement_above_step_dist_with_SAME_dist_must_have_rank_0_and_dot_5) {
    // with SAME distance greater than stepDist -->  only time will define rank
    clearMap();
    setSimTime(20.0);
    GridCellID cell{5, 5};
    update(mapFull, cell, 100, 5, simTime() , 300); // older and same distance above setpDist
    setSimTime(30.0);
    update(mapFull, cell, 101, 5, simTime() , 300); // younger and and same distance above setpDist (<-- select this one)
    auto newerEntry = mapFull.getCell(cell).getData()[101];
    auto olderEntry = mapFull.getCell(cell).getData()[100];
    YmfPlusDistStepVisitor v {0.5, simTime(), 100};
    mapFull.computeValues(&v);
    auto selected_value = mapFull.getCell(cell).val();

    EXPECT_EQ(newerEntry, selected_value);

    EXPECT_EQ(olderEntry->getSelectionRank(), 0.5);
    EXPECT_EQ(selected_value->getSelectionRank(), 0.0);
    EXPECT_EQ(selected_value->getCount(), 5);
}

TEST_F(RegularDcDMapAggregationTest, ymfDistStep_two_element_with_rank_aggrement_above_step_dist_with_DIFFERENT_dist_must_have_rank_0_and_1) {
    // with SAME distance greater than stepDist -->  only time will define rank
    clearMap();
    setSimTime(20.0);
    GridCellID cell{5, 5};
    update(mapFull, cell, 100, 5, simTime() , 330); // older and farther away, above setpDist
    setSimTime(30.0);
    update(mapFull, cell, 101, 5, simTime() , 300); // younger and closer, distance above setpDist (<-- select this one)
    auto newerEntry = mapFull.getCell(cell).getData()[101];
    auto olderEntry = mapFull.getCell(cell).getData()[100];
    YmfPlusDistStepVisitor v {0.5, simTime(), 100};
    mapFull.computeValues(&v);
    auto selected_value = mapFull.getCell(cell).val();

    EXPECT_EQ(newerEntry, selected_value);

    EXPECT_EQ(olderEntry->getSelectionRank(), 1.0);
    EXPECT_EQ(selected_value->getSelectionRank(), 0.0);
    EXPECT_EQ(selected_value->getCount(), 5);
}

TEST_F(RegularDcDMapAggregationTest, ymfDistStep_two_element_with_rank_disaggrement_with_same_relative_difference_must_have_rank_dot5_dot5) {
    // with SAME distance greater than stepDist -->  only time will define rank
    clearMap();
    setSimTime(20.0);
    GridCellID cell{5, 5};
    update(mapFull, cell, 100, 5, simTime() , 300); // older and closer, above setpDist
    setSimTime(30.0);
    update(mapFull, cell, 101, 5, simTime() , 330); // younger and farer away, distance above setpDist
    auto newerEntry = mapFull.getCell(cell).getData()[101];
    auto olderEntry = mapFull.getCell(cell).getData()[100];
    YmfPlusDistStepVisitor v {0.5, simTime(), 100};
    mapFull.computeValues(&v);
    auto selected_value = mapFull.getCell(cell).val();
    EXPECT_EQ(olderEntry->getSelectionRank(), 0.5);
    EXPECT_EQ(selected_value->getSelectionRank(), 0.5);
}

TEST_F(RegularDcDMapAggregationTest, ymfDistStep_two_element_with_rank_disaggrement_choose_older_value_due_to_closeness) {
    // with SAME distance greater than stepDist -->  only time will define rank

    std::stringstream hash_stream;
    std::string hash_str;

    for (double stepDist = 0; stepDist < 200; stepDist+=50) {
        for (double alpha = 0; alpha <= 1; alpha+=.25){
            std::stringstream n;
            n << "rank_alpha" << alpha << "_stepDist" <<  stepDist << ".csv";
            std::string fname = n.str();
//            std::cout << fname << std::endl;

            std::stringstream s;
            s << "ID; t_g; age; dist; rank" << std::endl;
            auto func =  [&s](const RegularDcdMap::cell_t& cell){
                for (const auto &entry : cell.getData()){
                    s << entry.second->getSource() << "; " \
                            << entry.second->getMeasureTime() << "; " \
                            << (simTime() - entry.second->getMeasureTime()) << "; " \
                            << entry.second->getEntryDist().sourceEntry << "; " \
                            << entry.second->getSelectionRank() \
                            << std::endl;
                }
            };
            ConstLambdaVisitor<RegularDcdMap::cell_t> visitor = ConstLambdaVisitor<RegularDcdMap::cell_t>( func );

            mapFull = dcdFactory->create(3);
            YmfPlusDistStepVisitor v {alpha, simTime(), stepDist};
            GridCellID cell{5, 5};
            clearMap();
            int id = 0;
            for(int dist=5; dist<10*50; dist+=5){
                for(int time=5; time <10*50; time+=5){
                    update(mapFull, cell, id, 999, time, dist);
                    id++;
                    setSimTime(time);
                }
            }
            mapFull.computeValues(&v);
            mapFull.visitCells(visitor);

            hash_stream << s.str();

            fs::path dir = fs::absolute(__FILE__).parent_path();
            std::ofstream myfile;
            myfile.open ((dir / fname).c_str());
            myfile << s.str();
            myfile.close();
        }
    }
    hash_str = hash_stream.str();
    boost::uuids::detail::sha1 sha1;
    sha1.process_bytes(hash_str.data(), hash_str.size());
    boost::uuids::detail::sha1::digest_type hash;
    sha1.get_digest(hash);
    #ifdef  BOOST_ENDIAN_BIG_BYTE
      for (unsigned i = 0; i < sizeof hash / sizeof hash[0]; ++i)
        boost::endian::endian_reverse_inplace(hash[i]);
    #endif
      std::stringstream hexDigest;
      boost::algorithm::hex(boost::make_iterator_range(
            reinterpret_cast<const char*>(hash),
            reinterpret_cast<const char*>(hash) + sizeof hash),
            std::ostream_iterator<char>(hexDigest));
      EXPECT_STREQ(hexDigest.str().c_str(), "BD8C172D5ED3351FC09A61D48E7AAD573BC5F7BA");
}

