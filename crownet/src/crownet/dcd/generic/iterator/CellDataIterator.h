/*
 * Cell.h
 *
 *  Created on: Nov 20, 2020
 *      Author: sts
 */

#pragma once

#include "crownet/common/iterator/FilterIterator.h"
#include "crownet/common/RsdProviderMixin.h"

namespace crownet {

/**
 * Predicate based iterator for Cell<C, N, T> objects.
 *
 * This class does not change any FilterIterator and is just here
 * for simplifying template specification in Cell<C, N, T> class
 * and Cell<C, N, T> specific predicates. See static factories
 *
 * Use static factory methods to use common iterators.
 */

template <typename CELL>
class CellDataIterator
    : public FilterIterator<
          CELL, typename CELL::map_t::iterator,
          typename CELL::map_t::value_type,
          std::function<bool(const typename CELL::map_t::value_type&)>> {
 public:
  CellDataIterator(typename CellDataIterator<CELL>::iterable_t* data,
                   typename CellDataIterator<CELL>::pred_t predicate);
  CellDataIterator(CellDataIterator& other,
                   typename CellDataIterator<CELL>::iter_t iter);

//  using iter_pred_t = typename CellDataIterator<CELL>::pred_t;
  // static factory methods with common predicates

  /**
   * Filter based on valid state.
   */
  static typename CellDataIterator<CELL>::pred_t getValidDataIter_pred(){
      typename CellDataIterator<CELL>::pred_t f =
          [](const typename CellDataIterator<CELL>::iter_value_t& val) -> bool {
        return val.second->valid();
      };
      return f;
  }

  static CellDataIterator<CELL> ValidDataIter(CELL* cell) {
      typename CellDataIterator<CELL>::pred_t f = CellDataIterator<CELL>::getValidDataIter_pred();
    return CellDataIterator<CELL>(cell,  f);
  }

  static typename CellDataIterator<CELL>::pred_t getValidRsdDataIter_pred(const int rsdid) {
      typename CellDataIterator<CELL>::pred_t f =
             [rsdid](const typename CellDataIterator<CELL>::iter_value_t& val) -> bool {
           return val.second->valid() && val.second->getResourceSharingDomainId() == rsdid;
         };
      return f;
  }

  static CellDataIterator<CELL> ValidRsdDataIter(CELL* cell, const int rsdid) {
      typename CellDataIterator<CELL>::pred_t f = CellDataIterator<CELL>::getValidRsdDataIter_pred(rsdid);
    return CellDataIterator<CELL>(cell, f);
  }

  static typename CellDataIterator<CELL>::pred_t getAllDataIter_pred() {
      typename CellDataIterator<CELL>::pred_t f =
           [](const typename CellDataIterator<CELL>::iter_value_t& val) -> bool {
         return true;
       };
    return f;
    }


};


#include "crownet/dcd/generic/iterator/CellDataIterator.tcc"

}  // namespace crownet
