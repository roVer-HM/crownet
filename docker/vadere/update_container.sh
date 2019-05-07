#!/bin/bash
docker build -t sam-dev.cs.hm.edu:5023/rover/rover-main/vadere:latest .

if [ $? -eq 0 ]; then
   docker login sam-dev.cs.hm.edu:5023
   docker push sam-dev.cs.hm.edu:5023/rover/rover-main/vadere:latest
else 
   echo "Container build did not succeed - no upload to registry."
fi
