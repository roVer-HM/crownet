#pragma once
#include "crownet/dcd/generic/CellIdStream.h"


template <typename C, typename T>
void InsertionOrderedCellIdStream<C, T>::addNew(const cell_key_t& cellId, const time_t& time){
    this->queue.emplace_front(cellId);
}


template <typename C, typename T>
const typename InsertionOrderedCellIdStream<C, T>::cell_key_t
InsertionOrderedCellIdStream<C, T>::nextCellId(){
    auto id = this->queue.front();
    this->queue.pop_front();
    this->queue.push_back(id);
    return id;
}


template <typename C, typename T>
const typename InsertionOrderedCellIdStream<C, T>::cell_key_t
InsertionOrderedCellIdStream<C, T>::peekFront(){
    return this->queue.front();
}


