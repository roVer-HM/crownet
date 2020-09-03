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

namespace rover {

class FileWriter {
 public:
  virtual ~FileWriter();
  FileWriter();
  FileWriter(FileWriter &&other);

  void initialize(std::string absPath, std::string delim = ";");
  void writeHeader(std::initializer_list<std::string> header);
  std::string del() const;

  std::ostream &write();

  template <typename T>
  friend std::ostream &operator<<(FileWriter &output, const T &_t);

 private:
  std::ofstream s;
  std::string delim;
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
  FileWriterBuilder &addPath(std::string path);
  FileWriter *build();

 private:
  using metadata_t = std::map<std::string, std::string>;
  metadata_t metadata;
  std::string path;
  std::initializer_list<std::string> header;
};

template <>
inline FileWriterBuilder &FileWriterBuilder::addMetadata(std::string key,
                                                         std::string value) {
  metadata[key] = value;
  return *this;
}

} /* namespace rover */
