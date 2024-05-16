#pragma once
#include "crownet/dcd/generic/transmissionOrderStrategies/OrderByCellIdStream.h"

#include <omnetpp/cexception.h>


template <typename C, typename N, typename T>
bool OrderByCellIdStream<C, N, T>::isValidAndCanBeSent(const time_t& time, const cell_key_t& cell_key) const {
    const auto& cell = this->map->getCell(cell_key);
    return cell.hasValid() &&
            cell.val() &&
            cell.val()->valid() &&
            cell.lastSent() < time;
}

template <typename C, typename N, typename T>
const bool OrderByCellIdStream<C, N, T>::hasNext(const time_t& now) {

    if(nextLoopDone.find(now) != nextLoopDone.end()){
        // for given time hasNext() already looped once. Break early.
        return false;
    }
    int size = cellOrder.size();

    for(int i = 0; i < size; i++){
        int curr = (this->head + i) % size;
        if (isValidAndCanBeSent(now, cellOrder[curr])){
            //found valid and ready to sent cell
            this->head = curr;
            return true;
        }
    }
    // no valid cell found. Keep head where it is
    this->nextLoopDone.insert(now); // save time point to break loop early for same time.
    return false;
}


template <typename C, typename N, typename T>
const typename OrderByCellIdStream<C, N, T>::cell_key_t
OrderByCellIdStream<C, N, T>::nextCellId(const time_t& now){
    if (this->hasNext(now)){
        auto id = cellOrder[head];
        head++;
        map->getCell(id).sentAt(now);
        return id;
    } else {
        throw omnetpp::cRuntimeError("No valid nextCellId");
    }
}


template <typename C, typename N, typename T>
typename OrderByCellIdStream<C, N, T>::Cell&
OrderByCellIdStream<C, N, T>::nextCell(const time_t& now) {
    auto id = nextCellId(now);
    return map->getCell(id);
}

template <typename C, typename N, typename T>
const int OrderByCellIdStream<C, N, T>::size(const time_t& now) {

    if(nextLoopDone.find(now) != nextLoopDone.end()){
        // for given time hasNext() already looped once without finding a valid cell. Break early.
        return 0;
    }
    int count = 0;
    for(const auto& cellId : this->cellOrder){
        if (this->isValidAndCanBeSent(now, cellId)){
            count++;
        }
    }
    return count;
}


template <typename C, typename N, typename T>
std::vector<typename OrderByCellIdStream<C, N, T>::cell_key_t>
OrderByCellIdStream<C, N, T>::getNumCellsOrLess(
        const time_t& now, const int numCells) {

    std::vector<typename OrderByCellIdStream<C, N, T>::cell_key_t> ret;

    while(this->hasNext(now)){
        auto cellId = this->nextCellId(now);
        ret.push_back(cellId);
        if ((ret.size() == numCells) || (ret.size() == this->cellOrder.size())){
            break; // enough cells collected or no more cells present, stop and return
        }
    }
    return ret;
}


