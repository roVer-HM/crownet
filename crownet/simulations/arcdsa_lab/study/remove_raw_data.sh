#!/bin/bash
canonical_path=$(pwd -P)

find ${canonical_path}/../results -type f -name "*.vec" -exec ls {} +
find ${canonical_path}/../results -type f -name "*.vci" -exec ls {} +

echo "Do you really want to remove all these raw data files? (y/n)"
read -r response
case "$response" in
    [yY][eE][sS]|[yY]) 
        echo "Removing all raw vector files..."
        find ${canonical_path}/../results -type f -name "*.vec" -exec rm -f {} +
        find ${canonical_path}/../results -type f -name "*.vci" -exec rm -f {} +
        ;;
    *)
        echo "Operation aborted."
        exit 1
        ;;
esac