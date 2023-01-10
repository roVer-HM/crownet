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

#include "DensityMapSceneCanvasVisualizer.h"
#include "crownet/common/ModuleAccess.h"

namespace crownet {

Define_Module(DensityMapSceneCanvasVisualizer);

DensityMapSceneCanvasVisualizer::DensityMapSceneCanvasVisualizer() {

}

DensityMapSceneCanvasVisualizer::~DensityMapSceneCanvasVisualizer() {

}


void DensityMapSceneCanvasVisualizer::initialize(int stage) {
    inet::visualizer::SceneVisualizerBase::initialize(stage);
    if(!hasGUI()) return;
    if (stage == INITSTAGE_PHYSICAL_ENVIRONMENT) {
        coordinateSystem = inet::getModuleFromPar<OsgCoordConverterProvider>(
                        par("coordConverterModule"), this);
        converter = coordinateSystem->getConverter();
        auto canvas = visualizationTargetModule->getCanvas();
        canvasProjection = CanvasProjection::getCanvasProjection(canvas);
        auto mapGroupFigure = createMapFigure();
        mapGroupFigure->setZIndex(par("zIndex"));
        visualizationTargetModule->getCanvas()->addFigure(mapGroupFigure);
        cDisplayString& displayString = visualizationTargetModule->getDisplayString();
        if (par("adjustBackgroundBox").boolValue()) {
            traci::TraCIPosition br = traci::TraCIPosition(
                    converter->getSimBound().upperRightPosition().x,
                    converter->getSimBound().lowerLeftPosition().y);
            cFigure::Point bottomRight = converter->toCanvas(br);
            displayString.setTagArg("bgb", 0, bottomRight.x);
            displayString.setTagArg("bgb", 1, bottomRight.y);
        }

    }
}

cGroupFigure *DensityMapSceneCanvasVisualizer::createMapFigure(){
    const cFigure::Color COLOR_BOUND = { 255, 0, 0 };
    const cFigure::Color COLOR_AREA_OF_INTREST = { 0, 0, 255 };
    const cFigure::Color COLOR_GRID = { 200, 200, 200};

    auto bound = new cGroupFigure("MapBound");
    auto aoi = new cGroupFigure("AreaOfIntrest");
    auto grid = new cGroupFigure("Grid");

    cRectangleFigure *rect = converter->toCanvas(converter->getSimBound(), "Bound");
    rect->setFilled(false);
    rect->setLineColor(COLOR_BOUND);
    bound->addFigure(rect);

    rect = converter->toCanvas(converter->getAreaOfInterestBound(), "AOI");
    rect->setFilled(false);
    rect->setLineColor(COLOR_AREA_OF_INTREST);
    rect->setLineStyle(cFigure::LINE_DOTTED);
    aoi->addFigure(rect);

    for (const auto& cellId : converter->getGridDescription().aoiIter()){
        auto c = converter->toCanvas(cellId);
        c->setLineColor(COLOR_GRID);
        grid->addFigure(c);
    }

    auto mapFigure = new cGroupFigure("Density Map");
    mapFigure->setTags("street_map");
    mapFigure->addFigure(bound);
    mapFigure->addFigure(aoi);
    mapFigure->addFigure(grid);

    return mapFigure;
}
}
