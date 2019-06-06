#!/bin/bash

LOCALDIR=`pwd`
WIKINAME=rover-main.wiki.git

echo "This script installs all dependencies for locally editing the project wiki."
echo "Afterwards, the wiki will be cloned to $LOCALDIR/$WIKINAME and gollum will be started."
echo ""
echo "While gollum is running, open a browser on localhost:4567 to edit the wiki."
echo ""
echo "[ENTER] to continue, [Strg-C] to interrupt."
read

sudo apt install ruby ruby-dev
sudo gem install gollum
sudo gem install github-markdown

git clone ssh://git@sam-dev.cs.hm.edu:5022/rover/rover-main.wiki.git
cd rover-main.wiki
gollum

