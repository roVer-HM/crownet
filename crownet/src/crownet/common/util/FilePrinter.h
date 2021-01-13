/*
 * FilePrinter.h
 *
 *  Created on: Dec 2, 2020
 *      Author: sts
 */

#pragma once

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

}  // namespace crownet
