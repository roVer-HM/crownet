/*
 * FileWriter.cc
 *
 *  Created on: Sep 2, 2020
 *      Author: sts
 */

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
    cConfigOption *scaObj = cConfigOption::find("output-scalar-file");
    if (scaObj) {
      std::string outputScalarFile = cSimulation::getActiveSimulation()
                                         ->getEnvir()
                                         ->getConfig()
                                         ->getAsFilename(scaObj);

      boost::filesystem::path p{outputScalarFile};
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
        : file(filePath), sep(sep), bufferSize(bufferSize) {}

BaseFileWriter::~BaseFileWriter(){
    close();
}
void BaseFileWriter::flush(){
    file << buffer.str();
    buffer.clear();
    file.flush();
}
void BaseFileWriter::close(){
    file << buffer.str();
    buffer.clear();
    file.flush();
    file.close();
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

std::ostream &BaseFileWriter::write(){
    long pos = buffer.tellp();
    if (pos > bufferSize){
        // write buffer to field
        file << buffer.str();
        buffer.clear();
    }
    return buffer;
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
ActiveFileWriter *ActiveFileWriterBuilder::build(std::shared_ptr<RegularDcdMap> map, const std::string &mapType){
    ActiveFileWriter *obj = nullptr;
    if (mapType == "all"){
        obj = new ActiveFileWriter(
                BaseFileWriter::getAbsOutputPath(path),
                std::make_shared<RegularDcdMapAllPrinter>(map));
    } else {
        obj = new ActiveFileWriter(
                BaseFileWriter::getAbsOutputPath(path),
                std::make_shared<RegularDcdMapValuePrinter>(map));
    }
    obj->writeMetaData(metadata);
    return obj;
}

ActiveFileWriter *ActiveFileWriterBuilder::build(std::shared_ptr<FilePrinter> printer) {
  ActiveFileWriter *obj = new ActiveFileWriter(
          BaseFileWriter::getAbsOutputPath(path),
          std::move(printer));
  obj->writeMetaData(metadata);
  return obj;
}

//FileWriter::FileWriter(FileWriter &&other)
//    : s(std::move(other.s)), sep(other.sep), printer(other.printer) {}

ActiveFileWriter::ActiveFileWriter(std::string filePath, std::shared_ptr<FilePrinter> printer, std::string sep, long bufferSize)
    : BaseFileWriter(filePath, sep, bufferSize), printer(std::move(printer)) {}

void ActiveFileWriter::writeHeader() { printer->writeHeaderTo(write(), sep); }

void ActiveFileWriter::writeData() { printer->writeTo(write(), sep); }



} /* namespace crownet */
