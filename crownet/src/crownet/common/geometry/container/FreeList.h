/*
 * FreeList.h
 *
 *  Created on: Oct 14, 2021
 *      Author: vm-sts
 */

#pragma once

#include <vector>
#include <omnetpp/cexception.h>




/**
 *  Random access container with reusing holes created by deleting elements.
 *  Based on: https://stackoverflow.com/a/48330314 (CC BY-SA 3.0, user4842163)
 */
template<class T>
class FreeList
{
public:
    struct Iterator {
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type = T;
        using pointer = value_type*;
        using reference = value_type&;

       Iterator(FreeList* list, int index = 0): freeList(list) {
           if (index > freeList->data.size()){
               index = freeList->data.size(); // invalid
           } else {
               while(index < freeList->data.size() && freeList->data[index].next != VALID){
                   ++index;
               }
               this->index = index;
           }
       }

       reference operator*() const  { return freeList->data[index].element;}
       pointer operator->() {return &freeList->data[index].element;}

       Iterator& operator++() {
           while(index < freeList->data.size() && freeList->data[index].next != VALID){
               ++index;
           }
           return *this;
       }
       Iterator operator++(int) {Iterator tmp = *this; ++(*this); return tmp;}

       friend bool operator==(const Iterator& a, const Iterator& b){
           return a.freeList == b.freeList && a.index == b.index;
       }

       friend bool operator!=(const Iterator& a, const Iterator& b){
           return a.freeList != b.freeList && a.index != b.index;
       }

    private:
       FreeList* freeList;
       int index;
    };

public:
    FreeList();

    // Insert element in next free index.
    int insert(const T& element);

    // Remove the nth element
    void erase(int n);

    // Remove all elements
    void clear();

    // Read only element wise access
    const T& operator[](int n) const;
    T& operator[](int n);


    // Number of valid entries
    int size() const;

    // Capacity of underling data structure
    int capacity() const;

    // Returns elements in memory order NOT insertion order.
    // Iterator will skip empty / invalid entries.
    Iterator begin() { return Iterator(this); }
    Iterator end() {return Iterator(this, data.size()); }


private:
    static const int VALID = -2;
    struct ListElement {
        T element;
        int next = VALID;
    };
    std::vector<ListElement> data;
    int first_free;
    int free_count;
};


template <class T>
FreeList<T>::FreeList(): first_free(-1), free_count(0){}



template <class T>
int FreeList<T>::insert(const T& element){

    if (first_free > -1){
        // use free element
        const int index = first_free;
        first_free = data[index].next;
        data[index].element = element;
        data[index].next = VALID;
        --free_count;
        return index;
    } else {
        // append back
        ListElement e;
        e.element = element;
        e.next = VALID;
        data.push_back(e);
        return static_cast<int>(data.size() - 1);
    }
}

template <class T>
void FreeList<T>::erase(int n){
    data[n].next = first_free;
    first_free = n;
    ++free_count;
}

template <class T>
void FreeList<T>::clear(){
    data.clear();
    first_free = -1;
    free_count = 0;
}

template <class T>
const T& FreeList<T>::operator[](int n) const{

    if (data[n].next != VALID){
        throw omnetpp::cRuntimeError("Access invalid index %d in FreeList", n);
    }
    return data[n].element;
}

template <class T>
T& FreeList<T>::operator[](int n){

    if (data[n].next != VALID){
        throw omnetpp::cRuntimeError("Access invalid index %d in FreeList", n);
    }
    return data[n].element;
}

template <class T>
int FreeList<T>::size() const{
    return static_cast<int>(data.size()) - free_count;
}

template <class T>
int FreeList<T>::capacity() const{
    return static_cast<int>(data.size());
}











