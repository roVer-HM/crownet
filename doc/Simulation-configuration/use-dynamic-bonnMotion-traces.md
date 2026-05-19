# Dynamic BonnMotion traces with creation and deletion support

The standard BonnMotion mobility module in INET will create all nodes at the 
beginning and will not remove nodes after the end of the traces is reached. 

The singleton module `BonnMotionMobilityServer` together with the per-node
mobility module `BonnMotionMobilityClient` enables this behavior.
Each node is created at the first time stamp mentioned in a given trace.
The order of traces in the files is not important. The BonnMotionMobilityServer
will sort the lines accordingly. 

## BonnMotionMobilityClient

Simple mobility module based on the `LineSegmentsMobilityBase` class. The nodes
provided by the traces must use this mobility module. No further settings are needed.

## BonnMotionMobilityServer

A top-level module that manages a vector of nodes with one node for 
each trace in the given BonnMotion file. 

Parameters:

* traceFile: BonnMotion trace file. Loaded only once.
* vectorNode: Name of the vector node to use for dynamic node creation. The vector 
              does not need to exist beforehand.
* moduleType: Type of node that should be created. These nodes must have a 
              mobility module of type `BonnMotionMobilityClient`.
* mobilityModulePath: Path to the mobility module


## Example Setup

If trace files are used, there is no mobility simulation framework to provide
coordinate conversion. Therefore, using a local converter is recommended.

```
[Config bonnMotion]
extends=noTraCI
# define local coordConverter 
*.coordConverter.typename = "OsgCoordConverterLocal"
*.coordConverter.srs_code = "+proj=utm +zone=32 +ellps=WGS84 +datum=WGS84 +units=m +no_defs"
*.coordConverter.offset_x = -692298.893546436m
*.coordConverter.offset_y = -5337359.5927164m
*.coordConverter.xBound = 281.135m
*.coordConverter.yBound = 233.492m

*.bonnMotionServer.typename = "BonnMotionMobilityServer"
*.bonnMotionServer.traceFile = "path/to/trace.bonnMotion"
*.bonnMotionServer.vectorNode  = "pNode"
*.pNode[*].mobility.typename = "BonnMotionMobilityClient"
*.pNode[*].... etc.
```


## Export from Vadere

To export BonnMotion traces for OMNeT++ from Vadere, add the following
files and processors in the Vadere configuration file. Ensure that the
reference IDs match and do not overlap with existing processors.

The BonnMotion processor relies on the `PedestrianPositionProcessor`, but
this processor does not need to be mapped in any output file. Ensure that there is
no metadata at the beginning of the file. If present, delete it.


Attributes:

* offset: adds the offset (translation) set in `referenceCoordinateSystem`
  if it is present. This is not needed when used with OMNeT++.
* origin: changes the coordinate origin. OMNeT++ uses `upper left` and
  Vadere (as well as Sumo) uses 'lower left'. 

```
{
  "files" : [ {
    "type" : "org.vadere.simulator.projects.dataprocessing.outputfile.BonnMotionTrajectoryFile",
    "filename" : "mf_2peds.bonnMotion",
    "processors" : [ 2 ]
  } ],
  "processors" : [  {
    "type" : "org.vadere.simulator.projects.dataprocessing.processor.PedestrianPositionProcessor",
    "id" : 1,
    "attributesType" : "org.vadere.state.attributes.processor.AttributesPedestrianPositionProcessor",
    "attributes" : {
      "interpolate" : true
    }
  }, {
    "type" : "org.vadere.simulator.projects.dataprocessing.processor.BonnMotionTrajectoryProcessor",
    "id" : 2,
    "attributesType" : "org.vadere.state.attributes.processor.AttributesBonnMotionTrajectoryProcessor",
    "attributes" : {
      "pedestrianPositionProcessorId" : 1,
      "scale" : {
        "x" : 1.0,
        "y" : 1.0
      },
      "translate" : {
        "x" : 0.0,
        "y" : 0.0
      },
      "applyOffset" : false,
      "origin" : "upper left"
    }
  } ],
  "isTimestamped" : true,
  "isWriteMetaData" : false
}
```