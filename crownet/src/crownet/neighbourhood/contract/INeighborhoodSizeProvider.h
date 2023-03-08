/*
 * INeighborhoodSizeProvider.h
 *
 *  Created on: Mar 8, 2023
 *      Author: vm-sts
 */

#ifndef CROWNET_NEIGHBOURHOOD_CONTRACT_INEIGHBORHOODSIZEPROVIDER_H_
#define CROWNET_NEIGHBOURHOOD_CONTRACT_INEIGHBORHOODSIZEPROVIDER_H_

namespace crownet {


class NeighborhoodSizeProvider {
public:
    virtual ~NeighborhoodSizeProvider() = default;
    virtual const int getNeighborhoodSize() = 0;
};

}

#endif /* CROWNET_NEIGHBOURHOOD_CONTRACT_INEIGHBORHOODSIZEPROVIDER_H_ */
