#!/bin/bash
# scope: write versions to config-file


# call this script with the following arguments:
# CROWNET_OPP_CONT_TAG CROWNET_VADERE_CONT_TAG CROWNET_CONTROL_CONT_TAG CROWNET_SUMO_CONT_TAG CROWNET_OPP_IDE_CONT_TAG CROWNET_VADERE_IDE_CONT_TAG CROWNET_CONTROL_IDE_CONT_TAG

CROWNET_OPP_CONT_TAG=$1
CROWNET_VADERE_CONT_TAG=$2   
CROWNET_CONTROL_CONT_TAG=$3
CROWNET_SUMO_CONT_TAG=$4

CROWNET_OPP_IDE_CONT_TAG=$5
CROWNET_VADERE_IDE_CONT_TAG=$6     
CROWNET_CONTROL_IDE_CONT_TAG=$7   

CONFIG_FILE="${CROWNET_HOME}/config/CONTAINER_VERSION.config"

echo "Write configuration to ${CONFIG_FILE}."
echo "-------------------- CONFIGURATION START---------------------------"

echo $(printf '#!/bin/bash') > "${CONFIG_FILE}" 
echo "" >> "${CONFIG_FILE}" 
echo $(printf 'export CROWNET_OPP_CONT_TAG="%s"' $CROWNET_OPP_CONT_TAG) >> "${CONFIG_FILE}"     
echo $(printf 'export CROWNET_VADERE_CONT_TAG="%s"' $CROWNET_VADERE_CONT_TAG) >> "${CONFIG_FILE}"     
echo $(printf 'export CROWNET_CONTROL_CONT_TAG="%s"' $CROWNET_CONTROL_CONT_TAG) >> "${CONFIG_FILE}"     
echo $(printf 'export CROWNET_SUMO_CONT_TAG="%s"' $CROWNET_SUMO_CONT_TAG) >> "${CONFIG_FILE}" 

echo $(printf 'export CROWNET_OPP_IDE_CONT_TAG="%s"' $CROWNET_OPP_IDE_CONT_TAG) >> "${CONFIG_FILE}"     
echo $(printf 'export CROWNET_VADERE_IDE_CONT_TAG="%s"' $CROWNET_VADERE_IDE_CONT_TAG) >> "${CONFIG_FILE}"     
echo $(printf 'export CROWNET_CONTROL_IDE_CONT_TAG="%s"' $CROWNET_CONTROL_IDE_CONT_TAG) >> "${CONFIG_FILE}"     

cat "${CONFIG_FILE}"
echo "---------------------- CONFIGURATION END --------------------------"
echo "Finished writing. See ${CONFIG_FILE} for the new configuration."
echo ""

