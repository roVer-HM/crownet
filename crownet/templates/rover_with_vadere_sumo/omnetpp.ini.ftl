<#if wizardType=="simulation">
[General]

################ Mobility parameters #####################
**.mobility.constraintAreaMinZ = 0m
**.mobility.constraintAreaMaxZ = 0m


################ TraCI parameters #####################
*.manager.updateInterval = 0.4s
*.manager.margin = 0
*.manager.autoShutdown = true


[Config vadere-debug]
# network = XXX

################ TraCI parameters #####################
*.manager.host = "vadere"
*.manager.port = 9998
*.manager.moduleType = "pedestrian=rover.nodes.MobileUe"
*.manager.moduleName = "pedestrian=ueD2D"
*.manager.moduleDisplayString = "pedestrian='i=veins/node/pedestrian;is=vs'"
*.manager.vadereCachePath = "vadere-cache/veins-test"
#*.manager.vadereScenarioPath = "vadere/scenarios/default.scenario"
*.manager.visualizer = "visualization"


################ PhysicalEnviroment #####################
#*.physicalEnvironment.config = xmldoc("vadere/default_env.xml")
*.physicalEnvironment.spaceMinX = -100m
*.physicalEnvironment.spaceMinY = -100m
*.physicalEnvironment.spaceMinZ = -100m
*.physicalEnvironment.spaceMaxX = 600m
*.physicalEnvironment.spaceMaxY = 600m
*.physicalEnvironment.spaceMaxZ = 600m



[Config sumo-debug]
# network = XXX

################ TraCI parameters #####################
*.manager.host = "sumo"
*.manager.port = 9999
*.manager.moduleType = "DEFAULT_PEDTYPE=rover.nodes.MobileUe"
*.manager.moduleName = "DEFAULT_PEDTYPE=ueD2D"
*.manager.moduleDisplayString = "DEFAULT_PEDTYPE='i=veins/node/pedestrian;is=vs'"
*.manager.sumoConfigDir = "sumo"
#*.manager.launchConfig = xmldoc("sumo/default.launchd.xml")
</#if>
