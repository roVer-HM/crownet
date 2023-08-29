

#pragma once

#include <set>
#include <vector>

#include "ICellIdStream.h"

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
    virtual void update(const time_t& time) override;

    virtual void setMap(dMapPtr map) override {this->map = map;}

    virtual const bool hasNext(const time_t& now) override;

    virtual const cell_key_t nextCellId(const time_t& now) override;
    virtual Cell& nextCell(const time_t& now) override;
    virtual const int size(const time_t& now)  override;
    virtual std::vector<cell_key_t> getNumCellsOrLess(const time_t& now, const int numCells) override;





protected:
    /**
     *  check if the cell is a valid cell and if it already was sent at the given time.
     */
    virtual bool isValidAndCanBeSent(const time_t& now, const cell_key_t& cell_key) const;

protected:
    std::vector<cell_key_t> vec;
    dMapPtr map;
    int head;
    time_t lastUpdated;
    std::set<time_t> nextLoopDone;
};

#include "InsertionOrderedCellIdStream.tcc"
}
