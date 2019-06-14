#!/bin/bash

helpInfo () {
	echo 'Usage: ./make.sh -noRM*'
	echo 'noRM:'
	echo -e ' \t -noRM is an optional parameter. It prevents the'
	echo -e ' \t removal of temperary files for viewing/debugging'
	echo -e '\n'
	echo 'About:'
	echo ' preprocess.sh will process your assets and modify'
	echo ' components with a valid crayon tag. The processed'
	echo ' assets content end up in the cdfs folder'
	echo -e '\nReports show that KOSs implementation of GZ'
	echo -e 'so be wary when using it'
	exit 0
}

#Parameters: $1 is original file name, $2 is destination path, $3 is the format (Eg. RGB565), $4 is noRM
packerSheet () {
	cd "$1"

	#The filename without the extension
	name=$(echo $1 | cut -d'.' -f 1)
	
	#Passing in the current dir instead of the pngs allows it to use spaced-name pngs
	TexturePacker --sheet "$2/$name.crayon_temp.png" --format gideros --data "$2/$name.crayon_temp.txt" \
	--size-constraints POT --max-width 1024 --max-height 1024 --pack-mode Best --disable-rotation --trim-mode \
	None --trim-sprite-names --algorithm Basic --png-opt-level 0 --extrude 0 --disable-auto-alias "$PWD"	#Also this is where "Resulting sprite sheet" message comes from...for some reason
	
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
	echo "$pngCount" >> "$2/$name.txt"	#Output the number of sprites on top

	#Make the new and improved txt file based on the dims of the original png's and the packer txt (Also crop name to remove last 2 fields)
	cat "$2/$name.crayon_temp.txt" | while read VAR ; do #I assume the file name has no command in it because its stupid
		spriteName=$(echo $VAR | cut -d',' -f 1)	#Extract the name
		VAR=$(echo "$VAR" | cut -d' ' -f 2,3,4,5 | tr -d ',')	#This keeps only the top left x/y and width/height. Also removes commas
		textFileName="$spriteName.crayon_anim"
		if [[ -f "$textFileName" ]];then #If theres a txt, then append its content to the end
			VAR+=" $(cat $textFileName)"
		else	#If there was no txt file, we assume its just a normal sprite (aka 1 frame animation)
			targetName="$spriteName.png"
			width="$(file "$targetName" | rev | cut -d' ' -f 6 | rev)"
			height="$(file "$targetName" | rev | cut -d' ' -f 4 | rev | cut -d',' -f 1)"
			VAR+=" $width $height 1"
		fi
		echo -e "$spriteName\n$VAR" >> "$2/$name.txt"
	done

	#We remove old packer png and txt
	if [ "$4" = 0 ];then
		rm "$2/$name.crayon_temp.png"
		rm "$2/$name.crayon_temp.txt"
	fi

	cd ..
}

#Parameters: $1 is asset, $2 is projectRoot/cdfs, $3 is the "noRM" flag
build () {
	cd "$1"
	ls "$PWD" | while read x; do

		crayonField=0
		#crayonCheck contains info per bit
			# 0th bit is if its gz compressed file/dir
			# 1st bit is if its an img dir
			# 2nd bit is if its a packer spritesheet dir
			# 3rd bit is if its a standalone texconv request

		#What shouldn't be valid
			# An ".img" with anything other than a gz
			# Multiple texconv tags
			# A packer request without a texconv request
			# A packer sheet on a file

		texconvFormat="NONE"
		#texconvFormat contains the format id
			# None = -1
			# ARGB1555 = 0
			# RGB565 = 1
			# ARGB4444 = 2
			# YUV422 = 3
			# BUMPMAP = 4
			# PAL4BPP = 5
			# PAL8BPP = 6

		outputName=""	#The name of the file/dir without crayon_tags

		oldIFS=$IFS
		IFS='.'	#This basically makes it so we tokenise based on the fullstop instead of the newline char
		for part in $x
		do
			if [ "$part" = "crayon_gz" ];then
				crayonField=$(($crayonField|$((1 << 0))))
			elif [ "$part" = "crayon_img" ];then
				crayonField=$(($crayonField|$((1 << 1))))
			elif [ "$part" = "crayon_spritesheet" ];then
				crayonField=$(($crayonField|$((1 << 2))))
			elif [ "$part" = "ARGB1555" ] || [ "$part" = "RGB565" ] || [ "$part" = "ARGB4444" ] ||
				[ "$part" = "YUV422" ] || [ "$part" = "BUMPMAP" ] || [ "$part" = "PAL4BPP" ] ||
				[ "$part" = "PAL8BPP" ];then
				if [ $(($crayonField & $((1<<3)) )) -ne 0 ];then	#Prevents multiple format tags
					echo "	ERROR: You can't use multiple texconv formats"
					IFS=$oldIFS	#Restore IFS, dunno if necessary for an exit
					exit 1
				fi
				texconvFormat="$part"
				crayonField=$(($crayonField|$((1 << 3))))
			else
				outputName+="$part".	#Only do this on a non-crayon tag
			fi

			if [ $(($crayonField & $((1 << 1)) )) -ne 0 ] && [ $crayonField -ne $((1 << 1)) ] &&
				[ $crayonField -ne $(( $((1 << 1)) + $((1 << 0)) )) ];then	#If we're trying to image-ify with invalid tags, error
				echo "	ERROR: If using crayon_img tag, you can only also have a crayon_gz tag"
				IFS=$oldIFS
				echo "	Filename: " $x
				exit 1
			fi
		done

		IFS=$oldIFS	#Restore IFS

		outputName=$(echo $outputName | sed 's/.$//')	#Remove the trailing character (A full stop hopefully)

		if [[ -d "$x" ]];then	#If directory

			if [ $(($crayonField & $((1 << 2)))) -ne 0 ];then	#If we have a packer sheet tag
				if [ "$texconvFormat" = "NONE" ];then	#If we don't have a texconv field
					echo "	ERROR: $x doesn't have a texture tag"
					exit 1
				fi
				packerSheet "$x" "$2" "$texconvFormat" "$3"	#This builds the spritesheet then converts it to a dtex
			else
				mkdir -p "$2/$outputName"

				build "$x" "$2/$outputName" "$3"

				if [ $(($crayonField & $((1 << 1)))) -ne 0 ];then	#If we have an image tag
					back="$PWD"
					cd "$2"
					$("$KOS_GENROMFS" -f "$outputName.img" -d "$2/$outputName")	#This depends on current dir hence the cd commands
					if [ "$3" = 0 ];then
						rm -R "$2/$outputName"	#Delete the processed dir
					fi
					cd "$back"
					outputName="$outputName.img"	#Change the output name for the gz command
				fi
				if [ $(($crayonField & $((1 << 0)))) -ne 0 ];then	#If we have a gz tag
					gzip -f -9 "$outputName"
				fi
			fi

		elif [[ -f "$x" ]];then	#If its a file

			ln "$PWD/$x" "$2"	#Create a copy that we'll process
			if [ "$x" != "$outputName" ];then	#If they're the same, we get an annoying message
				mv "$2/$x" "$2/$outputName"	#Renaming it
			fi

			if [ "$texconvFormat" != "NONE" ];then	#No GZ compression, but we want to convert the asset
				texconv -i "$2/$outputName" -o "$2/${outputName%.*}.dtex" -f $texconvFormat
				echo "$2/$outputName" "$2/$outputName.dtex"
				if [ "$3" = 0 ];then
					rm "$2/$outputName"	#Delete the copied png
				fi
			fi

			if [ $(($crayonField & $((1 << 0)))) -ne 0 ];then	#GZ compress it
				# Add an rm check here
				gzip -f -9 "$2/$outputName"
			fi

			if [ $(($crayonField)) -eq $((1 << 1)) ] || [ $(($crayonField)) -eq $((1 << 2)) ] ||
				[ $(($crayonField)) -eq $(((1 << 2) + (1 << 1))) ];then	#If its img or packer, thats invalid
				echo "	ERROR: Incompatible tags on file: $x"
				exit 1
			fi

		else
			echo "	ERROR: $x is not a file or directory"
			exit 1
		fi

	done
	errorCode=$?
	if [[ $errorCode != 0 ]]; then
		exit $errorCode
	fi

	cd ..
}

noRM=0	#0 means it will remove temp files, 1 means it won't remove them
assets="assets"
cdfs="cdfs"
projectRoot="$PWD"	#Make sure bash script is called from the real project root

while test ${#} -gt 0
do
	if [ "$1" = "-noRM" ];then
		noRM=1
		shift
	elif [ "$1" = "-h" ];then
		helpInfo
	else
		echo 'Please check your parameters Try -h parameter for help'
		exit 1
	fi
done

echo "WARNING: GZ instructions haven't been tested in this script"
echo -e "There's a chance items with a crayon_gz tag won't be processed right\n"
echo -e "I haven't tested the BUMPMAP texture format, but it should \"work\"\n"
build "$assets" "$projectRoot/$cdfs" "$noRM"

exit 0



#The below comment is very old and I'm not sure if its accurate or not. Please feel free to ignore it





#crayon fields:

#So far we check for files/dirs that have the fields crayon_anim, crayon_spritesheet, crayon_img and crayon_gz. Heres how they all behave

#crayon_img: Make the same dir in the cdfs dir, but without the crayonCheck field. Romdisks may contain more romdisks

#crayon_gz: If this field is present, once all other crayon fields have been done, the result is compressed into a gz file
	#Known bug: If you have a directory with the crayon_gz field, but no crayon_ig field, then it instead turns it into a romdisk
	#and doesn't compress it. I know how to fix it, but I can't see a reason why someone would do this so until someone can give a
	#good reason why you would want to only gz compress a directory, I won't fix this bug

#crayon_spritesheet: Only png's and a crayon_anim file of the same name are allowed in this dir. In the cdfs version in place of the
#crayon_spritesheet dir you will see a .dtex, .txt and possibly .dpal in its place. Other files in the crayon_spritesheet dir will be
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

#asset: x.FORMAT.crayon_spritesheet/
#cdfs: x.dtex
#      x.txt
#      x.dtex.pal	(Is present depending on what format was chosen)
