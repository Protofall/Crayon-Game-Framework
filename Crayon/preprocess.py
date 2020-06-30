#!/usr/bin/python3

import sys
import os
from os import path

def helpInfo():
	print('Usage: ./preprocess.py -noRM*')
	print('noRM:')
	print('   -noRM is an optional parameter. It prevents the')
	print('   removal of temperary files for viewing/debugging\n')
	print('About:')
	print('   preprocess.py will process your assets and modify')
	print('   components with a valid crayon tag. The processed')
	print('   assets content end up in the cdfs folder\n')
	print('KOSs implementation of GZ could be iffy so be wary')
	print('when using it. Don\'t use it on compressed stuff')
	
	exit(0)

#tags is stuff like Dreamcast compression (So far its just that)
def packerSheet(src_filename, dest_path, pixel_format, tags, noRM):
	#cd src_filename
	
	name = os.path.splitext(src_filename)[0] #Strip the extention
	# https://docs.python.org/3/library/os.path.html#os.path.splitext

#This checks if anything in the assets directory is older than anything
#in the processed directory. If so then we build, else terminate.
def check_age(asset_path, processed_path):
	print("WIP")

#Might be able to get processed_path from asset_path?
def build(asset_path, processed_path, noRM):
	if noRM == True:
		print('noRM TRUE')
	else:
		print('noRM FALSE')

	return 0

def main():
	print("WARNING: This script is an incomplete port/upgrade of preprocess.sh\n")

	noRM = False	#False means it will remove temp files, True means it won't remove them
	project_root = os.getcwd()
	assets_path = os.path.join(project_root, "assets")
	processed_path= os.path.join(project_root, "cdfs")
	
	if os.path.isdir(assets_path) == False:
		print("Can't find assets directory")
		exit(1)

	#check arguments
	help_enabled = False
	bad_input = False
	for arg in sys.argv[1:]:
		if arg == "-noRM":
			noRM = True
		elif arg == "-h" or arg == "--help":
			help_enabled = True
		else:
			bad_input = True;

	if help_enabled:
		helpInfo()

	if bad_input:
		print('Please check your parameters Try -h parameter for help')
		exit(1)

	print("WARNING: GZ instructions haven't been tested in this script")
	print("There's a chance items with a crayon_gz tag won't be processed right\n")
	build(assets_path, processed_path, noRM)

main()



#The below comments are very old and I'm not sure if its accurate or not. Please feel free to ignore it





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

