/*
 * FilePrinter.h
 *
 *  Created on: Dec 2, 2020
 *      Author: sts
 */

#pragma once
#include <sstream>
#include <utility>
#include <omnetpp/cclassdescriptor.h>

namespace crownet {

class FilePrinter {
 public:
  virtual ~FilePrinter() = default;

  // number of columns needed
  virtual int columns() const = 0;

  // write to stream. Only place sep *between* columns.
  // If FilePrinter only has one column do not write sep.
  virtual void writeTo(std::ostream& out, const std::string& sep) const = 0;
  virtual void writeHeaderTo(std::ostream& out,
                             const std::string& sep) const = 0;
};

template <typename T>
class FilePrinterMixin : public T, public FilePrinter{
public:
    FilePrinterMixin(const char *name=nullptr) : T(name), FilePrinter() {}
    virtual int columns() const override {
        return this->getDescriptor()->getFieldCount();
    }
    virtual void writeTo(std::ostream& out, const std::string& sep) const override {
        omnetpp::cClassDescriptor* d = this->getDescriptor();
        int fieldCount = d->getFieldCount();
        for (int i=0; i < fieldCount; i++){
            out << d->getFieldValueAsString(omnetpp::toAnyPtr(const_cast<FilePrinterMixin<T>*>(this)), i, 0);
            if(i < (fieldCount-1) ){
                out << sep;
            }
        }
    }
    virtual void writeHeaderTo(std::ostream& out, const std::string& sep) const override {
        omnetpp::cClassDescriptor * d = this->getDescriptor();
        int fieldCount = d->getFieldCount();
        for (int i=0; i < fieldCount; i++){
            out << d->getFieldName(i);
            if(i < (fieldCount-1) ){
                out << sep;
            }
        }
    }
};


class cObjectPrinter {
public:
    static cObjectPrinter shortBeaconInfoShortPrinter;

public:
    cObjectPrinter(std::vector<std::string> fields): fields(fields), fieldIds(), sepCount(fields.size()){}
    // number of columns needed
    int columns()const;
    std::ostream& writeTo(std::ostream& out, const omnetpp::cObject* data, const std::string& sep=";");
    std::ostream& writeHeaderTo(std::ostream& out, const std::string& sep=";");
    std::ostream& operator()(std::ostream& out, const omnetpp::cObject* data, const std::string& sep=";");
    std::string operator()(const omnetpp::cObject* data, const std::string& sep=";");

private:
    void readIds(const omnetpp::cObject* data);
    std::vector<std::string> fields;
    std::vector<int> fieldIds;
    int sepCount;
};



}  // namespace crownet
