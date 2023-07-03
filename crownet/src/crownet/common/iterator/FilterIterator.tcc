/*
 * MapDataIterator.tcc
 *
 *  Created on: Dec 1, 2020
 *      Author: sts
 */

#pragma once

#include "crownet/common/iterator/FilterIterator.h"

template <typename T, typename Iter, typename IterVal, typename Pred>
FilterIterator<T, Iter, IterVal, Pred>::FilterIterator(iterable_t* data,
                                                       pred_t predicate)
    : data(data), predicate(predicate) {
  this->iter = data->end();
  this->_begin = data->end();
  auto cIter = data->begin();
  while (cIter != data->end()) {
    if (this->predicate(*cIter)) {
      this->iter = cIter;
      this->_begin = cIter;
      break;
    }
    ++cIter;
  }
}

template <typename T, typename Iter, typename IterVal, typename Pred>
FilterIterator<T, Iter, IterVal, Pred>::FilterIterator(FilterIterator& other,
                                                       iter_t iter)
    : data(other.data),
      predicate(other.predicate),
      iter(iter),
      _begin(other._begin) {}

template <typename T, typename Iter, typename IterVal, typename Pred>
typename FilterIterator<T, Iter, IterVal, Pred>::iter_value_t
    FilterIterator<T, Iter, IterVal, Pred>::operator*() const {
  return *(this->iter);
}

template <typename T, typename Iter, typename IterVal, typename Pred>
FilterIterator<T, Iter, IterVal, Pred>& FilterIterator<T, Iter, IterVal, Pred>::
operator++() {
  bool _pred = false;
  while (!_pred) {
    this->iter++;
    if (this->iter == this->data->end()) {
      return *this;
    } else {
      _pred = this->predicate(*this->iter);
    }
  }
  return *this;
}

template <typename T, typename Iter, typename IterVal, typename Pred>
FilterIterator<T, Iter, IterVal, Pred> FilterIterator<T, Iter, IterVal, Pred>::
operator++(int) {
  FilterIterator<T, Iter, IterVal, Pred> tmp(*this);
  this->operator++();
  return tmp;
}

template <typename T, typename Iter, typename IterVal, typename Pred>
bool FilterIterator<T, Iter, IterVal, Pred>::operator==(
    const FilterIterator<T, Iter, IterVal, Pred>& rhs) {
  // compare pointer (iterator based on same data)
  return (this->data == rhs.data) && (this->iter == rhs.iter);
}

template <typename T, typename Iter, typename IterVal, typename Pred>
bool FilterIterator<T, Iter, IterVal, Pred>::operator!=(
    const FilterIterator<T, Iter, IterVal, Pred>& rhs) {
  return !operator==(rhs);
}

template <typename T, typename Iter, typename IterVal, typename Pred>
FilterIterator<T, Iter, IterVal, Pred>
FilterIterator<T, Iter, IterVal, Pred>::begin() {
  return FilterIterator<T, Iter, IterVal, Pred>(*this, this->_begin);
}

template <typename T, typename Iter, typename IterVal, typename Pred>
FilterIterator<T, Iter, IterVal, Pred>
FilterIterator<T, Iter, IterVal, Pred>::end() {
  return FilterIterator<T, Iter, IterVal, Pred>(*this, this->data->end());
}

template <typename T, typename Iter, typename IterVal, typename Pred>
const FilterIterator<T, Iter, IterVal, Pred> FilterIterator<T, Iter, IterVal, Pred>::begin() const {
    return const_cast<FilterIterator<T, Iter, IterVal, Pred>*>(this)->begin();
}

template <typename T, typename Iter, typename IterVal, typename Pred>
const FilterIterator<T, Iter, IterVal, Pred> FilterIterator<T, Iter, IterVal, Pred>::end() const {
    return const_cast<FilterIterator<T, Iter, IterVal, Pred>*>(this)->end();
}


template <typename T, typename Iter, typename IterVal, typename Pred>
int FilterIterator<T, Iter, IterVal, Pred>::distance() {
  // new iterator
  FilterIterator<T, Iter, IterVal, Pred> tmp(this->data, this->predicate);
  int i = 0;
  for (; tmp != tmp.end(); ++tmp) {
    ++i;
  }
  return i;
}
