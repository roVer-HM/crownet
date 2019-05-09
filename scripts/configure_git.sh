#!/bin/bash
echo "This script sets the git options recommended for the roVer project:"
echo "   - show log entries when diffing a submodule"
echo "   - push submodules on demand"
echo "Aliases:"
echo "sdiff       : diff for main project and all submodules"
echo "spush       : push all submodules"
echo "supdate     : update all submodules"
echo "sstash      : stash all changes in submodules"

git config diff.submodule log
git config push.recurseSubmodules on-demand

git config alias.sdiff '!'"git diff && git submodule foreach 'git diff'"
git config alias.spush 'push --recurse-submodules=on-demand'
git config alias.supdate 'submodule update --remote --merge'
git config alias.sstash  "submodule foreach 'git stash'"

