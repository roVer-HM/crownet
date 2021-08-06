/*
 * DcDMapFileWriter.h
 *
 *  Created on: Dec 2, 2020
 *      Author: sts
 */

#pragma once

#include <omnetpp.h>

#include "crownet/common/util/FilePrinter.h"
#include "crownet/dcd/regularGrid/RegularDcdMap.h"

namespace crownet {

// todo integrate with crownet/common/util/FileWriter.h
class RegularDcdMapValuePrinter : public FilePrinter {
 public:
  RegularDcdMapValuePrinter(RegularDcdMap* map) : map(map){};

  virtual int columns() const override;
  virtual void writeTo(std::ostream& out,
                       const std::string& sep) const override;
  virtual void writeHeaderTo(std::ostream& out,
                             const std::string& sep) const override;


 protected:
  RegularDcdMap* map;
};

// todo print all cell values  not only the 'selected' one
class RegularDcdMapAllPrinter : public RegularDcdMapValuePrinter {
 public:
  RegularDcdMapAllPrinter(RegularDcdMap* map)
      : RegularDcdMapValuePrinter(map){};
  virtual void writeTo(std::ostream& out,
                       const std::string& sep) const override;
};

// todo print global map
class RegularDcdMapGlobalPrinter : public RegularDcdMapValuePrinter {
 public:
  RegularDcdMapGlobalPrinter(RegularDcdMap* map)
      : RegularDcdMapValuePrinter(map){};
  virtual void writeTo(std::ostream& out,
                       const std::string& sep) const override;
  virtual void writeHeaderTo(std::ostream& out,
                             const std::string& sep) const override;
};

}  // namespace crownet
