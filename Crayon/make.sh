#!/bin/bash

helpInfo () {
	echo 'Usage: ./make.sh [build mode] [platform] [boot mode] [other]'
	echo 'build mode:'
	echo -e ' \t -preprocess to build preprocessed stuff'
	echo 'platform:'
	echo -e ' \t -dreamcast to make a dreamcast build'
	echo -e ' \t boot mode:'
	echo -e ' \t \t  -cd for loading from the cd directory'
	echo -e ' \t \t  -sd for loading from an ext2 formatted sd card'
	echo 'other:'
	echo -e ' \t -noRM Prevents the removal of temperary files for viewing'
	echo -e ' \t -clean to clean up'
	exit 0
}

packerSheet () {	#$3 is the format (Eg. RGB565)
	cd "$1"

	name=$(echo $1 | cut -d'.' -f 1)
	pngs=$(echo $PWD/*.png)
	TexturePacker --sheet "$2/$name.crayon_temp.png" --format gideros --data "$2/$name.crayon_temp.txt" --size-constraints POT --max-width 1024 --max-height 1024 --pack-mode Best --disable-rotation --trim-mode None --trim-sprite-names --algorithm Basic --png-opt-level 0 --extrude 0 --disable-auto-alias $pngs	#I need pngs to not be in quotes for it to work here

	#Texconv needs x by x images, so if width != height, we must update them to the larger one
	dims=$(identify "$2/$name.crayon_temp.png" | cut -d' ' -f3)
	dimW=$(echo $dims | cut -d'x' -f1)
	dimH=$(echo $dims | cut -d'x' -f2)
	if (( dimW != dimH ));then
		if (( dimH < dimW ));then
			convert "$2/$name.crayon_temp.png" -background none -extent "x"$dimW "$2/$name.crayon_temp.png"
		else
			convert "$2/$name.crayon_temp.png" -background none -extent "$dimH" "$2/$name.crayon_temp.png"
		fi
	fi

	texconv -i "$2/$name.crayon_temp.png" -o "$2/$name.dtex" -f "$3"

	pngCount=$(wc -l "$2/$name.crayon_temp.txt" | xargs | cut -d' ' -f 1)
	echo "$pngCount" >> "$2/$name.txt"

	#Make the new and improved txt file based on the dims of the original png's and the packer txt (Also crop name to remove last 2 fields)
	cat "$2/$name.crayon_temp.txt" | tr -d ',' | while read VAR ; do #I assume the file name has no command in it because its stupid
		VAR=$(echo "$VAR" | cut -d' ' -f 1,2,3,4,5)	#This removes the 4 last fields that are never used
		textFileName="$(echo $VAR | cut -d' ' -f 1).crayon_anim"
		if [[ -f "$textFileName" ]];then #If theres a txt, then append its content to the end
			VAR+=" $(cat $textFileName)"
		else	#If there was no txt file, we assume its just a normal sprite (aka 1 frame animation)
			targetName="$(echo $VAR | cut -d' ' -f 1).png"
			width="$(file $targetName | rev | cut -d' ' -f 6 | rev)"
			height="$(file $targetName | rev | cut -d' ' -f 4 | rev | cut -d',' -f 1)"
			VAR+=" $width $height 1"
		fi
		echo "$VAR" >> "$2/$name.txt"
	done

	#We remove old packer png and txt
	if [ "$4" = 0 ];then
		rm "$2/$name.crayon_temp.png"
		rm "$2/$name.crayon_temp.txt"
	fi

	cd ..
}

buildPreProcessed () {	#$1 is asset, $2 is projectRoot/cdfs, $3 is the current field
	cd "$1"
	ls "$PWD" | while read x; do

		crayonCheck=$(echo $x | rev | cut -d'.' -f 1 | rev)	#Gets the last part of the file or dir name
		crayonCheckSecond=$(echo $x | rev | cut -d'.' -f 2 | rev  | cut -d'.' -f 1)	#Gets the 2nd to last part of the file or dir name

		crayonField=0

		#crayonCheck contains info per bit
			#1st bit is if its gz compressed
			#2nd bit is if its an img
			#3rd bit is if its a packer spritesheet

		if [ "$crayonCheck" = "crayon_img" ] || [ "$crayonCheckSecond" = "crayon_img" ];then
			((crayonField=crayonField+1))
		fi

		if [ "$crayonCheck" = "crayon_gz" ] || [ "$crayonCheckSecond" = "crayon_gz" ];then
			((crayonField=crayonField+2))
		fi

		if [ "$crayonCheck" = "crayon_packer_sheet" ];then
			((crayonField+=4))
		fi

		if [[ -d "$x" ]];then
		    newX="$x"	#The name of the directory we want to make
		    if [ "$crayonField" = 0 ];then
		    	mkdir "$2/$newX"
				buildPreProcessed "$x" "$2/$newX"
		    elif [ "$crayonField" -le 3 ];then	#If we're on the x.crayon_img dir
		    	newX="${newX%.*}"
		    	if [ "$crayonField" = 3 ];then	#If we have the gz field, then there is a total of 2 fields to remove
		    		newX="${newX%.*}"
		    	fi
		    	mkdir "$2/$newX"
		    	buildPreProcessed "$x" "$2/$newX"

		    	#Commands to image-ify it
		    	back="$PWD"
		    	cd "$2"
		    	$($KOS_GENROMFS -f $newX.img -d $2/$newX -v)	#This depends on current dir hence the cd commands
		    	if [ "$crayonField" = 3 ];then	#For gz compressed romdisks
		    		gzip -f -9 "$newX.img"
		    	fi
		    	if [ "$3" = 0 ];then
		    		rm -R "$2/$newX"	#Delete the processed directory
		    	fi
		    	cd "$back"

			elif [ "$crayonField" -le 7 ];then	#If we're on the x.FORMAT.crayon_packer_sheet dir
				packerSheet "$x" "$2" "$crayonCheckSecond" "$3"	#This builds the spritesheet then converts it to a dtex
		    fi
		elif [[ -f "$x" ]];then	#This won't get triggered when within a crayon_packer_sheet dir so no problems there
			newX="$x"
		    if [ "$crayonField" = 2 ];then
		    	ln "$PWD/$x" "$2"
		    	newX=${newX//".crayon_gz"}
		    	mv "$2/$x" "$2/$newX"	#Renaming it
		    	gzip -f -9 "$2/$newX"
		    else
		    	ln "$PWD/$x" "$2"
		    fi
		else
		    echo "$x is not a file or directory, ERROR"
		    exit
		fi
	done

	cd ..
}

buildDreamcastExecutable () {
	files=$(echo $5/code/crayon/dreamcast/*.c)	#Compile the Crayon source code files
	for x in $files; do
		y=${x%.c}
		z=${y##*/}
		$(kos-cc $KOS_CFLAGS -c $y.c -o $PWD/$z.o)
		if [ "$?" == 1 ];then	#Checks to see if an error occurred
			echo "$z.o failed to build"
			exit 1
		fi
	done

	find . -type f -name "*.c" -execdir kos-cc $KOS_CFLAGS -c {} -o $PWD/{}%.c.o \;	#This will compile all files in project dir
	find . -type f -name "*.cpp" -execdir kos-cc $KOS_CFLAGS -c {} -o $PWD/{}%.cpp.o \;	#cpp/c++ files are untested

	ofiles=$(ls *.o)

	if [ "$2" -le 1 ];then #dc-cd and dc-sd
		$($KOS_CC $KOS_LDFLAGS -o $3.elf $KOS_START $ofiles -lz -lm $KOS_LIBS)	#Make the elf
		if [ "$?" == 1 ];then	#Checks to see if an error occurred
			echo "Failed to build the elf"
			exit 1
		fi

		if [ "$2" == 1 ];then	#dc-sd
			$(sh-elf-objcopy -R .stack -O binary $3.elf $1/$3.bin)	#Turn it into a binary and place it in the cdfs dir
		else	#dc-cd
			$(sh-elf-objcopy -R .stack -O binary $3.elf $3.bin)	#Turn it into a binary
			$($KOS_BASE/utils/scramble/scramble $3.bin $1/1st_read.bin)	#scramble
			$(mkisofs -G $4 -C 0,11702 -J -l -r -o $3.iso $1) #IPBIN and name and cdfs
			cdi4dc "$3.iso" "$3.cdi"
		fi
	fi
}

preprocess=0	# 0 for don't build, 1 for build
platform=-1 #-1 for undefined, 0 for Dreamcast
bootMode=-1	#-1 = undefined/none, 0 = dc-cd, 1 = dc-sd
noRM=0	#0 means it will remove temp files, 1 means it won't remove them

NAME=${PWD##*/}	#The name of the program is the same name as the project
IPBIN="$KOS_BASE/../IP.BIN"	#Change this depending on where your IP.BIN file is located
assets="assets"
cdfs="cdfs"
projectRoot="$PWD"	#Make sure bash script is called from the real project root
crayonRoot="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"	#The directory of the crayon source code

if [ $# = 0 ];then
	echo 'No params, try "-h" for help'
	exit 1
fi

while test ${#} -gt 0
do
	if [ "$1" = "-preprocess" ];then
		preprocess=1
		shift
	elif [ "$1" = "-clean" ];then
		rm *.o	#Removes all object files
		rm "$NAME.elf"	#Removes the elf
		rm "$NAME.bin"	#Removes the normal binary
		rm "$NAME.iso"	#Removes the iso
		rm "$NAME.cdi"	#Removes the cdi
		rm -R "$cdfs/"*
		echo "Clean complete"
		exit 0
	elif [ "$1" = "-dreamcast" ];then
		if [ "$platform" -ne -1 ];then
			echo 'Please check your paramters (platform). Try -h parameter for help'
			exit 1
		fi
		platform=0
		shift
		if [ "$1" = "-cd" ];then
			bootmode=0
		elif [ "$1" = "-sd" ];then
			bootmode=1
		else
			echo 'Please check your paramters (bootmode). Try -h parameter for help'
			exit 1
		fi
		shift
	elif [ "$1" = "-noRM" ];then
		noRM=1
		shift
	elif [ "$1" = "-h" ];then
		helpInfo
	else
		echo 'Please check your paramters Try -h parameter for help'
		exit 1
	fi
done

if [ "$preprocess" = 1 ];then
	buildPreProcessed "$assets" "$projectRoot/$cdfs" "$noRM"
fi

if [ "$platform" = 0 ]; then	#Dreamcast
	buildDreamcastExecutable "$cdfs" "$bootMode" "$NAME" "$IPBIN" "$crayonRoot"	#It will build cd or sd depending on bootMode
fi

exit 0

#crayon fields:

#So far we check for files/dirs that have the fields crayon_anim, crayon_packer_sheet, crayon_img and crayon_gz. Heres how they all behave

#crayon_img: Make the same dir in the cdfs dir, but without the crayonCheck field. Romdisks may contain more romdisks

#crayon_gz: If this field is present, once all other crayon fields have been done, the result is compressed into a gz file
	#Known bug: If you have a directory with the crayon_gz field, but no crayon_ig field, then it instead turns it into a romdisk
	#and doesn't compress it. I know how to fix it, but I can't see a reason why someone would do this so until someone can give a
	#good reason why you would want to only gz compress a directory, I won't fix this bug

#crayon_packer_sheet: Only png's and a crayon_anim file of the same name are allowed in this dir. In the cdfs version in place of the
#crayon_packer_sheet dir you will see a .dtex, .txt and possibly .dpal in its place. Other files in the crayon_packer_sheet dir will be
#ignored. Also note not every png has a crayon_anim. If no crayon_anim is found its assumed the png is a regular sprite (1-Frame animation)

#If a non-crayon related file is found it will be hard linked into the corresponding dir in the cdfs dir
#If a non-crayon directory is found (That doesn't contain anything crayon related inside) then run mkdir anyways,
#since folders don't take much space

#Also note the field "crayon_temp" is used by this bash script to help construct the final result and those crayon_temp's are deleted
#once no longer needed

#------------------------------------------------------------------------

#Examples:

#asset: x.crayon_img/
#cdfs: x.img

#asset: x.crayon_img.crayon_gz/
#cdfs: x.img.gz

#asset: x.crayon_gz.txt
#cdfs: x.txt.gz

#asset: x.FORMAT.crayon_packer_sheet/
#cdfs: x.dtex
#      x.txt
#      x.dtex.pal	(Is present depending on what format was chosen)
