#/bin/bash
# Small script to download measurement data from a cloud ressource.

# Initial measurement results for cellular communication via eNodeB
# RES="https://sam.cs.hm.edu/samcloud/index.php/s/rjAeQQ4mcCjaTzK/download?files=measurements_pub_1.tar.gz"
# Extended measurement results (cellular and direct communication, measurements in Feb.-Apr. 2020)
RES="https://sam.cs.hm.edu/samcloud/index.php/s/rjAeQQ4mcCjaTzK/download?files=measurements_pub_2.tar.gz"

TARGET_DIR="measurements"
BASE_DIR=$(dirname "$0")

pushd .

cd $BASE_DIR
mkdir $TARGET_DIR
cd $TARGET_DIR

echo "Downloading measurement traces from cloud storage..."
echo "Ressource: $RES"
wget $RES -O measurements.tar.gz

echo "Extracting measurement traces..."
tar -xzf measurements.tar.gz
rm measurements.tar.gz

popd

echo "Done."


