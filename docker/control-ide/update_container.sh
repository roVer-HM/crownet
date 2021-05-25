#!/bin/bash
CONTAINER='flowcontrol-ide'
IDE_URL="https://download.jetbrains.com/python/pycharm-professional-2021.1.1.tar.gz"
IDE_TAR="pycharm.tar.gz"
CACHE="~/.cache/crownet"

if [[ ! -d "$CACHE" ]]; then
  mkdir --verbose --parent $CACHE
fi

if [[ ! -f "$CACHE/$IDE_TAR" ]]; then
    wget $IDE_URL -O "$CACHE/$IDE_TAR"
fi

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
$DIR/../../scripts/upd_container.sh $CONTAINER

