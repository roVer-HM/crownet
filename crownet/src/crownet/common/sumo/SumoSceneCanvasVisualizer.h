/*
 * SumoSceneCanvasVisualizer.h
 *
 *  Created on: Jul 7, 2021
 *      Author: vm-sts
 */

#ifndef CROWNET_COMMON_SUMO_SUMOSCENECANVASVISUALIZER_H_
#define CROWNET_COMMON_SUMO_SUMOSCENECANVASVISUALIZER_H_

#include "inet/common/geometry/common/CanvasProjection.h"
#include "inet/visualizer/base/SceneVisualizerBase.h"
#include "crownet/common/converter/OsgCoordinateConverter.h"


using namespace omnetpp;
using namespace inet;

namespace crownet {

class SumoSceneCanvasVisualizer : public inet::visualizer::SceneVisualizerBase{
protected:
  std::shared_ptr<OsgCoordinateConverter> converter;
  CanvasProjection *canvasProjection = nullptr;

public:
    virtual ~SumoSceneCanvasVisualizer() = default;
    virtual void initialize(int stage) override;
    cGroupFigure *createMapFigure(const cXMLElement* map);

protected:
    std::vector<cFigure::Point> parseShape(const char* shapeAttr);

};

} /* namespace crownet */

#endif /* CROWNET_COMMON_SUMO_SUMOSCENECANVASVISUALIZER_H_ */
