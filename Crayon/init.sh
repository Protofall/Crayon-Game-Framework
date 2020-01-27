#!/bin/bash

if ! [ -z "$CRAYON_BASE2" ]	#Checking if the var present, if so its already initialised
then
	exit 0
fi

script_dir="$( cd "$( echo "${BASH_SOURCE[0]%/*}" )"; pwd )"

pattern=" "
if [[ $script_dir =~ "$pattern" ]]	#path contains a space, that will cause issues with a makefile
then
	echo "The Crayon repository is not in a valid path, make sure there are no spaces in your path name"
	echo "Path name: $script_dir"
	exit 1
fi

line="export CRAYON_BASE=\"$script_dir\""

# echo "$line" >> ~/.profile

#Build the dreamcast version of Crayon
make PLATFORM=dreamcast

#Install ALdc as a KOS PORT if it isn't already
if [[ ! -d "$KOS_PORTS/ALdc" ]]
then
	$script_dir/add-ALdc-2-kos-ports.sh
fi

#Make the binaries folder if it doesn't already exist (It should already be in the path)
bins="$KOS_BASE/../bin"
if [ ! -d "$bins" ]; then
  mkdir -p "$bins";
fi

#Install texturepacker, texconv and cdi4dc
#-

#Install Crayon-Utilities (clone in same dir as Crayon, compile the exeutables then copy them to bins)
#-

echo "--Crayon Installed--"
