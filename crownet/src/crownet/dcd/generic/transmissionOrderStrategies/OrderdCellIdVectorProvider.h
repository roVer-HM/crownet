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

        for(const auto &e : map->valid()){
            float dist = std::abs(e.first.x() - cell.x()) + std::abs(e.first.y() - cell.y());
            CmpKey k {e.second.lastSent(), dist };
            vec.push_back(std::make_pair(e.first, k));
        }

        std::stable_sort(
                vec.begin(),
                vec.end(),
                [this](const std::pair<CellId, CmpKey>& a, const std::pair<CellId, CmpKey>& b) -> bool {
                    if (a.second.lastSent == b.second.lastSent){
                        if(this->distanceAscendingOrder){
                            return a.second.distance < b.second.distance;
                        } else {
                            return a.second.distance > b.second.distance;
                        }
                    }
                    if(this->lastSentAscendingOrder){
                        return a.second.lastSent < b.second.lastSent;
                    } else {
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
