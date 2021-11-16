/*
 * DcDMapIterator.h
 *
 *  Created on: Dec 1, 2020
 *      Author: sts
 */

#pragma once

#include "crownet/common/iterator/FilterIterator.h"

namespace crownet {

template <typename MAP>
class DcDMapIterator
    : public FilterIterator<
          MAP, typename MAP::map_t::iterator, typename MAP::map_t::value_type,
          std::function<bool(const typename MAP::map_t::value_type&)>> {
 public:
  using iterable_t = typename DcDMapIterator<MAP>::iterable_t;
  using iter_value_t = typename DcDMapIterator<MAP>::iter_value_t;
  using iter_t = typename DcDMapIterator<MAP>::iter_t;
  using pred_t = typename DcDMapIterator<MAP>::pred_t;

  DcDMapIterator(iterable_t* data, pred_t predicate);
  DcDMapIterator(DcDMapIterator& other, iter_t iter);

  //  typename DcDMapIterator<MAP>::iter_value_t operator*() const override;

  static DcDMapIterator<MAP> LocalCellIter(MAP* map) {
    pred_t f = [](const iter_value_t& val) -> bool {
      return val.second.hasLocal();
    };
    return DcDMapIterator<MAP>(map, f);
  }

  static DcDMapIterator<MAP> ValidLocalCellIter(MAP* map) {
    pred_t f = [](const iter_value_t& val) -> bool {
      return val.second.hasValidLocal();
    };
    return DcDMapIterator<MAP>(map, f);
  }

  static DcDMapIterator<MAP> ValidCellIter(MAP* map) {
    pred_t f = [](const iter_value_t& val) -> bool {
      return val.second.hasValid();
    };
    return DcDMapIterator<MAP>(map, f);
  }
};
#include "crownet/dcd/generic/iterator/DcDMapIterator.tcc"
}  // namespace crownet
