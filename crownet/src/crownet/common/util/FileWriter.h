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
#include <omnetpp.h>
#include "traci/Boundary.h"
#include "Writer.h"
#include "FilePrinter.h"

namespace crownet {


class BaseFileWriter : public BufferWriter {
public:
    static std::string getAbsOutputPath(std::string fileName);

public:
    BaseFileWriter(std::string filePath="", std::string sep=";", long bufferSize=8192);
    ~BaseFileWriter();
    void writeMetaData(std::map<std::string, std::string>& mData);

    virtual void initialize() override;

    const char * getFilePath() const { return filePath.c_str();}
    void setFilePath(const char * path){ this->filePath = std::string(path);}
    const char * getSep() const { return sep.c_str();}
    void setSep(const char * sep) { this->sep = std::string(sep); }

    virtual void flush() override;
    virtual void close() override;

    bool isInitialized() const {return  init;}
    virtual void onInit() {/* do nothing */}

    template <typename T>
    friend std::ostream &operator<<(BaseFileWriter &output, const T &_t);

private:
    std::string filePath;
    std::ofstream file;

protected:
    void writeBuffer() override;
    std::string sep;
};

template <typename T>
inline std::ostream &operator<<(BaseFileWriter &output, const T &_t) {
  output.write() << _t;
  return output.write();
}



class ActiveFileWriter : public BaseFileWriter,
                         public ActiveWriter {

 public:
  virtual ~ActiveFileWriter()=default;
  ActiveFileWriter(std::string filePath, std::shared_ptr<FilePrinter> printer, std::string sep=";", long bufferSize=8192);


  virtual void initWriter() override;
  virtual void writeData() override;
  virtual void finish() override { close(); }


 private:
  std::shared_ptr<FilePrinter> printer;
};

class DevNullWriter : public ActiveWriter {
 public:
  virtual ~DevNullWriter()=default;
  DevNullWriter(){};
  virtual void initWriter() override {};
  virtual void writeData() override {};
  virtual void finish() override {};
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


 protected:
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
template <>
inline FileWriterBuilder &FileWriterBuilder::addMetadata(std::string key, const traci::Boundary& b) {
    std::stringstream s;
    s << b.lowerLeftPosition().x << ";" << b.lowerLeftPosition().y << ";" << \
            b.upperRightPosition().x << ";" << b.upperRightPosition().y;
  metadata[key] = s.str();
  return *this;
}


class ActiveFileWriterBuilder : public FileWriterBuilder {
public:

    ActiveFileWriter *build(std::shared_ptr<FilePrinter> printer);
    template <typename M>
    ActiveFileWriter *build(std::shared_ptr<M> map, const std::string &mapType);

};

} /* namespace crownet */
