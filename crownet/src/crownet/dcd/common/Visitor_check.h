/*
 * Visitor_check.h
 *
 *  Created on: Jun 26, 2023
 *      Author: vm-sts
 */

#ifndef CROWNET_DCD_COMMON_VISITOR_CHECK_H_
#define CROWNET_DCD_COMMON_VISITOR_CHECK_H_



// https://slashvar.github.io/2019/08/17/Detecting-Functions-existence-with-SFINAE.html
template <typename V, typename T>
auto setTimeIfIdempotenceVisitor(V &visitor, T time, int) -> decltype(visitor.setLastCall(time))
 {
    visitor.setLastCall(time);
 }

template <typename V, typename T>
auto setTimeIfIdempotenceVisitor(V &visitor, T time, float) -> void
 {
    /*do nothing*/
 }

template <typename V, typename T>
auto setTimeIfIdempotenceVisitor(V *visitor, T time, int) -> decltype(visitor->setLastCall(time))
 {
    visitor->setLastCall(time);
 }

template <typename V, typename T>
auto setTimeIfIdempotenceVisitor(V *visitor, T time, float) -> void
 {
    /*do nothing*/
 }


#endif /* CROWNET_DCD_COMMON_VISITOR_CHECK_H_ */
