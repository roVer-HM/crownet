//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#pragma once

#include <omnetpp.h>
#include "inet/common/InitStages.h"
#include "crownet/common/util/NeighborhoodEventWriter.h"

namespace crownet {

using namespace inet;

class IFileWriterRegister {
public:
    virtual ~IFileWriterRegister() = default;

    template <typename T>
    T* getWriterAs(std::string writerKey){
        return dynamic_cast<T*>(getWriter(writerKey));
    }
    virtual bool hasWriter(std::string writerKey) = 0;

private:
    virtual BaseFileWriter* getWriter(std::string writerKey) = 0;

};




class FileWriterRegister : public omnetpp::cSimpleModule, public IFileWriterRegister {
public:
    virtual ~FileWriterRegister();

    // cSimpleModule
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override { throw omnetpp::cRuntimeError("not supported"); }
    virtual void finish() override;

    virtual bool hasWriter(std::string writerKey) override;
    virtual BaseFileWriter* getWriter(std::string writerKey) override;

private:
    cValueMap* writerRegister = nullptr;
};

}
