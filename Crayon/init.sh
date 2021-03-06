#!/bin/bash

if ! [ -z "$CRAYON_BASE" ]; then	# Checking if the var present, if so its already initialised
	exit 0
fi

script_dir="$( cd "$( echo "${BASH_SOURCE[0]%/*}" )"; pwd )"

# TODO: Remove this in the scons update
pattern=" "
if [[ $script_dir =~ "$pattern" ]]; then	# Path contains a space, that will cause issues with a makefile
	echo "The Crayon repository is not in a valid path, make sure there are no spaces in your path name"
	echo "Path name: $script_dir"
	exit 1
fi

# Build the dreamcast version of Crayon
	# TODO: When the SCons changes are done, this line won't be required
make PLATFORM=dreamcast

# Install ALdc2 as a KOS PORT if it isn't already
if [[ ! -d "$KOS_PORTS/ALdc" ]]; then
	$script_dir/add-ALdc2-to-kos-ports.sh
fi

# Make the binaries folder if it doesn't already exist
bins="$script_dir/bins"
if [ ! -d "$bins" ]; then
  mkdir -p "$bins";
fi

user_dir=$(pwd)
temp_dir="$script_dir/temp"

# Install cdi4dc
repo_path="$temp_dir/img4dc"
if [ ! -d "$repo_path" ]; then
	sudo apt install cmake
	git clone https://github.com/Kazade/img4dc.git "$repo_path"
	if [ -d "$repo_path" ]; then
		cd "$repo_path"
		echo "$(pwd)"
		cmake .
		make
		cp "cdi4dc/cdi4dc" "$bins"
		cd "$user_dir"
	fi
fi

# Install texconv
repo_path="$temp_dir/texconv"
if [ ! -d "$repo_path" ]; then
	sudo apt install qt5-default qtbase5-dev
	git clone https://github.com/tvspelsfreak/texconv "$repo_path"
	if [ -d "$repo_path" ]; then
		cd "$repo_path"
		qmake
		make
		cp "texconv" "$bins"
		cd "$user_dir"
	fi
fi

# Install texturepacker
dpkg-query -l 'texturepacker' > "$temp_dir/garbage.txt"
res=$?
rm "$temp_dir/garbage.txt"
if [ $res == 1 ]; then
	wget -O "$temp_dir/texturepacker.deb" "https://www.codeandweb.com/download/texturepacker/5.2.0/TexturePacker-5.2.0-ubuntu64.deb"
	sudo apt install "$temp_dir/texturepacker.deb"
fi

# Install Crayon-Utilities (clone in same dir as Crayon, compile the exeutables then copy them to bins)
repo_path="$temp_dir/crayon-utilities"
if [ ! -d "$repo_path" ]; then
	sudo apt install libpng-dev
	git clone https://github.com/Protofall/Crayon-Utilities "$repo_path"
	if [ -d "$repo_path" ]; then
		cd "$repo_path"

		scons
		cp "DreamcastEyecatcherTool/DreamcastEyecatcherTool" "$bins"
		cp "VmuLcdBitmapTool/VmuLcdBitmapTool" "$bins"
		cp "DreamcastSavefileIconTool/DreamcastSavefileIconTool" "$bins"

		cd "$user_dir"
	fi
fi

# Finally set these environment variables (We do it here just incase something above throws an error)
echo "export CRAYON_BASE=\"$script_dir\"" >> ~/.profile

# Add the binaries folder to PATH variable
echo "export PATH=\"\$PATH:$bins\"" >> ~/.profile

echo "--Crayon Installed--"
