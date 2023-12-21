/*
 * CellOrder.h
 *
 *  Created on: Aug 28, 2023
 *      Author: vm-sts
 */

#ifndef CROWNET_DCD_GENERIC_TRANSMISSIONORDERSTRATEGIES_ORDERBYLASTSENTANDCELLDISTANCE_H_
#define CROWNET_DCD_GENERIC_TRANSMISSIONORDERSTRATEGIES_ORDERBYLASTSENTANDCELLDISTANCE_H_

#include "crownet/dcd/generic/DcdMap.h"
#include <algorithm>

namespace crownet {


template <typename C, typename N, typename T>
class OrderdCellIdVectorProvider {
public:
    using Map = DcDMap<C, N, T>;
    using CellId = typename Map::cell_key_t;

    virtual ~OrderdCellIdVectorProvider() = default;

    virtual std::vector<CellId> getCellOrder(Map* map) = 0;

};

template <typename C, typename N, typename T>
class OrderByCellDistance: public OrderdCellIdVectorProvider<C, N, T>{
public:
    using Map = typename OrderdCellIdVectorProvider<C,N,T>::Map;
    using CellId = typename Map::cell_key_t;

    virtual ~OrderByCellDistance() = default;
    OrderByCellDistance(bool ascending = true): ascending_order(ascending){};


    virtual std::vector<CellId> getCellOrder(Map* map) override {


        std::vector<std::pair<CellId, double>> sorted_cells;
        CellId cell = map->getOwnerCell();

        for(const auto &e : map->valid()){
            float dist = std::sqrt(
                std::pow(e.first.x() - cell.x(), 2) + std::pow(e.first.y() - cell.y(), 2)
                );

            sorted_cells.push_back(std::make_pair(e.first, dist));
        }

        // sort non-descending i.e. ascending
       std::stable_sort(
           sorted_cells.begin(),
           sorted_cells.end(),
           [](const std::pair<CellId, double>& a, const std::pair<CellId, double>& b) -> bool {
               return a.second < b.second;
       });

       if (ascending_order){
           // do nothing already sorted
       } else{
           std::reverse(sorted_cells.begin(), sorted_cells.end());
       }

       std::vector<CellId> ret;
       for(const auto& e : sorted_cells){
           ret.push_back(e.first);
       }
       return ret;
    }
private:
    bool ascending_order = true;
};


template <typename C, typename N, typename T>
class OrderByLastSentAndCellDistance : public OrderdCellIdVectorProvider<C, N, T>{
public:
    using Map = typename OrderdCellIdVectorProvider<C,N,T>::Map;
    using CellId = typename Map::cell_key_t;
    virtual ~OrderByLastSentAndCellDistance() = default;
    OrderByLastSentAndCellDistance(bool lastSentAscendingOrder = true, bool distanceAscendingOrder = true)
        :OrderdCellIdVectorProvider<C, N, T>(), lastSentAscendingOrder(lastSentAscendingOrder), distanceAscendingOrder(distanceAscendingOrder)  {}

    struct CmpKey {
        T lastSent;
        float distance;
    };

    virtual std::vector<CellId> getCellOrder(Map* map) override {
        std::vector<std::pair<CellId, CmpKey>> vec;
        CellId cell = map->getOwnerCell();

         // 0,0 / 3,
        for(const auto &e : map->valid()){
            float dist = std::sqrt(
                std::pow(e.first.x() - cell.x(), 2) + std::pow(e.first.y() - cell.y(), 2)
                );
            CmpKey k {e.second.lastSent(), dist };
            vec.push_back(std::make_pair(e.first, k));
        }
        /* 
        *  Cells are sorted first by the time the cell was last sent and than, if the age is equal, based
        *  on the distance between the cell and the nodes current location.  
        */
        std::stable_sort(
                vec.begin(),
                vec.end(),
                [this](const std::pair<CellId, CmpKey>& a, const std::pair<CellId, CmpKey>& b) -> bool {
                    // comparison function object (i.e. an object that satisfies the requirements of Compare) 
                    // which returns true if the first argument shoud be ordered before the second.
                    if (a.second.lastSent == b.second.lastSent){
                        if(this->distanceAscendingOrder){
                            // ascending: sort a *before* b if a is smaller (i.e. closer) than b
                            return a.second.distance < b.second.distance;
                        } else {
                            // desending: sort a *before* b if a is bigger (i.e. farer away) than b
                            return a.second.distance > b.second.distance;
                        }
                    }

                    if(this->lastSentAscendingOrder){
                        // ascending: sort a *before* b if a is smaller (i.e. older, last sent was long ago) than b
                        return a.second.lastSent < b.second.lastSent;
                    } else {
                        // descending: sort a *before* b if a is bigger (i.e. younger, last sent was recently) than b
                        return a.second.lastSent > b.second.lastSent;
                    }
        });

        std::vector<CellId> ret;
        for(const auto& e : vec){
            ret.push_back(e.first);
        }
        return ret;

    }

private:
    bool lastSentAscendingOrder;
    bool distanceAscendingOrder;
};

} /* namespace crownet */

#endif /* CROWNET_DCD_GENERIC_TRANSMISSIONORDERSTRATEGIES_ORDERBYLASTSENTANDCELLDISTANCE_H_ */
