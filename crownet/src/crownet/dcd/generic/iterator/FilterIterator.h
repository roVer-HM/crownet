/*
 * MapDataIterator.h
 *
 *  Created on: Nov 30, 2020
 *      Author: sts
 */

#pragma once

#include <functional>

namespace crownet {
//
//template <typename C, typename N, typename T>
//class Cell;
//
//template <typename C, typename N, typename T>
//class DcDMap;

/**
 * todo comment
 */
template <typename T, typename Iter, typename IterVal, typename Pred>
class FilterIterator : public std::iterator<std::output_iterator_tag, IterVal> {
 public:
  using iterable_t = T;
  using iter_t = Iter;
  using iter_value_t = IterVal;
  using pred_t = Pred;

  FilterIterator(iterable_t* data, pred_t predicate);
  FilterIterator(FilterIterator& other, iter_t iter);

  virtual iter_value_t operator*() const;
  FilterIterator<T, Iter, IterVal, Pred>& operator++();
  FilterIterator<T, Iter, IterVal, Pred> operator++(int);
  bool operator==(const FilterIterator<T, Iter, IterVal, Pred>& rhs);
  bool operator!=(const FilterIterator<T, Iter, IterVal, Pred>& rhs);

  FilterIterator<T, Iter, IterVal, Pred> begin();
  FilterIterator<T, Iter, IterVal, Pred> end();

  int distance();

 protected:
  iterable_t* data;
  pred_t predicate;
  iter_t iter;
  iter_t _begin;
};
#include "crownet/dcd/generic/iterator/FilterIterator.tcc"

}  // namespace crownet
