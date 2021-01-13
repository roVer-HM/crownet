/*
 * DcDMapIterator.tcc
 *
 *  Created on: Dec 1, 2020
 *      Author: sts
 */

#pragma once

#include "crownet/dcd/generic/iterator/DcDMapIterator.h"

template <typename MAP>
DcDMapIterator<MAP>::DcDMapIterator(
    typename DcDMapIterator<MAP>::iterable_t* data,
    typename DcDMapIterator<MAP>::pred_t predicate)
    : FilterIterator<
          MAP, typename MAP::map_t::iterator, typename MAP::map_t::value_type,
          std::function<bool(const typename MAP::map_t::value_type&)>>(
          data, predicate) {}

template <typename MAP>
DcDMapIterator<MAP>::DcDMapIterator(DcDMapIterator& other,
                                    typename DcDMapIterator<MAP>::iter_t iter)
    : FilterIterator<
          MAP, typename MAP::map_t::iterator, typename MAP::map_t::value_type,
          std::function<bool(const typename MAP::map_t::value_type&)>>(other,
                                                                       iter) {}

// template <typename MAP>
// typename DcDMapIterator<MAP>::iter_value_t DcDMapIterator<MAP>::operator*()
//    const {
//  return *(this->iter);
//}
