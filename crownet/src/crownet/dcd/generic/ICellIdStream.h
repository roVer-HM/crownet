
#pragma once

namespace crownet {

template <typename C, typename N, typename T>
class DcDMap;

template <typename C, typename N, typename T>
class Cell;


template <typename C, typename N, typename T>
class ICellIdStream {
public:
    using cell_key_t = C;
    using node_key_t = N;
    using time_t = T;
    using dMapPtr = DcDMap<C, N, T>*;
    using Cell = Cell<C, N, T>;

    virtual ~ICellIdStream() = default;

    // CellId of cell that was added to the underling density map
    virtual void addNew(const cell_key_t& cellId, const time_t& time) = 0;
    virtual void setMap(dMapPtr map) = 0;

    /**
     * True if at least one more cellId/cell can be provided that is valid and
     * was not sent during the given time.
     */
    virtual const bool hasNext(const time_t& now) = 0;

    /**
     * Return next cellId or cell. If hasNext() returns false this method throws an exception.
     */
    virtual const cell_key_t nextCellId(const time_t& now) = 0;
    virtual Cell& nextCell(const time_t& now) = 0;

//    /**
//     * return first element of stream.  If hasNext() returns false this method throws an exception.
//     */
//    virtual const cell_key_t peekCellId() = 0;
//    virtual CellPtr peekCell() = 0;

    virtual void update(const time_t& time) = 0;

};

}
