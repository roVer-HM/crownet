/*
 * SumoSceneCanvasVisualizer.cc
 *
 *  Created on: Jul 7, 2021
 *      Author: vm-sts
 */

#include <omnetpp/cexception.h>
#include "SumoSceneCanvasVisualizer.h"
#include "inet/common/ModuleAccess.h"
#include "crownet/common/converter/OsgCoordConverter.h"

using namespace omnetpp;
namespace crownet {

Define_Module(SumoSceneCanvasVisualizer);


void SumoSceneCanvasVisualizer::initialize(int stage){
    SceneVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_PHYSICAL_ENVIRONMENT) {
        auto canvas = visualizationTargetModule->getCanvas();
        converter = getModuleFromPar<OsgCoordConverterProvider>(par("coordinateSystemModule"), this)->getConverter();
        canvasProjection = CanvasProjection::getCanvasProjection(canvas);

        cXMLElement *mapXml = par("mapFile").xmlValue();
        auto mapGroupFigure = createMapFigure(mapXml);
        mapGroupFigure->setZIndex(par("zIndex"));
        visualizationTargetModule->getCanvas()->addFigure(mapGroupFigure);

        cDisplayString& displayString = visualizationTargetModule->getDisplayString();
        if (par("adjustBackgroundBox").boolValue()) {
            cFigure::Point bottomRight(converter->getBoundaryWidth(), converter->getBoundaryHeight());
            displayString.setTagArg("bgb", 0, bottomRight.x);
            displayString.setTagArg("bgb", 1, bottomRight.y);
        }
    }
}


cGroupFigure *SumoSceneCanvasVisualizer::createMapFigure(const cXMLElement* map){
    const cFigure::Color COLOR_HIGHWAY_PATH = { 0, 0, 0 };
    const cFigure::Color COLOR_FOOTWAY_PATH = { 128, 128, 128 };
    const cFigure::Color COLOR_JUNCTION_PATH = { 255, 85, 0 };
    const cFigure::Color COLOR_POLY_PATH_BUILDING = { 100, 100, 100 };
    const cFigure::Color COLOR_POLY_PATH_WATER = { 60, 60, 255 };
    const cFigure::Color COLOR_NATURAL_PATH = { 60, 255, 60 };

    auto lanesGroup = new cGroupFigure("lanes");
    auto junctionGroup = new cGroupFigure("junctions");
    auto mapFigure = new cGroupFigure("sumo");
    mapFigure->addFigure(lanesGroup);
    mapFigure->addFigure(junctionGroup);


    auto lanes = map->getElementsByTagName("lane");
    for(cXMLElement* lane : lanes){
        std::string id = lane->getAttribute("id");
        auto width_str = lane->getAttribute("width");
        auto speed_str = lane->getAttribute("speed");

        if (width_str == nullptr || speed_str == nullptr){
            continue;
        }

        double width = std::strtod(std::string(width_str).c_str(), nullptr);
        double speed = std::strtod(std::string(speed_str).c_str(), nullptr);

        cFigure::Color highwayColor= COLOR_FOOTWAY_PATH;

        std::vector<cFigure::Point> points = parseShape(lane->getAttribute("shape"));
        cPolylineFigure *polyline = new cPolylineFigure();
        polyline->setPoints(points);
        polyline->setZoomLineWidth(true);
        polyline->setLineColor(highwayColor);
        polyline->setName(id.c_str());
        polyline->setLineWidth(width);
        polyline->setCapStyle(cFigure::CAP_BUTT);
        polyline->setJoinStyle(cFigure::JOIN_ROUND);
        lanesGroup->addFigure(polyline);
    }

    auto junctions = map->getElementsByTagName("junction");
    for(cXMLElement* junc : junctions){
        std::string id = junc->getAttribute("id");
        std::vector<cFigure::Point> points = parseShape(junc->getAttribute("shape"));

        if (points.size() > 0) {
            cPolygonFigure *polygon = new cPolygonFigure();
            points.pop_back();
            polygon->setPoints(points);
            polygon->setFilled(true);
            polygon->setLineColor(cFigure::BLACK);
            polygon->setFillColor(COLOR_JUNCTION_PATH);
            polygon->setName(id.c_str());
            junctionGroup->addFigure(polygon);
        }
    }

    auto poly = map->getElementsByTagName("poly");
    for(cXMLElement* p : poly){
        std::string id = p->getAttribute("id");
        std::string type = p->getAttribute("type");

        cFigure::Color polyColor;

        if (type.substr(0, 8) == "building") {
            polyColor = COLOR_POLY_PATH_BUILDING;
        } else if (type.substr(0, 13) == "natural.water") {
            polyColor = COLOR_POLY_PATH_WATER;
        } else if (type.substr(0, 7) == "highway") {
            polyColor = COLOR_HIGHWAY_PATH;
        } else if (type.substr(0, 7) == "natural" || type.substr(0, 6) == "forest" || type.substr(0, 7) == "leisure") {
            polyColor = COLOR_NATURAL_PATH;
        } else {
            continue;
        }

        std::vector<cFigure::Point> points = parseShape(p->getAttribute("shape"));

        if (points.size() > 0) {
            cPolygonFigure *polygon = new cPolygonFigure();
            points.pop_back();
            polygon->setPoints(points);
            polygon->setFilled(true);
            polygon->setFillColor(polyColor);
            polygon->setName(id.c_str());
            junctionGroup->addFigure(polygon);
        }
    }


    return mapFigure;
}

std::vector<cFigure::Point> SumoSceneCanvasVisualizer::parseShape(const char* shapeAttr){
    cStringTokenizer tokenizer(shapeAttr, ", ");
    std::vector<double> points = tokenizer.asDoubleVector();
    std::vector<cFigure::Point> fPoints;
    if (points.size() % 2 != 0){
        throw cRuntimeError("expected an even number of double values");
    }

    if (points.size() == 0) {
        return fPoints;
    }

    for(int i=0; i < points.size()-1; i=i+2){
        fPoints.push_back(converter->toCanvas(points[i], points[i+1]));
    }

    return fPoints;
}



} /* namespace crownet */
