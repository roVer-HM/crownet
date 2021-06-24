#!/bin/bash
CONTAINER='vadere-ide'
IDE_URL="https://download.jetbrains.com/idea/ideaIU-2021.1.2.tar.gz"
#  community version: use IDE_URL="https://download.jetbrains.com/idea/ideaIC-2021.1.2.tar.gz"

IDE_TAR="intellij.tar.gz"
CACHE="~/.cache/crownet"

if [[ ! -d "$CACHE" ]]; then
  mkdir --verbose --parent $CACHE
fi

if [[ ! -f "$CACHE/$IDE_TAR" ]]; then
    wget $IDE_URL -O "$CACHE/$IDE_TAR"
fi

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
$DIR/../../scripts/upd_container.sh $CONTAINER

