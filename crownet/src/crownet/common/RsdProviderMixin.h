/*
 * RsdProviderMixin.h
 *
 *  Created on: Jun 23, 2023
 *      Author: vm-sts
 */

#ifndef CROWNET_COMMON_RSDPROVIDERMIXIN_H_
#define CROWNET_COMMON_RSDPROVIDERMIXIN_H_

#include <omnetpp/clistener.h>
#include "inet/common/InitStages.h"
#include "inet/common/ModuleAccess.h"
#include <omnetpp/clistener.h>


using namespace inet;


namespace crownet {

class RsdListener : public omnetpp::cListener{
    public:
        virtual ~RsdListener() = default;
        virtual void receiveSignal(cComponent *source, simsignal_t signalID, intval_t i, cObject *details) override;
        const int getResourceSharingDomainId() const {return resourceSharingDomainId;}
        const int getResourceSharingDomainIdPrev() const {return resourceSharingDomainId_prev;}
        void setSignal(omnetpp::simsignal_t servingCell_) {this->servingCell_ = servingCell_;}
    private:
        int resourceSharingDomainId = -1;
        int resourceSharingDomainId_prev = -1;
        omnetpp::simsignal_t servingCell_;

};


template<typename T>
class RsdProviderMixin : public T {

public:
    virtual ~RsdProviderMixin() = default;

    virtual void initialize(int stage) override;
    virtual void finish() override;


    const int getResourceSharingDomainId() const {return rsd.getResourceSharingDomainId();}
    const int getResourceSharingDomainIdPrev() const {return rsd.getResourceSharingDomainIdPrev();}


private:
    RsdListener rsd;
    omnetpp::simsignal_t servingCell_;


};

template<typename T>
inline void RsdProviderMixin<T>::finish(){
    getContainingNode(this)->unsubscribe(servingCell_, &rsd);
    T::finish();
}


template<typename T>
inline void RsdProviderMixin<T>::initialize(int stage){
    T::initialize(stage);
    if(stage == INITSTAGE_LOCAL){
        servingCell_ = T::registerSignal("servingCell");
        rsd.setSignal(servingCell_);
        getContainingNode(this)->subscribe(servingCell_, &rsd);
    }
}


} /* namespace crownet */

#endif /* CROWNET_COMMON_RSDPROVIDERMIXIN_H_ */
