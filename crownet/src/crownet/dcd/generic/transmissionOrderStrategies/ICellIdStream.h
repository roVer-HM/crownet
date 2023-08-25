
#pragma once

namespace crownet {

template <typename C, typename N, typename T>
class DcDMap;

template <typename C, typename N, typename T>
class Cell;

/**
 * Provide a stream of cellID's or cells that are valid and where not transmitted
 * at the time given.
 *
 * The ICellIdStream takes an underlying DcDMap and is inform automatically by
 * the DcDMap of new cells.
 *
 *
 *
 *
 */
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
     * True if at the NEXT cellId/cell in the stream can be provided that is valid and
     * was not sent during the given time.
     */
    virtual const bool hasNext(const time_t& now) const = 0 ;
    virtual const int size(const time_t& now) const =0;

    /**
     *  Consume cell from ICellIdStream and mark cell as consumed (cell.sentAt(now))
     *  Only the cellId is returned. If no valid cell exist an exception is thrown.
     */
    virtual const cell_key_t nextCellId(const time_t& now) = 0;

    /**
     * Consume cell from ICellIdStream and mark cell as consumed (cell.sentAt(now))
     * If no valid cell exist an exception is thrown.
     */
    virtual Cell& nextCell(const time_t& now) = 0;
    /**
     * Return at most numCells valid cells that where not send at current time.
     */
    virtual std::vector<cell_key_t> getNumCellsOrLess(const time_t& now, const int numCells) = 0;


    virtual void update(const time_t& time) = 0;


};

}
