

#pragma once

#include <queue>
#include "crownet/dcd/generic/ICellIdStream.h"

namespace crownet {

template <typename C, typename N, typename T>
class DcDMap;

template <typename C, typename N, typename T>
class Cell;

template <typename C, typename N, typename T>
class InsertionOrderedCellIdStream : public ICellIdStream<C, N, T> {
public:
    using cell_key_t = C;
    using node_key_t = N;
    using time_t = T;
    using dMapPtr = DcDMap<C, N, T>*;
    using Cell = Cell<C, N, T>;

    virtual void addNew(const cell_key_t& cellId, const time_t& time) override;

    virtual void setMap(dMapPtr map) override {this->map = map;}

    virtual const bool hasNext(const time_t& now) override;

    virtual const cell_key_t nextCellId(const time_t& now) override;
    virtual Cell& nextCell(const time_t& now) override;


    virtual void update(const time_t& time) override {/*nothing*/};

protected:
    virtual void moveBack();


protected:
    std::deque<cell_key_t> queue;
    dMapPtr map;
};

#include "CellIdStream.tcc"
}
