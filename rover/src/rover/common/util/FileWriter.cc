/*
 * FileWriter.cc
 *
 *  Created on: Sep 2, 2020
 *      Author: sts
 */

#include "rover/common/util/FileWriter.h"

#include <omnetpp.h>
#include <boost/algorithm/string/join.hpp>
#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>
#include <memory>

namespace rover {

FileWriterBuilder::FileWriterBuilder(std::string sep) {
  metadata["IDXCOL"] = "1";
  metadata["DATACOL"] = "-1";
  metadata["SEP"] = sep;
}

FileWriterBuilder &FileWriterBuilder::addPath(std::string path) {
  this->path = path;
  return *this;
}

FileWriter *FileWriterBuilder::build() {
  FileWriter *obj = new FileWriter();
  obj->initialize(path, metadata["SEP"]);
  int n = metadata.size();
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
    : s(std::move(other.s)), delim(std::move(other.delim)) {}

FileWriter::FileWriter() : s(), delim(";") {}

void FileWriter::initialize(std::string fileName, std::string _delim) {
  using namespace omnetpp;
  cConfigOption *scaObj = cConfigOption::find("output-scalar-file");
  if (scaObj) {
    std::string outputScalarFile = cSimulation::getActiveSimulation()
                                       ->getEnvir()
                                       ->getConfig()
                                       ->getAsFilename(scaObj);

    boost::filesystem::path p{outputScalarFile};
    std::string _path = p.parent_path().string() + "/" + fileName;
    s = std::ofstream(_path);
    delim = _delim;
  } else {
    throw cRuntimeError("output-scalar-file not found");
  }
}

void FileWriter::writeHeader(std::initializer_list<std::string> header) {
  boost::iterator_range<std::initializer_list<std::string>::const_iterator> rng(
      header.begin(), header.end());
  std::string joined = boost::algorithm::join(rng, delim);
  s << joined << std::endl;
}

std::string FileWriter::del() const { return delim; }

std::ostream &FileWriter::write() { return s; }

} /* namespace rover */
