/*
 * FileWriter.h
 *
 *  Created on: Sep 2, 2020
 *      Author: sts
 */

#pragma once

#include <fstream>
#include <sstream>
#include <map>
#include <memory>
#include <vector>

#include "FilePrinter.h"

namespace crownet {


class BaseFileWriter {
public:
    static std::string getAbsOutputPath(std::string fileName);

public:
    BaseFileWriter(std::string filePath, std::string sep=";", long bufferSize=8192);
    ~BaseFileWriter();
    void flush();
    void close();
    void writeMetaData(std::map<std::string, std::string>& mData);
    std::ostream &write();

    template <typename T>
    friend std::ostream &operator<<(BaseFileWriter &output, const T &_t);

protected:
    std::string sep;

private:
    std::ofstream file;
    long bufferSize;
    std::ostringstream buffer;
};

template <typename T>
inline std::ostream &operator<<(BaseFileWriter &output, const T &_t) {
  output.write() << _t;
  return output.write();
}

class FileWriter : public BaseFileWriter{

 public:
  virtual ~FileWriter()=default;
  FileWriter(std::string filePath, std::shared_ptr<FilePrinter> printer, std::string sep=";", long bufferSize=8192);
//  FileWriter(FileWriter &&other);


  void writeHeader();
  void writeData();


 private:
  std::shared_ptr<FilePrinter> printer;
};



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
