#/bin/bash
# Small script to download measurement data from a cloud ressource.

# RES="https://sam.cs.hm.edu/samcloud/index.php/s/j3oNKycwebpe5kL/download?path=%2FOpenAirInterface&files=Schaipp_Project_1_2018-06-28.zip"
RES="https://sam.cs.hm.edu/samcloud/index.php/s/j3oNKycwebpe5kL/download?path=%2FOpenAirInterface&files=Schaipp_Project_2_2019-10-25.zip"
TARGET_DIR="measurements"
BASE_DIR=$(dirname "$0")

pushd .

cd $BASE_DIR
mkdir $TARGET_DIR
cd $TARGET_DIR

echo "Downloading measurement traces from cloud storage..."
echo "Ressource: $RES"
wget $RES -O measurements.zip

echo "Extracting measurement traces..."
unzip measurements.zip
rm measurements.zip

popd

echo "Done."



