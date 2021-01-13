/*
 * cell.tcc
 *
 *  Created on: Nov 21, 2020
 *      Author: sts
 */

#pragma once

#include "crownet/dcd/generic/iterator/CellDataIterator.h"

///////////////////////////////////////////////////////////////////////////////
/// CellDataIterator   ////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template <typename CELL>
CellDataIterator<CELL>::CellDataIterator(
    typename CellDataIterator<CELL>::iterable_t* data,
    typename CellDataIterator<CELL>::pred_t predicate)
    : FilterIterator<
          CELL, typename CELL::map_t::iterator,
          typename CELL::map_t::value_type,
          std::function<bool(const typename CELL::map_t::value_type&)>>(
          data, predicate) {}

template <typename CELL>
CellDataIterator<CELL>::CellDataIterator(
    CellDataIterator& other, typename CellDataIterator<CELL>::iter_t iter)
    : FilterIterator<
          CELL, typename CELL::map_t::iterator,
          typename CELL::map_t::value_type,
          std::function<bool(const typename CELL::map_t::value_type&)>>(other,
                                                                        iter) {}
