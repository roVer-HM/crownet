#pragma once
#include "crownet/dcd/generic/CellIdStream.h"
#include <omnetpp/cexception.h>


template <typename C, typename N, typename T>
void InsertionOrderedCellIdStream<C, N, T>::addNew(const cell_key_t& cellId, const time_t& time){
    this->queue.emplace_front(cellId);
}

template <typename C, typename N, typename T>
const bool InsertionOrderedCellIdStream<C, N, T>::hasNext(const time_t& now) {


   for(int i=0; i < this->queue.size(); i++) {

       auto id = this->queue.front();
       auto cell = this->map->getCell(id);

       if (cell.lastSent() >= now){
           // all cells were sent at this time point.
           // keep order and return false
           return false;
       }
       // cell must be sent if the cell is valid

       if (cell.hasValid() && // cell has at least on valid entry
               cell.val() &&  // cell has an selected/calculated
               cell.val()->valid()) // cell has an selected/calculated and its valid
       {
           // current value can be sent
           // keep order and return true
           return true;
       } else {
          // current value is not valid. move to the end of the queue
           moveBack();
       }
   }
   // no cell is valid return false
   return false;

}

template <typename C, typename N, typename T>
void InsertionOrderedCellIdStream<C, N, T>::moveBack(){
    auto id = this->queue.front();
    this->queue.pop_front();
    this->queue.push_back(id);
}

template <typename C, typename N, typename T>
const typename InsertionOrderedCellIdStream<C, N, T>::cell_key_t
InsertionOrderedCellIdStream<C, N, T>::nextCellId(const time_t& now){
    if (this->hasNext(now)){
        auto id = this->queue.front();
        moveBack();
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


