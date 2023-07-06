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
#include "crownet/dcd/regularGrid/MapCellAggregationAlgorithms.h"
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


class DPMM_CellAggregationAlgTest : public MapTest {
 public:
  using Entry = IEntry<IntIdentifer, omnetpp::simtime_t>;
  DPMM_CellAggregationAlgTest(): MapTest(){}


  void SetUp() override {
      mapFull = dcdFactory->create_shared_ptr(3);
      clearMap();
  }

  void clearMap() {
      clear_map(mapFull, 0.0);
  }


 protected:
  std::shared_ptr<RegularDcdMap> mapFull;
};

TEST_F(DPMM_CellAggregationAlgTest, ymf_one_element) {

    setSimTime(20.0);
    GridCellID cell{5, 5};
    update(mapFull, cell, 100, 5, simTime() , 100);
    auto entry = mapFull->getCell(cell).getData()[100];
    YmfVisitor v {simTime()};
    mapFull->computeValues(&v);
    auto selected_value = mapFull->getCell(cell).val();

    expect_different_object_but_same_content(entry, selected_value);  //EXPECT_EQ(entry, selected_value);

    EXPECT_EQ(selected_value->getSelectionRank(), 0.0);
    EXPECT_EQ(selected_value->getCount(), 5);
}

TEST_F(DPMM_CellAggregationAlgTest, ymfDist_one_element_must_have_rank_1) {

    setSimTime(20.0);
    GridCellID cell{5, 5};
    update(mapFull, cell, 100, 5, simTime() , 100);
    auto entry = mapFull->getCell(cell).getData()[100];
    YmfPlusDistVisitor v {0.5, simTime()};
    mapFull->computeValues(&v);
    auto selected_value = mapFull->getCell(cell).val();

    expect_different_object_but_same_content(entry, selected_value); //EXPECT_EQ(entry, selected_value);

    EXPECT_EQ(selected_value->getSelectionRank(), 1.0);
    EXPECT_EQ(selected_value->getCount(), 5);
}


TEST_F(DPMM_CellAggregationAlgTest, ymfDist_two_elements) {

    setSimTime(20.0);
    GridCellID cell{5, 5};
    update(mapFull, cell, 100, 5, simTime() , 110); // older and farer away entry
    setSimTime(30.0);
    update(mapFull, cell, 101, 5, simTime() , 100); // younger and closer (<-- select this one)
    auto newerEntry = mapFull->getCell(cell).getData()[101];
    auto olderEntry = mapFull->getCell(cell).getData()[100];
    YmfPlusDistVisitor v {0.5, simTime()};
    mapFull->computeValues(&v);
    auto selected_value = mapFull->getCell(cell).val();

    expect_different_object_but_same_content(newerEntry, selected_value); // EXPECT_EQ(newerEntry, selected_value);

    EXPECT_EQ(olderEntry->getSelectionRank(), 0.5*1.0 + 0.5*(110./210.));
    EXPECT_EQ(selected_value->getSelectionRank(), 0.0 + 0.5*(100./210.));
    EXPECT_EQ(selected_value->getCount(), 5);
}


TEST_F(DPMM_CellAggregationAlgTest, ymfPlusDistStep_one_element_must_have_rank_1_a) {
    // with distance greater than stepDist

    setSimTime(20.0);
    GridCellID cell{5, 5};
    update(mapFull, cell, 100, 5, simTime() , 100);
    auto entry = mapFull->getCell(cell).getData()[100];
    YmfPlusDistStepVisitor v {0.5, simTime(), 50};
    mapFull->computeValues(&v);
    auto selected_value = mapFull->getCell(cell).val();

    expect_different_object_but_same_content(entry, selected_value); //EXPECT_EQ(entry, selected_value);

    EXPECT_EQ(selected_value->getSelectionRank(), 1.0);
    EXPECT_EQ(selected_value->getCount(), 5);
}

TEST_F(DPMM_CellAggregationAlgTest, ymfPlusDistStep_one_element_must_have_rank_1_b) {
    // with distance smaller than stepDist

    setSimTime(20.0);
    GridCellID cell{5, 5};
    update(mapFull, cell, 100, 5, simTime() , 20);
    auto entry = mapFull->getCell(cell).getData()[100];
    YmfPlusDistStepVisitor v {0.5, simTime(), 50};
    mapFull->computeValues(&v);
    auto selected_value = mapFull->getCell(cell).val();

    expect_different_object_but_same_content(entry, selected_value); //EXPECT_EQ(entry, selected_value);

    EXPECT_EQ(selected_value->getSelectionRank(), 1.0);
    EXPECT_EQ(selected_value->getCount(), 5);
}


TEST_F(DPMM_CellAggregationAlgTest, ymfPlusDistStep_two_element_with_rank_aggrement_differ_in_both_ranks) {
    // with distance greater than stepDist for both

    setSimTime(20.0);
    GridCellID cell{5, 5};
    update(mapFull, cell, 100, 5, simTime() , 110); // older and farer away entry
    setSimTime(30.0);
    update(mapFull, cell, 101, 5, simTime() , 100); // younger and closer (<-- select this one)
    auto newerEntry = mapFull->getCell(cell).getData()[101];
    auto olderEntry = mapFull->getCell(cell).getData()[100];
    YmfPlusDistStepVisitor v {0.5, simTime(), 50};
    mapFull->computeValues(&v);
    auto selected_value = mapFull->getCell(cell).val();

    expect_different_object_but_same_content(newerEntry, selected_value); // EXPECT_EQ(newerEntry, selected_value);

    EXPECT_EQ(olderEntry->getSelectionRank(), 0.5*1.0 + 0.5*(110./210.));
    EXPECT_EQ(selected_value->getSelectionRank(), 0.5*0. + 0.5*(100./210.));
    EXPECT_EQ(selected_value->getCount(), 5);
}

TEST_F(DPMM_CellAggregationAlgTest, ymfPlusDistStep_two_element_with_rank_aggrement_below_step_dist_must_have_rank_dot_25_and_dot_75) {
    /**
     * with distance smaller than stepDist for both --> only time will define rank
     *
     * In this case the second value will reach a rank of at most 0.5 given the alpha
     * value is set to 0.5. Because  both values are ranked the same for the distance
     * (same -> rank = 0.0, both are equally good based on the distance rank) and only
     * the age is used. For the age the second value got the worst rank = 1.0, combined
     * with alpha this will lead to a rank of 0.5
     */

    setSimTime(20.0);
    GridCellID cell{5, 5};
    update(mapFull, cell, 100, 5, simTime() , 110); // older and farer away entry
    setSimTime(30.0);
    update(mapFull, cell, 101, 5, simTime() , 100); // younger and closer (<-- select this one)
    auto newerEntry = mapFull->getCell(cell).getData()[101];
    auto olderEntry = mapFull->getCell(cell).getData()[100];
    YmfPlusDistStepVisitor v {0.5, simTime(), 300};
    mapFull->computeValues(&v);
    auto selected_value = mapFull->getCell(cell).val();

    expect_different_object_but_same_content(newerEntry, selected_value); //EXPECT_EQ(newerEntry, selected_value);

    EXPECT_EQ(olderEntry->getSelectionRank(), 0.5*1.0 + 0.5*(300./600.));
    EXPECT_EQ(selected_value->getSelectionRank(), 0.5*0.0 + 0.5*(300./600.) );
    EXPECT_EQ(selected_value->getCount(), 5);
}


TEST_F(DPMM_CellAggregationAlgTest, ymfPlusDistStep_two_element_with_rank_aggrement_above_step_dist_with_SAME_dist_must_have_rank_dot_25_and_dot_75) {
    // with SAME distance greater than stepDist -->  only time will define rank

    setSimTime(20.0);
    GridCellID cell{5, 5};
    update(mapFull, cell, 100, 5, simTime() , 300); // older and same distance above setpDist
    setSimTime(30.0);
    update(mapFull, cell, 101, 5, simTime() , 300); // younger and and same distance above setpDist (<-- select this one)
    auto newerEntry = mapFull->getCell(cell).getData()[101];
    auto olderEntry = mapFull->getCell(cell).getData()[100];
    YmfPlusDistStepVisitor v {0.5, simTime(), 100};
    mapFull->computeValues(&v);
    auto selected_value = mapFull->getCell(cell).val();

    expect_different_object_but_same_content(newerEntry, selected_value); //EXPECT_EQ(newerEntry, selected_value);

    EXPECT_EQ(olderEntry->getSelectionRank(), 0.75);
    EXPECT_EQ(selected_value->getSelectionRank(), 0.25);
    EXPECT_EQ(selected_value->getCount(), 5);
}

TEST_F(DPMM_CellAggregationAlgTest, ymfPlusDistStep_two_element_with_rank_aggrement_above_step_dist_with_DIFFERENT_dist) {
    // with SAME distance greater than stepDist -->  only time will define rank

    setSimTime(20.0);
    GridCellID cell{5, 5};
    update(mapFull, cell, 100, 5, simTime() , 330); // older and farther away, above setpDist
    setSimTime(30.0);
    update(mapFull, cell, 101, 5, simTime() , 300); // younger and closer, distance above setpDist (<-- select this one)
    auto newerEntry = mapFull->getCell(cell).getData()[101];
    auto olderEntry = mapFull->getCell(cell).getData()[100];
    YmfPlusDistStepVisitor v {0.5, simTime(), 100};
    mapFull->computeValues(&v);
    auto selected_value = mapFull->getCell(cell).val();

    expect_different_object_but_same_content(newerEntry, selected_value);

    EXPECT_EQ(olderEntry->getSelectionRank(), 0.5*1.0 +0.5*(330./630.));
    EXPECT_EQ(selected_value->getSelectionRank(), 0.0*1.0 +0.5*(300./630.));
    EXPECT_EQ(selected_value->getCount(), 5);
}

TEST_F(DPMM_CellAggregationAlgTest, ymfPlusDistStep_two_element_with_rank_disaggrement_with_same_relative_difference) {
    // with SAME distance greater than stepDist -->  only time will define rank

    setSimTime(20.0);
    GridCellID cell{5, 5};
    update(mapFull, cell, 100, 5, simTime() , 300); // older and closer, above setpDist
    setSimTime(30.0);
    update(mapFull, cell, 101, 5, simTime() , 330); // younger and farer away, distance above setpDist
    auto newerEntry = mapFull->getCell(cell).getData()[101];
    auto olderEntry = mapFull->getCell(cell).getData()[100];
    YmfPlusDistStepVisitor v {0.5, simTime(), 100};
    mapFull->computeValues(&v);
    auto selected_value = mapFull->getCell(cell).val();
    EXPECT_EQ(olderEntry->getSelectionRank(), 0.5*1.0 +0.5*(300./630.));
    EXPECT_EQ(selected_value->getSelectionRank(), 0.0*1.0 +0.5*(330./630.));
}

TEST_F(DPMM_CellAggregationAlgTest, ymfPlusDistStep_two_element_with_rank_disaggrement_choose_older_value_due_to_closeness) {
    // with SAME distance greater than stepDist -->  only time will define rank

    std::stringstream hash_stream;
    std::string hash_str;

    for (double stepDist = 0; stepDist <= 200; stepDist+=50) {
        for (double alpha = 0; alpha <= 1; alpha+=.25){
            std::stringstream n;
            n << "rank_alpha" << alpha << "_stepDist" <<  stepDist << ".csv";
            std::string fname = n.str();


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

            ConstLambdaVisitor<RegularDcdMap::cell_t> visitor {func};


            mapFull = dcdFactory->create_shared_ptr(3);
            YmfPlusDistStepVisitor v {alpha, simTime(), stepDist};
            GridCellID cell{5, 5};

            int id = 0;
            for(int dist=5; dist<10*50; dist+=5){
                for(int time=5; time <10*50; time+=5){
                    update(mapFull, cell, id, 999, time, dist);
                    id++;
                    setSimTime(time);
                }
            }
            mapFull->computeValues(&v);
            mapFull->visitCells(visitor);

            hash_stream << s.str();

            fs::path dir = fs::absolute(__FILE__).parent_path();
            std::ofstream myfile;
            myfile.open ((dir / fname).c_str());
            myfile << s.str();
            myfile.close();
            mapFull.reset();
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
      EXPECT_STREQ(hexDigest.str().c_str(), "7549DF831B36A50D87CF95130E2292D7A0638488");

}


