# vadere Scenario Files
This folder contains the vadere scenario files - they can be edited within the vadere-gui.

Warning: NEVER create dynamic nodes in vadere at t==0.0s - since simu5G
only assumes that a node is dynamically created if it appears at a 
simulation time > 0.0s

Temporary workaround: create all nodes in vadere at t > sync step, i.e.
at t >0.4 until this problem has been fixed

Note: The additional *_env.xml files are used for visualizing the scenario within OMNeT++. 
They can be exported in vadere in the "Post-Visualization" step. Simply select the camera 
symbol and choose "INET Env" as export format.
