/*
 * FreeList.h
 *
 *  Created on: Oct 14, 2021
 *      Author: vm-sts
 */

#pragma once

#include <vector>
#include <omnetpp/cexception.h>

template<class T>
class FreeList
{
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

    // Number of valid entries
    int size() const;

    // Lenght of underling data structure
    int length() const;

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
int FreeList<T>::size() const{
    return static_cast<int>(data.size()) - free_count;
}

template <class T>
int FreeList<T>::length() const{
    return static_cast<int>(data.size());
}











