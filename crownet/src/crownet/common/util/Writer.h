/*
 * Writer.h
 *
 *  Created on: Jan 26, 2022
 *      Author: vm-sts
 */

#pragma once

#include <fstream>
#include <sstream>
#include <omnetpp.h>


namespace crownet {

class ActiveWriter {
public:
    virtual ~ActiveWriter() = default;
    virtual void initWriter() = 0;
    virtual void writeData() = 0;
    virtual void finish() = 0;
};


class BufferWriter : public omnetpp::cOwnedObject {

public:
    BufferWriter(long bufferSize=8192): bufferSize(bufferSize){}
    virtual ~BufferWriter() = default;
    std::ostream &write();
    virtual void flush() = 0;
    virtual void close() = 0;
    virtual void initialize() = 0;
    long getBufferSize() const { return bufferSize; }
    void setBufferSize(long bufferSize) { this->bufferSize = bufferSize; }


protected:
    virtual void writeBuffer() = 0;
    bool init = false;
    bool closed = false;

    long bufferSize;
    std::ostringstream buffer;

};

} // namespace
