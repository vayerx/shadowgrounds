#!/bin/bash

if [ $# != 1 ]; then
    echo "Usage: $0 target_path"
    exit -1
else
    DIRS=(Resource)
    FILES=(launcher.pl SGConfig.pm SGOptions.pm LauncherWindow.pm LauncherWindow.glade LauncherImage.bmp)
    TARGET_PATH=$1
    for d in ${DIRS[@]}; do
	echo "Making directory: $TARGET_PATH/$d"
	mkdir -p "$TARGET_PATH/$d"
    done
    for f in ${FILES[@]}; do
	echo "Copying $f to $TARGET_PATH/$f"
	cp "$f" "$TARGET_PATH/$f"
    done
fi
