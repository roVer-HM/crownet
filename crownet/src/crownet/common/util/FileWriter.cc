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
FileWriter *FileWriterBuilder::build(RegularDcdMap* map, const std::string &mapType){
    FileWriter *obj = nullptr;
    if (mapType == "all"){
        obj = new FileWriter(std::make_shared<RegularDcdMapAllPrinter>(map));
    } else {
        obj = new FileWriter(std::make_shared<RegularDcdMapValuePrinter>(map));
    }
    obj->initialize(path, metadata["SEP"]);
    // write metadata
    int n = metadata.size();
    obj->write() << "#";
    for (const auto &e : metadata) {
      obj->write() << e.first << "=" << e.second;
      if (n > 1) {
        obj->write() << ",";
      }
      n--;
    }
    obj->write() << "\n";


    return obj;
}

FileWriter *FileWriterBuilder::build(std::shared_ptr<FilePrinter> printer) {
  FileWriter *obj = new FileWriter(std::move(printer));
  obj->initialize(path, metadata["SEP"]);
  // write metadata
  int n = metadata.size();
  obj->write() << "#";
  for (const auto &e : metadata) {
    obj->write() << e.first << "=" << e.second;
    if (n > 1) {
      obj->write() << ",";
    }
    n--;
  }
  obj->write() << "\n";

  return obj;
}

FileWriter::~FileWriter() {
  s.flush();
  s.close();
}

FileWriter::FileWriter(FileWriter &&other)
    : s(std::move(other.s)), sep(other.sep), printer(other.printer) {}

FileWriter::FileWriter(std::shared_ptr<FilePrinter> printer)
    : s(), sep(";"), printer(std::move(printer)) {}

void FileWriter::writeHeader() { printer->writeHeaderTo(s, sep); }

void FileWriter::writeData() { printer->writeTo(s, sep); }

void FileWriter::initialize(std::string fileName, std::string _delim) {
  using namespace omnetpp;
  cConfigOption *scaObj = cConfigOption::find("output-scalar-file");
  if (scaObj) {
    std::string outputScalarFile = cSimulation::getActiveSimulation()
                                       ->getEnvir()
                                       ->getConfig()
                                       ->getAsFilename(scaObj);

    boost::filesystem::path p{outputScalarFile};
    std::string _path = p.parent_path().string() + "/" + fileName + ".csv";
    s = std::ofstream(_path);
    sep = _delim;
  } else {
    throw cRuntimeError("output-scalar-file not found");
  }
}

std::string FileWriter::del() const { return sep; }

std::ostream &FileWriter::write() { return s; }

} /* namespace crownet */
