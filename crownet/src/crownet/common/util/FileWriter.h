/*
 * FileWriter.h
 *
 *  Created on: Sep 2, 2020
 *      Author: sts
 */

#pragma once

#include <fstream>
#include <map>
#include <memory>
#include <vector>

#include "FilePrinter.h"

namespace crownet {


class FileWriter {
 public:
  virtual ~FileWriter();
  FileWriter(std::shared_ptr<FilePrinter> printer);
  FileWriter(FileWriter &&other);

  void initialize(std::string absPath, std::string delim = ";");
  std::string del() const;

  std::ostream &write();
  void writeHeader();
  void writeData();

  template <typename T>
  friend std::ostream &operator<<(FileWriter &output, const T &_t);

 private:
  std::ofstream s;
  std::string sep;
  std::shared_ptr<FilePrinter> printer;
};

template <typename T>
inline std::ostream &operator<<(FileWriter &output, const T &_t) {
  output.s << _t;
  return output.s;
}

class FileWriterBuilder {
 public:
  FileWriterBuilder(std::string sep = ";");
  virtual ~FileWriterBuilder() = default;

  template <typename T>
  FileWriterBuilder &addMetadata(std::string key, T value) {
    metadata[key] = std::to_string(value);
    return *this;
  }
  FileWriterBuilder &addPath(const std::string &path);

  FileWriter *build(std::shared_ptr<FilePrinter> printer);
  template <typename M>
  FileWriter *build(std::shared_ptr<M> map, const std::string &mapType);

 private:
  using metadata_t = std::map<std::string, std::string>;
  metadata_t metadata;
  std::string path;
};

template <>
inline FileWriterBuilder &FileWriterBuilder::addMetadata(std::string key,
                                                         std::string value) {
  metadata[key] = value;
  return *this;
}

} /* namespace crownet */
