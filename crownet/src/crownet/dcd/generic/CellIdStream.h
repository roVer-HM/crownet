

#pragma once

#include <queue>
#include "crownet/dcd/generic/ICellIdStream.h"

namespace crownet {


template <typename C, typename T>
class InsertionOrderedCellIdStream : public ICellIdStream<C, T> {
public:
    using cell_key_t = C;
    using time_t = T;
    virtual void addNew(const cell_key_t& cellId, const time_t& time) override;

    virtual const cell_key_t nextCellId() override;
    virtual const cell_key_t peekFront() override;
    virtual void update(const time_t& time) override {/*nothing*/};

protected:
    std::deque<cell_key_t> queue;
};

#include "CellIdStream.tcc"
}
