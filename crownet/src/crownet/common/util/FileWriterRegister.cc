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

#include "inet/common/ModuleAccess.h"


#include "FileWriterRegister.h"



namespace crownet {

Define_Module(FileWriterRegister);

FileWriterRegister::~FileWriterRegister(){}



void FileWriterRegister::initialize(int stage){
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL){
        writerRegister = (cValueMap*) par("register").objectValue();
    }
}

void FileWriterRegister::finish() {
    for(auto e : writerRegister->getFields()){
        auto w = dynamic_cast<BaseFileWriter*>(e.second.objectValue());
        if(w){
            w->close();
        }
    }
}
bool FileWriterRegister::hasWriter(std::string writerKey) {
    return writerRegister->containsKey(writerKey.c_str());
}


BaseFileWriter* FileWriterRegister::getWriter(std::string writerKey) {
    if (writerKey == "neighborhoodWriter" && writerRegister->containsKey(writerKey.c_str())) {
        auto w = (NeighborhoodEventWriter*)writerRegister->get(writerKey.c_str()).objectValue();
        if (!w->isInitialized()){
            w->initialize();
        }
        return dynamic_cast<BaseFileWriter*>(w);
     }
     throw cRuntimeError("Writer not found for key %s", writerKey.c_str());
}

} // namespace crownet

