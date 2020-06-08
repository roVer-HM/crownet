//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "rover/aid/simple/SimpleAidConfig.h"

namespace rover {

Define_Module(SimpleAidConfig);

SimpleAidConfig::SimpleAidConfig() {
  // TODO Auto-generated constructor stub
}

SimpleAidConfig::~SimpleAidConfig() {
  // TODO Auto-generated destructor stub
}

void SimpleAidConfig::initialize(int stage) {
  if (stage == INITSTAGE_LOCAL) {
    cfg1 = par("cfg1").stdstringValue();
    cfg2 = par("cfg2").stdstringValue();
  }
}
}  // namespace rover
