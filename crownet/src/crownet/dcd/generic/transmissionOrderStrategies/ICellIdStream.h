
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
 * This API access an underlying DcDMap and is informed by the DcDMap of new
 * cells `addNew(const cell_key_t& cellId, const time_t& time)` if new cells are
 * added, and after the DcDMap is applied the cell value algorithm `update(const time_t& time)`.
 *
 * both addNew or update can change the order of returned cells.
 *
 * The ICellIdStream can be consumed only once for a provided time. Meaning, calling repeatedly
 * `nextCellId`, `nextCell`, or `getNumCellsOrLess` with the same time stamp will drain the stream.
 * leading to an exception for `nextCellId`, `nextCell` and and empty list for `getNumCellsOrLess`.
 *
 * The `update` method is called every time the underlying DcDMap evaluates `computeValues`
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

    /**
     * called by the underlying DcdMap to inform the ICellIdStream about new cells
     */
    virtual void addNew(const cell_key_t& cellId, const time_t& time) = 0;
    /**
     * Set underlying DcdMap called during DcdMap creation
     */
    virtual void setMap(dMapPtr map) = 0;

    /**
     * Called by the underlying DcdMap after `computeValues` was executed. The ICellIdStream will reset
     * any state if needed and provides a linear order of cells valid cell_id's / cells  to be consumed.
     */
    virtual void update(const time_t& time) = 0;

    /**
     * True if the ICellIdStream as at least one cell that is valid (i.e. has at least one valid entry and the cell value is set)
     * and that cell was not sent during at the given time already.
     * This method can change the head position of the implemented ICellIdStream and might cache the given time to make subsequent
     * lookups faster.
     */
    virtual const bool hasNext(const time_t& now) = 0 ;

    /**
     * Number of cells in the ICellIdStream for which `hasNext` predicates return `True`.
     * This method can change the head position of the implemented ICellIdStream and might cache the given time to make subsequent
     * lookups faster.
     *
     */
    virtual const int size(const time_t& now) =0;

    /**
     *  Consumes the next cell_id in the order defined by the concrete implementation.
     *  The associated cell in the DcdMap will be mark cell as consumed (cell.sentAt(now))
     *  If no valid cell exist (i.e. the stream is empty) an exception is thrown.
     */
    virtual const cell_key_t nextCellId(const time_t& now) = 0;

    /**
     *  Consumes the next cell_id in the order defined by the concrete implementation.
     *  The associated cell in the DcdMap will be mark as consumed (cell.sentAt(now))
     *  and a reference to the cell is returned.
     *  If no valid cell exist (i.e. the stream is empty) an exception is thrown.
     */
    virtual Cell& nextCell(const time_t& now) = 0;

    /**
     *  Consumes the next zero to at most `numCells` cell_id's in the order defined by the concrete implementation.
     *  The associated cells in the DcdMap will be mark as consumed (cell.sentAt(now))
     *  and a the cell_ids are returned in order.
     *  If no valid cell exist (i.e. the stream is empty) an empty vector is returned.
     */
    virtual std::vector<cell_key_t> getNumCellsOrLess(const time_t& now, const int numCells) = 0;





};

}
