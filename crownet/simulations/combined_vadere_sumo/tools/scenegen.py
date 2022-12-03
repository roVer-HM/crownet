#!/usr/bin/env python3
import math
import xml.etree.ElementTree as ET
import argparse
import re

def parse_svg_path(path):
    pointer = 0
    tokens = path.split(' ')
    coords = []

    def parse_coords(pair):
        return [float(c) for c in pair.split(',')]

    def consume_coords():
        nonlocal pointer

        pointer += 1
        return parse_coords(tokens[pointer])

    def consume_value():
        nonlocal pointer

        pointer += 1
        return float(tokens[pointer])

    def last_coords():
        if len(coords) == 0:
            return 0, 0

        return coords[-1]

    def add_coords(c1, c2):
        return [v1 + v2 for v1, v2 in zip(c1, c2)]

    while pointer < len(tokens):
        tok = tokens[pointer]
        if tok == 'm' or tok == 'l':
            coords.append(add_coords(consume_coords(), last_coords()))
        elif tok == 'M' or tok == 'L':
            coords.append(consume_coords())
        elif tok == 'v':
            coords.append(add_coords((0, consume_value()), last_coords()))
        elif tok == 'V':
            coords.append((last_coords()[0], consume_value()))
        elif tok == 'h':
            coords.append(add_coords((consume_value(), 0), last_coords()))
        elif tok == 'H':
            coords.append((consume_value(), last_coords()[1]))
        elif tok == 'Z':
            return coords
        elif tok == 'z':
            return coords
        else:
            coords.append(add_coords(parse_coords(tok), last_coords()))

        pointer += 1

    return coords

class SumoJunction:
    def __init__(self, x, y):
        self.x = x
        self.y = y

    def __repr__(self):
        return self.get_id()

    def __eq__(self, other):
        return self.get_id() == other.get_id()

    def get_id(self):
        return f'{self.x}_{self.y}'


class SumoEdge:
    def __init__(self, id, x1, y1, x2, y2):
        self.id = id
        self.x1 = x1
        self.x2 = x2
        self.y1 = y1
        self.y2 = y2

    def __repr__(self):
        return self.get_id()

    def __eq__(self, other):
        return self.get_id() == other.get_id()

    def get_id(self):
        return self.id

    def get_from_junction(self):
        return SumoJunction(self.x1, self.y1)

    def get_to_junction(self):
        return SumoJunction(self.x2, self.y2)

    def get_lane_id(self):
        return f'{self.get_id()}_0'

    def get_lane_length(self):
        return math.sqrt((self.x1 - self.x2) ** 2 + (self.y1 - self.y2) ** 2)

    def get_lane_shape(self):
        return f'{self.x1},{self.y1} {self.x2},{self.y2}'


class SumoConnection:
    def __init__(self, efrom, eto):
        self.efrom = efrom
        self.eto = eto


class VaderePoly:
    def __init__(self, id, coords, scene_w, scene_h, targets = ""):
        self.id = id
        self.coords = coords
        self.targets = targets
        self.scene_w = scene_w
        self.scene_h = scene_h

    @classmethod
    def from_rect(cls, id, x, y, w, h, scene_w, scene_h, targets = ""):
        return cls(
            id,
            [(x, y), (x + w, y), (x + w, y + h), (x, y + h)],
            scene_w,
            scene_h,
            targets
        )

    def rotate_by(self, rotation):
        self.coords = [VaderePoly.rotate_coords(c, rotation) for c in self.coords]

    def get_coords(self):
        return [(x, self.scene_h - y) for x, y in self.coords]

    @staticmethod
    def rotate_coords(coords, alpha):
        alpha = math.radians(alpha)

        x = coords[0]
        y = coords[1]

        x1 = x * math.cos(alpha) - y * math.sin(alpha)
        y1 = x * math.sin(alpha) + y * math.cos(alpha)

        return x1, y1

    @classmethod
    def from_svg_path(cls, id, path_string, scene_w, scene_h):
        return cls(id, parse_svg_path(path_string), scene_w, scene_h)



class GeoConfig:
    def __init__(self, epsg, offx, offy, sumo_bounds):
        self.epsg = epsg
        self.offx = offx
        self.offy = offy
        self.sumo_bounds = sumo_bounds


edges = []
width = 0
height = 0
sources = []
targets = []
obstacles = []
geo_config = None


def generate_sumo_net(sumo_net):
    junctions = []
    connections = []

    for edge in edges:
        if not edge.get_from_junction() in junctions:
            junctions.append(edge.get_from_junction())

        if not edge.get_to_junction() in junctions:
            junctions.append(edge.get_to_junction())

    for edge in edges:
        for other in edges:
            if edge.x2 == other.x1 and edge.y2 == other.y1:
                connections.append(SumoConnection(edge.get_id(), other.get_id()))

    net_xml = '<?xml version="1.0" encoding="UTF-8"?>\n'
    net_xml += '<net version="1.0" junctionCornerDetail="5" limitTurnSpeed="5.50" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://sumo.dlr.de/xsd/net_file.xsd">\n'

    net_xml += f'   <location netOffset="{-geo_config.offx},{-geo_config.offy}" convBoundary="0,0,{width},{height}" origBoundary="{geo_config.sumo_bounds}" projParameter="{geo_config.epsg}"/>\n'

    for edge in edges:
        net_xml += f'   <edge id="{edge.get_id()}" from="jun{edge.get_from_junction().get_id()}" to="jun{edge.get_to_junction().get_id()}" priority="-1">\n'
        net_xml += f'       <lane id="lan{edge.get_lane_id()}" index="0" speed="1.40" length="{edge.get_lane_length()}" width="2.50" shape="{edge.get_lane_shape()}"/>\n'
        net_xml += f'   </edge>\n'

    for junction in junctions:
        net_xml += f'   <junction id="jun{junction.get_id()}" type="priority" x="{junction.x}" y="{junction.y}" incLanes="" intLanes="">\n'
        net_xml += f'       <request index="0" response="0" foes="0"/>\n'
        net_xml += f'   </junction>\n'

    for connection in connections:
        net_xml += f'   <connection from="{connection.efrom}" to="{connection.eto}" fromLane="0" toLane="0" dir="l" state="m"/>\n'

    net_xml += '</net>\n'

    with open(sumo_net, 'w') as f:
        f.write(net_xml)


def generate_sumo(sumo_file):
    sumo_cfg = '<?xml version="1.0" encoding="UTF-8"?>\n'
    sumo_cfg += '<configuration xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://sumo.dlr.de/xsd/sumoConfiguration.xsd">\n'
    sumo_cfg += '   <input>\n'
    sumo_cfg += '       <net-file value="sumo.net.xml"/>\n'
    sumo_cfg += '       <route-files value="sumo.rou.xml"/>\n'
    sumo_cfg += '   </input>\n'
    sumo_cfg += '</configuration>\n'

    with open(sumo_file, 'w') as f:
        f.write(sumo_cfg)


def generate_vadere(vadere_file):
    vadere_json = '{\n'
    vadere_json += '    "name": "vadere",\n'
    vadere_json += '    "description": "",\n'
    vadere_json += '    "release": "2.2",\n'
    vadere_json += '    "commithash": "f5c800a863199107f62f86a7eea53065e9d62b86",\n'
    vadere_json += '    "processWriters": {\n'
    vadere_json += '        "files": [\n'
    vadere_json += '            {\n'
    vadere_json += '                "type": "org.vadere.simulator.projects.dataprocessing.outputfile.EventtimePedestrianIdOutputFile",\n'
    vadere_json += '                "filename": "postvis.traj",\n'
    vadere_json += '                "processors": [\n'
    vadere_json += '                    5,\n'
    vadere_json += '                    6\n'
    vadere_json += '                ]\n'
    vadere_json += '            }\n'
    vadere_json += '        ],\n'
    vadere_json += '        "processors": [\n'
    vadere_json += '            {\n'
    vadere_json += '                "type": "org.vadere.simulator.projects.dataprocessing.processor.FootStepProcessor",\n'
    vadere_json += '                "id": 5\n'
    vadere_json += '            },\n'
    vadere_json += '            {\n'
    vadere_json += '                "type": "org.vadere.simulator.projects.dataprocessing.processor.FootStepTargetIDProcessor",\n'
    vadere_json += '                "id": 6\n'
    vadere_json += '            }\n'
    vadere_json += '        ],\n'
    vadere_json += '        "isTimestamped": true,\n'
    vadere_json += '        "isWriteMetaData": false\n'
    vadere_json += '    },\n'
    vadere_json += '    "scenario": {\n'
    vadere_json += '        "mainModel": "org.vadere.simulator.models.osm.OptimalStepsModel",\n'
    vadere_json += '        "attributesModel": {\n'
    vadere_json += '            "org.vadere.state.attributes.models.AttributesOSM": {\n'
    vadere_json += '                "stepCircleResolution": 4,\n'
    vadere_json += '                "numberOfCircles": 1,\n'
    vadere_json += '                "optimizationType": "DISCRETE",\n'
    vadere_json += '                "varyStepDirection": true,\n'
    vadere_json += '                "movementType": "ARBITRARY",\n'
    vadere_json += '                "stepLengthIntercept": 0.4625,\n'
    vadere_json += '                "stepLengthSlopeSpeed": 0.2345,\n'
    vadere_json += '                "stepLengthSD": 0.036,\n'
    vadere_json += '                "movementThreshold": 0.0,\n'
    vadere_json += '                "minStepLength": 0.1,\n'
    vadere_json += '                "minimumStepLength": true,\n'
    vadere_json += '                "maxStepDuration": 1.7976931348623157E308,\n'
    vadere_json += '                "dynamicStepLength": true,\n'
    vadere_json += '                "updateType": "EVENT_DRIVEN",\n'
    vadere_json += '                "seeSmallWalls": false,\n'
    vadere_json += '                "targetPotentialModel": "org.vadere.simulator.models.potential.fields.PotentialFieldTargetGrid",\n'
    vadere_json += '                "pedestrianPotentialModel": "org.vadere.simulator.models.potential.PotentialFieldPedestrianCompactSoftshell",\n'
    vadere_json += '                "obstaclePotentialModel": "org.vadere.simulator.models.potential.PotentialFieldObstacleCompactSoftshell",\n'
    vadere_json += '                "submodels": []\n'
    vadere_json += '            },\n'
    vadere_json += '            "org.vadere.state.attributes.models.AttributesPotentialCompactSoftshell": {\n'
    vadere_json += '                "pedPotentialIntimateSpaceWidth": 0.45,\n'
    vadere_json += '                "pedPotentialPersonalSpaceWidth": 1.2,\n'
    vadere_json += '                "pedPotentialHeight": 50.0,\n'
    vadere_json += '                "obstPotentialWidth": 0.8,\n'
    vadere_json += '                "obstPotentialHeight": 6.0,\n'
    vadere_json += '                "intimateSpaceFactor": 1.2,\n'
    vadere_json += '                "personalSpacePower": 1,\n'
    vadere_json += '                "intimateSpacePower": 1\n'
    vadere_json += '            },\n'
    vadere_json += '            "org.vadere.state.attributes.models.AttributesFloorField": {\n'
    vadere_json += '                "createMethod": "HIGH_ACCURACY_FAST_MARCHING",\n'
    vadere_json += '                "potentialFieldResolution": 0.3,\n'
    vadere_json += '                "obstacleGridPenalty": 0.1,\n'
    vadere_json += '                "targetAttractionStrength": 1.0,\n'
    vadere_json += '                "cacheType": "BIN_CACHE",\n'
    vadere_json += '                "cacheDir": "",\n'
    vadere_json += '                "timeCostAttributes": {\n'
    vadere_json += '                    "standardDeviation": 0.7,\n'
    vadere_json += '                    "type": "UNIT",\n'
    vadere_json += '                    "obstacleDensityWeight": 3.5,\n'
    vadere_json += '                    "pedestrianSameTargetDensityWeight": 3.5,\n'
    vadere_json += '                    "pedestrianOtherTargetDensityWeight": 3.5,\n'
    vadere_json += '                    "pedestrianWeight": 3.5,\n'
    vadere_json += '                    "queueWidthLoading": 1.0,\n'
    vadere_json += '                    "pedestrianDynamicWeight": 6.0,\n'
    vadere_json += '                    "loadingType": "CONSTANT",\n'
    vadere_json += '                    "width": 0.2,\n'
    vadere_json += '                    "height": 1.0\n'
    vadere_json += '                }\n'
    vadere_json += '            }\n'
    vadere_json += '        },\n'
    vadere_json += '        "attributesSimulation": {\n'
    vadere_json += '            "finishTime": 400.0,\n'
    vadere_json += '            "simTimeStepLength": 0.4,\n'
    vadere_json += '            "realTimeSimTimeRatio": 0.1,\n'
    vadere_json += '            "writeSimulationData": true,\n'
    vadere_json += '            "visualizationEnabled": true,\n'
    vadere_json += '            "printFPS": false,\n'
    vadere_json += '            "digitsPerCoordinate": 2,\n'
    vadere_json += '            "useFixedSeed": true,\n'
    vadere_json += '            "fixedSeed": -7492697142818052001,\n'
    vadere_json += '            "simulationSeed": 0\n'
    vadere_json += '        },\n'
    vadere_json += '        "attributesPsychology": {\n'
    vadere_json += '            "usePsychologyLayer": false,\n'
    vadere_json += '            "psychologyLayer": {\n'
    vadere_json += '                "perception": "SimplePerceptionModel",\n'
    vadere_json += '                "cognition": "CooperativeCognitionModel",\n'
    vadere_json += '                "attributesModel": {\n'
    vadere_json += '                    "org.vadere.state.attributes.models.psychology.perception.AttributesSimplePerceptionModel": {\n'
    vadere_json += '                        "priority": {\n'
    vadere_json += '                            "1": "InformationStimulus",\n'
    vadere_json += '                            "2": "ChangeTargetScripted",\n'
    vadere_json += '                            "3": "ChangeTarget",\n'
    vadere_json += '                            "4": "Threat",\n'
    vadere_json += '                            "5": "Wait",\n'
    vadere_json += '                            "6": "WaitInArea",\n'
    vadere_json += '                            "7": "DistanceRecommendation"\n'
    vadere_json += '                        }\n'
    vadere_json += '                    },\n'
    vadere_json += '                    "org.vadere.state.attributes.models.psychology.cognition.AttributesCooperativeCognitionModel": {}\n'
    vadere_json += '                }\n'
    vadere_json += '            }\n'
    vadere_json += '        },\n'
    vadere_json += '        "topography": {\n'
    vadere_json += '            "attributes": {\n'
    vadere_json += '                "bounds": {\n'
    vadere_json += '                    "x": 0.0,\n'
    vadere_json += '                    "y": 0.0,\n'
    vadere_json += f'                    "width": {width},\n'
    vadere_json += f'                    "height": {height}\n'
    vadere_json += '                },\n'
    vadere_json += '                "boundingBoxWidth": 0.5,\n'
    vadere_json += '                "bounded": true,\n'
    vadere_json += '                "referenceCoordinateSystem": {\n'
    vadere_json += f'                    "epsgCode": "{geo_config.epsg}",\n'
    vadere_json += '                    "description": "Generated using SVG2Scen",\n'
    vadere_json += '                    "translation": {\n'
    vadere_json += f'                        "x": {geo_config.offx},\n'
    vadere_json += f'                        "y": {geo_config.offy}\n'
    vadere_json += '                    }\n'
    vadere_json += '                }\n'
    vadere_json += '            },\n'
    vadere_json += '            "obstacles": [\n'

    for obstacle in obstacles:
        if obstacle != obstacles[0]:
            vadere_json += ','

        vadere_json += '                {\n'
        vadere_json += '                     "shape" : {\n'
        vadere_json += '                     "type" : "POLYGON",\n'
        vadere_json += '                     "points" : [\n'

        for coord in obstacle.get_coords():
            if coord != obstacle.get_coords()[0]:
                vadere_json += ','

            vadere_json += '                            {\n'
            vadere_json += f'                                "x": {coord[0]},\n'
            vadere_json += f'                                "y": {coord[1]}\n'
            vadere_json += '                            }\n'

        vadere_json += '                     ]\n'
        vadere_json += '                     },\n'
        vadere_json += f'                     "id" : {obstacle.id}\n'
        vadere_json += '                }\n'

    vadere_json += '            ],\n'
    vadere_json += '            "measurementAreas": [],\n'
    vadere_json += '            "stairs": [],\n'
    vadere_json += '            "targets": [\n'

    for target in targets:
        if target != targets[0]:
            vadere_json += ','

        vadere_json += '                {\n'
        vadere_json += f'                    "id": {target.id},\n'
        vadere_json += '                    "absorbing": true,\n'
        vadere_json += '                    "shape": {\n'
        vadere_json += '                        "type": "POLYGON",\n'
        vadere_json += '                        "points": [\n'
        vadere_json += '                            {\n'
        vadere_json += f'                                "x": {target.get_coords()[0][0]},\n'
        vadere_json += f'                                "y": {target.get_coords()[0][1]}\n'
        vadere_json += '                            },\n'
        vadere_json += '                            {\n'
        vadere_json += f'                                "x": {target.get_coords()[1][0]},\n'
        vadere_json += f'                                "y": {target.get_coords()[1][1]}\n'
        vadere_json += '                            },\n'
        vadere_json += '                            {\n'
        vadere_json += f'                                "x": {target.get_coords()[2][0]},\n'
        vadere_json += f'                                "y": {target.get_coords()[2][1]}\n'
        vadere_json += '                            },\n'
        vadere_json += '                            {\n'
        vadere_json += f'                                "x": {target.get_coords()[3][0]},\n'
        vadere_json += f'                                "y": {target.get_coords()[3][1]}\n'
        vadere_json += '                            }\n'
        vadere_json += '                        ]\n'
        vadere_json += '                    },\n'
        vadere_json += '                    "waitingTime": 0.0,\n'
        vadere_json += '                    "waitingTimeYellowPhase": 0.0,\n'
        vadere_json += '                    "parallelWaiters": 0,\n'
        vadere_json += '                    "individualWaiting": true,\n'
        vadere_json += '                    "deletionDistance": 0.1,\n'
        vadere_json += '                    "startingWithRedLight": false,\n'
        vadere_json += '                    "nextSpeed": -1.0\n'
        vadere_json += '                }\n'

    vadere_json += '            ],\n'
    vadere_json += '            "targetChangers": [],\n'
    vadere_json += '            "absorbingAreas": [],\n'
    vadere_json += '            "aerosolClouds": [],\n'
    vadere_json += '            "droplets": [],\n'
    vadere_json += '            "sources": [\n'

    for source in sources:
        if source != sources[0]:
            vadere_json += ','

        vadere_json += '                {\n'
        vadere_json += f'                    "id": {source.id},\n'
        vadere_json += '                    "shape": {\n'
        vadere_json += '                        "type": "POLYGON",\n'
        vadere_json += '                        "points": [\n'
        vadere_json += '                            {\n'
        vadere_json += f'                                "x": {source.get_coords()[0][0]},\n'
        vadere_json += f'                                "y": {source.get_coords()[0][1]}\n'
        vadere_json += '                            },\n'
        vadere_json += '                            {\n'
        vadere_json += f'                                "x": {source.get_coords()[1][0]},\n'
        vadere_json += f'                                "y": {source.get_coords()[1][1]}\n'
        vadere_json += '                            },\n'
        vadere_json += '                            {\n'
        vadere_json += f'                                "x": {source.get_coords()[2][0]},\n'
        vadere_json += f'                                "y": {source.get_coords()[2][1]}\n'
        vadere_json += '                            },\n'
        vadere_json += '                            {\n'
        vadere_json += f'                                "x": {source.get_coords()[3][0]},\n'
        vadere_json += f'                                "y": {source.get_coords()[3][1]}\n'
        vadere_json += '                            }\n'
        vadere_json += '                        ]\n'
        vadere_json += '                    },\n'
        vadere_json += '                    "interSpawnTimeDistribution": "constant",\n'
        vadere_json += '                    "distributionParameters": {\n'
        vadere_json += '                        "updateFrequency": 60.0\n'
        vadere_json += '                    },\n'
        vadere_json += '                    "spawnNumber": 2,\n'
        vadere_json += '                    "maxSpawnNumberTotal": 6,\n'
        vadere_json += '                    "startTime": 0.0,\n'
        vadere_json += '                    "endTime": 90.0,\n'
        vadere_json += '                    "spawnAtRandomPositions": false,\n'
        vadere_json += '                    "spawnAtGridPositionsCA": false,\n'
        vadere_json += '                    "useFreeSpaceOnly": true,\n'
        vadere_json += '                    "targetIds": [\n'
        vadere_json += f'                        {source.targets}\n'
        vadere_json += '                    ],\n'
        vadere_json += '                    "groupSizeDistribution": [\n'
        vadere_json += '                        1.0\n'
        vadere_json += '                    ],\n'
        vadere_json += '                    "dynamicElementType": "PEDESTRIAN",\n'
        vadere_json += '                    "attributesPedestrian": null\n'
        vadere_json += '                }\n'

    vadere_json += '            ],\n'
    vadere_json += '            "dynamicElements": [],\n'
    vadere_json += '            "attributesPedestrian": {\n'
    vadere_json += '                "radius": 0.195,\n'
    vadere_json += '                "densityDependentSpeed": false,\n'
    vadere_json += '                "speedDistributionMean": 1.34,\n'
    vadere_json += '                "speedDistributionStandardDeviation": 0.26,\n'
    vadere_json += '                "minimumSpeed": 0.5,\n'
    vadere_json += '                "maximumSpeed": 2.2,\n'
    vadere_json += '                "acceleration": 2.0,\n'
    vadere_json += '                "footstepHistorySize": 4,\n'
    vadere_json += '                "searchRadius": 1.0,\n'
    vadere_json += '                "walkingDirectionCalculation": "BY_TARGET_CENTER",\n'
    vadere_json += '                "walkingDirectionSameIfAngleLessOrEqual": 45.0\n'
    vadere_json += '            },\n'
    vadere_json += '            "teleporter": null,\n'
    vadere_json += '            "attributesCar": null\n'
    vadere_json += '        },\n'
    vadere_json += '        "stimulusInfos": []\n'
    vadere_json += '    }\n'
    vadere_json += '}'

    with open(vadere_file, 'w') as f:
        f.write(vadere_json)


def parse_svg_rect_for_vadere(child):
    global width, height

    x = float(child.attrib['x'])
    y = float(child.attrib['y'])
    w = float(child.attrib['width'])
    h = float(child.attrib['height'])
    id = child.attrib['id']

    targets = ""
    if 'data-targets' in child.attrib:
        targets = child.attrib['data-targets']

    poly = VaderePoly.from_rect(id, x, y, w, h, width, height, targets)

    if 'transform' in child.attrib:
        match = re.match(r'rotate\((.+)\)', child.attrib['transform'])
        if match:
            alpha = float(match.group(1))
            poly.rotate_by(alpha)

    return poly


def parse_svg(svg):
    global edges, sources, targets, obstacles
    global width, height
    global geo_config

    tree = ET.parse(svg)
    g = tree.getroot()[0]

    width = float(tree.getroot().attrib['width'])
    height = float(tree.getroot().attrib['height'])

    geo_config = GeoConfig(
        tree.getroot().attrib['data-epsg'],
        float(tree.getroot().attrib['data-offset-x']),
        float(tree.getroot().attrib['data-offset-y']),
        tree.getroot().attrib['data-sumo-origbounds']
    )

    for child in g:
        if child.tag == '{http://www.w3.org/2000/svg}path':
            path = child.attrib['d']
            id = child.attrib['id']

            if child.attrib['style'].startswith('fill:#0000ff'):
                obstacles.append(VaderePoly.from_svg_path(id, path, width, height))
            else:
                pfrom = path.split(' ')[1]
                pto = path.split(' ')[3]


                edges.append(SumoEdge(
                    id,
                    float(pfrom.split(',')[0]),
                    float(pfrom.split(',')[1]),
                    float(pto.split(',')[0]),
                    float(pto.split(',')[1])
                ))

        elif child.tag == '{http://www.w3.org/2000/svg}rect':
            if child.attrib['style'].startswith('fill:#00ff00'):
                sources.append(parse_svg_rect_for_vadere(child))

            elif child.attrib['style'].startswith('fill:#ff0000'):
                targets.append(parse_svg_rect_for_vadere(child))

            elif child.attrib['style'].startswith('fill:#0000ff'):
                obstacles.append(parse_svg_rect_for_vadere(child))


def main():
    parser = argparse.ArgumentParser(description='Generate SUMO and Vadere scenario files from SVG images.')
    parser.add_argument('--svg')
    parser.add_argument('--sumo')
    parser.add_argument('--net')
    parser.add_argument('--vadere')
    args = parser.parse_args()

    parse_svg(args.svg)

    if args.sumo:
        generate_sumo(args.sumo)

    if args.net:
        generate_sumo_net(args.net)

    if args.vadere:
        generate_vadere(args.vadere)


if __name__ == '__main__':
    main()