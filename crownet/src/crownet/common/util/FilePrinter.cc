/*
 * FilePrinter.cc
 *
 *  Created on: Oct 21, 2021
 *      Author: vm-sts
 */

#include "crownet/common/util/FilePrinter.h"

namespace crownet {

cObjectPrinter cObjectPrinter::shortBeaconInfoShortPrinter(
        {"nodeId","receivedTimeCurrent", "maxSequencenumber", "positionCurrent", "numberOfNeighboursCurrent"});


int cObjectPrinter::columns() const { return fields.size(); }

std::ostream& cObjectPrinter::writeTo(std::ostream& out, const omnetpp::cObject* data, const std::string& sep) {
    readIds(data);
    omnetpp::cClassDescriptor* d = data->getDescriptor();
    for(int i = 0; i < fieldIds.size(); i++){
        out << d->getFieldValueAsString(omnetpp::toAnyPtr(data), fieldIds[i], 0);
        if(i < sepCount){
            out << sep;
        }
    }
    return out;
}

std::ostream& cObjectPrinter::writeHeaderTo(std::ostream& out, const std::string& sep) {
    for(int i = 0; i < fields.size(); i++){
        out << fields[i];
        if(i < sepCount){
            out << sep;
        }
    }
    return out;
}

std::ostream& cObjectPrinter::operator()(std::ostream& out, const omnetpp::cObject* data, const std::string& sep){
    return writeTo(out, data, sep);
}

std::string cObjectPrinter::operator()(const omnetpp::cObject* data, const std::string& sep){
    std::ostringstream out;
    writeTo(out, data, sep);
    return out.str();
}


void cObjectPrinter::readIds(const omnetpp::cObject* data){
    if (fields.size() > 0 && fields.size() != fieldIds.size()){
        omnetpp::cClassDescriptor* d = data->getDescriptor();
        for(int i = 0; i < fields.size(); i++){
            int fieldId = d->findField(fields[i].c_str());
            if (fieldId == -1){
               throw omnetpp::cRuntimeError("expected field with name %s", fields[i].c_str());
            } else {
                fieldIds.push_back(fieldId);
            }
        }
    }
}



}
