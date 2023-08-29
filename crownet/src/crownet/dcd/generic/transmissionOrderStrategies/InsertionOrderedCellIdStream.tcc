#pragma once
#include "crownet/dcd/generic/transmissionOrderStrategies/InsertionOrderedCellIdStream.h"

#include <omnetpp/cexception.h>
#include <algorithm>


template <typename C, typename N, typename T>
void InsertionOrderedCellIdStream<C, N, T>::addNew(const cell_key_t& cellId, const time_t& time){
    if(std::find(this->vec.begin(), this->vec.end(), cellId) == this->vec.end()){
        this->vec.push_back(cellId);
    } else {/*nothing to do cell already in vec*/}
}

template <typename C, typename N, typename T>
void InsertionOrderedCellIdStream<C,N,T>::update(const time_t& time){
    if (this->lastUpdated < time){
        this->lastUpdated = time;
        this->nextLoopDone.clear(); //reset hasNext loop marker.
        //keep head where it is and wrap around to start if needed.
    }
}

template <typename C, typename N, typename T>
bool InsertionOrderedCellIdStream<C, N, T>::isValidAndCanBeSent(const time_t& now, const cell_key_t& cell_key) const{
    const auto& cell = this->map->getCell(cell_key);
    return cell.hasValid() &&
            cell.val() &&
            cell.val()->valid() &&
            cell.lastSent() < now;
}

template <typename C, typename N, typename T>
const bool InsertionOrderedCellIdStream<C, N, T>::hasNext(const time_t& now) {

    if(nextLoopDone.find(now) != nextLoopDone.end()){
        // for given time hasNext() already looped once. Break early.
        return false;
    }
    int size = vec.size();

    for(int i = 0; i < size; i++){
        int curr = (this->head + i) % size;
        if (isValidAndCanBeSent(now, vec[curr])){
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
const typename InsertionOrderedCellIdStream<C, N, T>::cell_key_t
InsertionOrderedCellIdStream<C, N, T>::nextCellId(const time_t& now){
    if (this->hasNext(now)){
        auto id = vec[this->head];
        this->head = (this->head + 1) % vec.size();
        map->getCell(id).sentAt(now);
        return id;
    } else {
        throw omnetpp::cRuntimeError("No valid nextCellId");
    }
}


template <typename C, typename N, typename T>
typename InsertionOrderedCellIdStream<C, N, T>::Cell&
InsertionOrderedCellIdStream<C, N, T>::nextCell(const time_t& now){
    auto id = nextCellId(now);
    return map->getCell(id);
}

template <typename C, typename N, typename T>
const int InsertionOrderedCellIdStream<C, N, T>::size(const time_t& now) {
    if(nextLoopDone.find(now) != nextLoopDone.end()){
        // for given time hasNext() already looped once. Break early.
        return 0;
    }
    int count = 0;
    for(const auto& cellId : this->vec){
        if (this->isValidAndCanBeSent(now, cellId)){
            count++;
        }
    }
    return count;
}




template <typename C, typename N, typename T>
std::vector<typename InsertionOrderedCellIdStream<C, N, T>::cell_key_t>
InsertionOrderedCellIdStream<C, N, T>::getNumCellsOrLess(
        const time_t& now, const int numCells) {

    std::vector<typename InsertionOrderedCellIdStream<C, N, T>::cell_key_t> ret;

    while(this->hasNext(now)){
        auto cellId = this->nextCellId(now);
        ret.push_back(cellId);
        if ((ret.size() == numCells) || (ret.size() == this->vec.size())){
            break; // enough cells collected or no more cells present, stop and return
        }
    }
    return ret;
}


