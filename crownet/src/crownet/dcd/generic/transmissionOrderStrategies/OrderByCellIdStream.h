/*
 * OrderByCellIdStream.h
 *
 *  Created on: Aug 28, 2023
 *      Author: vm-sts
 */

#ifndef CROWNET_DCD_GENERIC_TRANSMISSIONORDERSTRATEGIES_ORDERBYCELLIDSTREAM_H_
#define CROWNET_DCD_GENERIC_TRANSMISSIONORDERSTRATEGIES_ORDERBYCELLIDSTREAM_H_


#include <algorithm>

#include "ICellIdStream.h"


namespace crownet {

template <typename C, typename N, typename T>
class DcDMap;

template <typename C, typename N, typename T>
class Cell;

template <typename C, typename N, typename T>
class OrderdCellIdVectorProvider;


template <typename C, typename N, typename T>
class OrderByCellIdStream : public ICellIdStream<C, N, T> {
public:
    using cell_key_t = C;
    using node_key_t = N;
    using time_t = T;
    using dMapPtr = DcDMap<C, N, T>*;
    using Cell = Cell<C, N, T>;
    using OrderVecProviderPtr = std::shared_ptr<OrderdCellIdVectorProvider<C, N, T>>;


    OrderByCellIdStream(OrderVecProviderPtr vecProvider):ICellIdStream<C, N, T>(), vecProvider(vecProvider){}

    virtual void addNew(const cell_key_t& cellId, const time_t& time) override {/* nothing do to. Order defined during update*/};

    virtual void setMap(dMapPtr map) override {this->map = map;}

    virtual const bool hasNext(const time_t& now) override;

    virtual const cell_key_t nextCellId(const time_t& now) override;
    virtual Cell& nextCell(const time_t& now) override;
    virtual const int size(const time_t& now) override;
    virtual std::vector<cell_key_t> getNumCellsOrLess(const time_t& now, const int numCells) override;


    virtual void update(const time_t& time) override {
        if (lastUpdated < time){
            // update
            cellOrder.clear();
            cellOrder = vecProvider->getCellOrder(map);


            head = 0;
            nextLoopDone.clear();
            lastUpdated = time;
        }
    }
protected:
    virtual bool isValidAndCanBeSent(const time_t& time, const cell_key_t& cell_key) const;


protected:
    dMapPtr map;
    std::vector<cell_key_t> cellOrder;
    OrderVecProviderPtr vecProvider;
    time_t lastUpdated;
    int head;
    std::set<time_t> nextLoopDone;
};

#include "OrderByCellIdStream.tcc"
} // namespace


#endif /* CROWNET_DCD_GENERIC_TRANSMISSIONORDERSTRATEGIES_ORDERBYCELLIDSTREAM_H_ */
