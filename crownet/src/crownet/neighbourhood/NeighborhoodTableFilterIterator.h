/*
 * EntropyNeigborhoodTableIterator.h
 *
 *  Created on: Oct 4, 2022
 *      Author: vm-sts
 */

#pragma once

#include "crownet/crownet.h"
#include "inet/common/geometry/common/Coord.h"
#include "crownet/applications/beacon/BeaconReceptionInfo.h"
#include "crownet/common/converter/OsgCoordConverter.h"
#include "crownet/common/iterator/FilterIterator.h"
#include "crownet/common/entropy/EntropyProvider.h"
#include <list>

namespace crownet {

class GlobalEntropyMap;

using NeighborhoodTable_t = std::map<int, BeaconReceptionInfo*>;
using NeighborhoodTablePred_t = std::function<bool(const NeighborhoodTable_t::value_type&)>;
using NeighborhoodTableValue_t = NeighborhoodTable_t::value_type;



template <typename T, typename Iter, typename IterVal, typename Pred>
class NeighborhoodTableFilterIterator : public FilterIterator<T, Iter, IterVal, Pred> {
  public:
    NeighborhoodTableFilterIterator(
            typename FilterIterator<T, Iter, IterVal, Pred>::iterable_t* data,
            typename FilterIterator<T, Iter, IterVal, Pred>::pred_t predicate,
            GlobalEntropyMap* map)
    : FilterIterator<T, Iter, IterVal, Pred>(data, predicate), map(map){}
    NeighborhoodTableFilterIterator(
            NeighborhoodTableFilterIterator& other,
            typename FilterIterator<T, Iter, IterVal, Pred>::iter_t iter): FilterIterator<T, Iter, IterVal, Pred>(other, iter), map(other.map){}

    virtual typename FilterIterator<T, Iter, IterVal, Pred>::iter_value_t operator*() const override;

  private:
    GlobalEntropyMap* map = nullptr;
};
#include "crownet/neighbourhood/NeighborhoodTableFilterIterator.tcc"

using NeighborhoodTableIter_t = FilterIterator<NeighborhoodTable_t, NeighborhoodTable_t::iterator, NeighborhoodTableValue_t, NeighborhoodTablePred_t>;
using NeighborhoodTableLazyIter_t = NeighborhoodTableFilterIterator<NeighborhoodTable_t, NeighborhoodTable_t::iterator, NeighborhoodTableValue_t, NeighborhoodTablePred_t>;

}
