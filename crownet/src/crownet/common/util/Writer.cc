/*
 * Writer.cc
 *
 *  Created on: Jan 26, 2022
 *      Author: vm-sts
 */




#include "Writer.h"

namespace crownet {

std::ostream &BufferWriter::write(){
    long pos = buffer.tellp();
    if (pos > bufferSize){
        // write buffer to field
        writeBuffer();
    }
    return buffer;
}

}
