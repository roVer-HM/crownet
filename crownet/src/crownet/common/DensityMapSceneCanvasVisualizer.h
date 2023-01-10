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

#pragma once

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/geometry/common/CanvasProjection.h"
#include "inet/common/geometry/common/GeographicCoordinateSystem.h"
#include "inet/visualizer/base/SceneVisualizerBase.h"
#include "crownet/common/converter/OsgCoordConverter.h"

using namespace inet;

namespace crownet {

class DensityMapSceneCanvasVisualizer : public inet::visualizer::SceneVisualizerBase {
public:
    DensityMapSceneCanvasVisualizer();
    virtual ~DensityMapSceneCanvasVisualizer();
protected:
  CanvasProjection *canvasProjection = nullptr;
  OsgCoordConverterProvider* coordinateSystem;
  std::shared_ptr<OsgCoordinateConverter> converter;

protected:
  virtual void initialize(int stage) override;
  cGroupFigure *createMapFigure();

};
}

