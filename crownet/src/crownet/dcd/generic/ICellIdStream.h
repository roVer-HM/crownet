
#pragma once

namespace crownet {

template <typename C, typename T>
class ICellIdStream {
public:
    using cell_key_t = C;
    using time_t = T;
    virtual ~ICellIdStream() = default;

    // CellId of cell that was added to the underling density map
    virtual void addNew(const cell_key_t& cellId, const time_t& time) = 0;

    /**
     * Get next cell ID from queue and sort selected id back into the queue based
     * on the implemented algorithm.
     */
    virtual const cell_key_t nextCellId() = 0;
    virtual const cell_key_t peekFront() = 0;
    virtual void update(const time_t& time) = 0;

};

}
