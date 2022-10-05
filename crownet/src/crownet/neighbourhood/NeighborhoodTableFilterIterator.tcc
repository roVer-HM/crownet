#pragma once

#include "crownet/neighbourhood/NeighborhoodTableFilterIterator.h"

template <typename T, typename Iter, typename IterVal, typename Pred>
typename FilterIterator<T, Iter, IterVal, Pred>::iter_value_t
NeighborhoodTableFilterIterator<T, Iter, IterVal, Pred>::operator*() const {
    auto val = *(this->iter);
    return this->map->getValue(val.first);
}
