/*
 * MapDataIterator.h
 *
 *  Created on: Nov 30, 2020
 *      Author: sts
 */

#pragma once

#include <functional>

namespace crownet {

template <typename T, typename Iter, typename IterVal, typename Pred>
class FilterIterator {
public:
    using iterator_category = std::output_iterator_tag;  // Define the iterator category
    using value_type = IterVal;                          // Define the type of values the iterator points to
    using difference_type = std::ptrdiff_t;              // Define the type to compute distances
    using pointer = IterVal*;                            // Define the pointer type
    using reference = IterVal&;                          // Define the reference type

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

    const FilterIterator<T, Iter, IterVal, Pred> begin() const;
    const FilterIterator<T, Iter, IterVal, Pred> end() const;

    int distance() const;

protected:
    iterable_t* data;
    pred_t predicate;
    iter_t iter;
    iter_t _begin;
};
#include "crownet/common/iterator/FilterIterator.tcc"

}  // namespace crownet
