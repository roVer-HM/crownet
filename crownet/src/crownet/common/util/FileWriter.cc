/*
 * FileWriter.cc
 *
 *  Created on: Sep 2, 2020
 *      Author: sts
 */

#include "crownet/crownet.h"
#include "crownet/common/util/FileWriter.h"
#include "crownet/dcd/regularGrid/RegularDcdMap.h"
#include "crownet/dcd/regularGrid/RegularDcdMapPrinter.h"

#include <omnetpp.h>
#include <boost/algorithm/string/join.hpp>
#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>
#include <memory>

namespace crownet {

std::string BaseFileWriter::getAbsOutputPath(std::string fileName){

    boost::filesystem::path pp{fileName};
    if (pp.is_absolute()){
        return fileName;
    }
    cConfigOption *scaObj = cConfigOption::find("output-scalar-file");
    if (scaObj) {
      std::string outputScalarFile = cSimulation::getActiveSimulation()
                                         ->getEnvir()
                                         ->getConfig()
                                         ->getAsFilename(scaObj);

      boost::filesystem::path p{outputScalarFile};
      boost::filesystem::absolute(p);
      if (!boost::filesystem::exists(p.parent_path())){
           boost::filesystem::create_directories(p.parent_path());
      }
      std::string _path;
      if (0 == fileName.compare(fileName.size() - 4, fileName.size(), ".csv")){
          _path = p.parent_path().string() + "/" + fileName;
      } else {
          _path = p.parent_path().string() + "/" + fileName + ".csv";
      }

      return _path;
    } else {
      throw cRuntimeError("output-scalar-file not found");
    }

};

BaseFileWriter::BaseFileWriter(std::string filePath, std::string sep, long bufferSize)
        : BufferWriter(bufferSize), filePath(filePath),  sep(sep){}

void BaseFileWriter::initialize(){
    if (isInitialized()){
        return; // only once
    }
    if (filePath == ""){
        throw cRuntimeError("Path is not set");
    }
    filePath = getAbsOutputPath(filePath);
    EV_INFO << "create file: " << filePath << endl;
    file = std::ofstream(filePath);
    init = true;
    onInit();
}

BaseFileWriter::~BaseFileWriter(){
    close();
}

void BaseFileWriter::writeBuffer(){
    file << buffer.str();
    buffer.str(std::string());
    buffer.clear();
}
void BaseFileWriter::flush(){
    writeBuffer();
    file.flush();
}
void BaseFileWriter::close(){
    if (!closed){
        flush();
        file.close();
        closed = true;
    }
}

void BaseFileWriter::writeMetaData(std::map<std::string, std::string>& mData){
    int n = mData.size();
    write() << "#";
    for (const auto &e : mData) {
      write() << e.first << "=" << e.second;
      if (n > 1) {
        write() << ",";
      }
      n--;
    }
    write() << std::endl;

}






FileWriterBuilder::FileWriterBuilder(std::string sep) {
  metadata["IDXCOL"] = "1";
  metadata["DATACOL"] = "-1";
  metadata["SEP"] = sep;
}

FileWriterBuilder &FileWriterBuilder::addPath(const std::string &path) {
  this->path = path;
  return *this;
}


template <>
ActiveFileWriter *ActiveFileWriterBuilder::build(std::shared_ptr<RegularDcdMap> map, MapCfg *mapCfg){
    ActiveFileWriter *obj = nullptr;
    if (strcmp(mapCfg->getMapTypeLog(), "all") == 0){
        obj = new ActiveFileWriter(
                path,
                std::make_shared<RegularDcdMapAllPrinter>(map));
    } else {
        obj = new ActiveFileWriter(
                path,
                std::make_shared<RegularDcdMapValuePrinter>(map));
    }
    obj->initialize();
    obj->writeMetaData(metadata);
    obj->flush();
    return obj;
}

ActiveFileWriter *ActiveFileWriterBuilder::build(std::shared_ptr<FilePrinter> printer) {
  ActiveFileWriter *obj = new ActiveFileWriter(
          path,
          std::move(printer));
  obj->initialize();
  obj->writeMetaData(metadata);
  obj->flush();
  return obj;
}

ActiveFileWriter::ActiveFileWriter(std::string filePath, std::shared_ptr<FilePrinter> printer, std::string sep, long bufferSize)
    : BaseFileWriter(filePath, sep, bufferSize), printer(std::move(printer)) {}

void ActiveFileWriter::initWriter() { printer->writeHeaderTo(write(), sep); }

void ActiveFileWriter::writeData() { printer->writeTo(write(), sep); }



} /* namespace crownet */
