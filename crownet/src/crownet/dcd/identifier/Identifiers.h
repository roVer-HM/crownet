/*
 * nodeIdentifiers.h
 *
 *  Created on: Nov 20, 2020
 *      Author: sts
 */

#pragma once

#include <iostream>
#include <sstream>
#include <string>

#include "crownet/common/util/FilePrinter.h"

namespace crownet {

class CellIdentifiere : public FilePrinter {
 public:
  virtual ~CellIdentifiere() = default;
  virtual std::string str() const { return "---"; }
};  // namespace crownet

std::ostream& operator<<(std::ostream& os, const CellIdentifiere& i);

class GridCellID : public CellIdentifiere {
 public:
  GridCellID() {}
  GridCellID(int i, int j) : id(std::make_pair(i, j)) {}
  GridCellID(const GridCellID& other): id(other.id) {}
  std::string str() const override;
  bool operator<(const GridCellID& rhs) const;
  bool operator==(const GridCellID& rhs) const;
  virtual int columns() const override;
  virtual void writeTo(std::ostream& out,
                       const std::string& sep) const override;
  virtual void writeHeaderTo(std::ostream& out,
                             const std::string& sep) const override;
  virtual std::pair<int, int> val() const { return id; }
  virtual std::pair<int, int> value() const { return id; }
  virtual int x() const {return id.first;}
  virtual int y() const {return id.second;}

 private:
  std::pair<int, int> id;
};

///////////////////////////////////////////////////////////////////////////////

template <typename T>
class NodeIdentifiere : public FilePrinter {
 public:
  virtual ~NodeIdentifiere() = default;
  NodeIdentifiere() {}
  NodeIdentifiere(T id) : id(id) {}
  std::string str() const {
    std::stringstream os;
    os << id;
    return os.str();
  }

  bool operator<(const NodeIdentifiere<T>& rhs) const { return id < rhs.id; }
  bool operator==(const NodeIdentifiere<T>& rhs) const { return id == rhs.id; }
  bool operator!=(const NodeIdentifiere<T>& rhs) const { return id != rhs.id; }

  T value() const { return id; }

 protected:
  T id;
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const NodeIdentifiere<T>& i) {
  return os << i.str();
}

class IntIdentifer : public NodeIdentifiere<int> {
 public:
  IntIdentifer() {}
  IntIdentifer(int id) : NodeIdentifiere<int>(id) {}
  int columns() const override;
  virtual void writeTo(std::ostream& out,
                       const std::string& sep) const override;
  virtual void writeHeaderTo(std::ostream& out,
                             const std::string& sep) const override;
};

// class DblIdentifer : public NodeIdentifiere<double> {};

}  // namespace crownet
