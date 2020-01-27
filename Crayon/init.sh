#!/bin/bash

if ! [ -z "$CRAYON_BASE2" ]; then	#Checking if the var present, if so its already initialised
	exit 0
fi

script_dir="$( cd "$( echo "${BASH_SOURCE[0]%/*}" )"; pwd )"

pattern=" "
if [[ $script_dir =~ "$pattern" ]]; then	#path contains a space, that will cause issues with a makefile
	echo "The Crayon repository is not in a valid path, make sure there are no spaces in your path name"
	echo "Path name: $script_dir"
	exit 1
fi

line="export CRAYON_BASE=\"$script_dir\""

# echo "$line" >> ~/.profile

#Build the dreamcast version of Crayon
make PLATFORM=dreamcast

#Install ALdc as a KOS PORT if it isn't already
if [[ ! -d "$KOS_PORTS/ALdc" ]]; then
	$script_dir/add-ALdc-2-kos-ports.sh
fi

#Make the binaries folder if it doesn't already exist (It should already be in the path)
bins="$KOS_BASE/../bin"
if [ ! -d "$bins" ]; then
  mkdir -p "$bins";
fi

user_dir=$(pwd)
temp_dir="$script_dir/temp"

#Install cdi4dc
repo_path="$temp_dir/img4dc"
sudo apt-get install cmake
git clone https://github.com/Kazade/img4dc.git "$repo_path"
if [ -d "$repo_path" ]; then
	cd "$repo_path"
	echo "$(pwd)"
	cmake .
	make
	cp "cdi4dc/cdi4dc" "$bins"
	cd "$user_dir"
fi

#Install texconv
repo_path="$temp_dir/texconv"
sudo apt-get install qt5-default qtbase5-dev
git clone https://github.com/tvspelsfreak/texconv "$repo_path"
if [ -d "$repo_path" ]; then
	cd "$repo_path"
	qmake
	make
	cp "texconv" "$bins"
	cd "$user_dir"
fi

#Install texturepacker
#-

#Install Crayon-Utilities (clone in same dir as Crayon, compile the exeutables then copy them to bins)
repo_path="$temp_dir/crayon-utilities"
sudo apt-get install libpng-dev
git clone https://github.com/Protofall/Crayon-Utilities "$repo_path"
if [ -d "$repo_path" ]; then
	cd "$repo_path"

	cd "VmuEyeCatchCreator"
	make
	cp "VmuEyeCatchCreator" "$bins"
	cd "../"

	cd "VmuLcdIconCreator"
	make
	cp "VmuLcdIconCreator" "$bins"
	cd "../"

	cd "VmuSFIconCreator"
	make
	cp "VmuSFIconCreator" "$bins"
	cd "../"

	cd "$user_dir"
fi

echo "--Crayon Installed--"
