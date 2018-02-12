#!/bin/bash

helpInfo () {
	echo 'Usage: ./make-assist [build mode] [load mode]'
	echo 'build mode:'
	echo -e ' \t -make to build preprocessed stuff'
	echo -e ' \t -clean to clean up'
	echo 'load mode (Not yet implemented)':
	echo -e ' \t  -cd for loading from the cd dir'
	echo -e ' \t  -sd for loading from an ext2 formatted sd card'
	exit
}

packerSheet () {	#$3 is the format
	cd "$1"

	name=$(echo $1 | cut -d'.' -f 1)
	pngs=$(echo $PWD/*.png)
	TexturePacker --sheet "$2/$name.crayon_temp.png" --format gideros --data "$2/$name.crayon_temp.txt" --size-constraints POT --max-width 1024 --max-height 1024 --pack-mode Best --disable-rotation --trim-mode None --trim-sprite-names --algorithm Basic --png-opt-level 0 --extrude 0 --disable-auto-alias $pngs	#I need pngs to not be in quotes for it to work here

	texconv -i "$2/$name.crayon_temp.png" -o "$2/$name.dtex" -f "$3"

	#Make the new and improved txt file based on the dims of the original png's and the packer txt (Also crop name to remove last 2 fields)
	cat "$2/$name.crayon_temp.txt" | tr -d ',' | while read VAR ; do #I assume the file name has no command in it because its stupid
		textFileName="$(echo $VAR | cut -d' ' -f 1)"
		textFileName+=".crayon_anim"	#Might change to crayon_anim later
		if [[ -f "$textFileName" ]];then #If theres a txt, then append its content to the end
			VAR+=" $(cat $textFileName)"
		else	#If there was no txt file, we assume its just a normal sprite (aka 1 frame animation)
			width="$(file rose.png | rev | cut -d' ' -f 6 | rev)"
			height="$(file rose.png | rev | cut -d' ' -f 4 | rev | cut -d',' -f 1)"
			VAR+=" $width $height 1"
		fi
		echo "$VAR" >> "$2/$name.txt"
	done

	#We remove old packer png and txt
	rm "$2/$name.crayon_temp.png"
	rm "$2/$name.crayon_temp.txt"

	cd ..
}

buildPreProcessed () {	#$1 is asset, $2 is projectRoot/pack, $3 is the current field
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
		    	rm -R "$2/$newX"	#Remove this line to check if the pre-processing stuff worked
		    	cd "$back"

			elif [ "$crayonField" -le 7 ];then	#If we're on the x.FORMAT.crayon_packer_sheet dir
				packerSheet "$x" "$2" "$crayonCheckSecond"	#This builds the spritesheet then converts it to a dtex
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

buildMode=-1	#-1 = unknown, 0 = make, 1 = clean
projectRoot="$PWD"	#Make sure command is called from the real project root
assets="assets"
pack="cdfs"

if [ $# = 0 ];then
	echo 'No params, try "-h" for help'
	exit
fi

while test ${#} -gt 0
do
	if [ "$1" == "-make" ];then
		if [ "$buildMode" -ne -1 ];then
			echo 'Please check your paramters (build mode). Try -h parameter for help'
			exit
		fi
		buildMode=0
		shift
	elif [ "$1" == "-clean" ];then
		if [ "$buildMode" -ne -1 ];then
			echo 'Please check your paramters (build mode). Try -h parameter for help'
			exit
		fi
		buildMode=1
		shift
	elif [ "$1" == "-h" ];then
		helpInfo
	else
		echo 'Please check your paramters Try -h parameter for help'
		exit
	fi
done

if [ "$buildMode" == -1 ];then	#This code will never be activated, will it?
	echo 'No build mode selected'
	exit
fi

if [ "$buildMode" == 0 ];then
	buildPreProcessed "$assets" "$projectRoot/$pack"
else
	cd "$pack"
	rm -R *
	cd ..
fi

exit

#crayon fields:

#So far we check for files/dirs that have the fields crayon_anim, crayon_packer_sheet, crayon_img and crayon_gz. Heres how they all behave

#crayon_img: Make the same dir in the pack dir, but without the crayonCheck field. Romdisks may contain more romdisks

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
