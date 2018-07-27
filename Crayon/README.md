# Crayon, framework for a new KOS project

### Priority:

+ ! = Can wait till after final Minesweeper build
+ !! = Can wait till after initial Minesweeper demo
+ !!! = Most important, needed for initial Minesweeper demo

### Requirements:

+ (DONE) Romdisk Swapping support (!!!)
+ (DONE) Spritesheet support (!!!)
+ Rendering lists of ease of use (!!!)
+ (DONE) 4bpp loading/rendering support (!!!)
+ (DONE BY DEFAULT?) Maple/Controller support (!!!)
+ Palette entry modification support (!!!)
+ (Error_Freeze() Isn't enough) Error display support (!!!)
+ Video mode support (!!!)
+ Selector to easily choose between "sd" and "cd" dirs (cd for CD-Rs, sd for the sdloader method) (!!)
+ Threading support (!!)
+ VGA/Progressive Scan support (!!)
+ (DONE) Non paletted ARGB4444 loading/rendering support (!!)
+ VMU save/load support (!!)
+ VMU draw to screen support (!!)
+ Sound support (!!)
+ (DONE) 8bpp loading/rendering support (!)
+ (DONE) Non paletted RGB565 loading/rendering support (!)
+ RAM/VRAM (And SRAM?) space debug (!)

## Instructions on usage

As of now when making a new project, you'll need to link the exact path to a crayon library (Unlike in real library where you can just do "graphics.h"). Also I think your project directory and the Crayon directory need to have the same parent directory unless you modify the makefile to properly call Crayon's bash file

Crayon's root:

	+ code	//Contains crayon library stuff
	+ makefile
	+ make.sh

Your Project's root:

	+ assets/
	+ cdfs/
	+ makefile	//Same makefile from Crayon directory

assets can contain a normal file system, but files/folders with crayon_x fields will be pre-processed. Basically the stuff inside there dirs will get processed and the processed result will be the cdfs directory.

(Explain later what each field means)

The picture formats are PAL4BPP, PAL8BPP, RGB565 (Opqaue), ARGB4444 (Transparent) or ARGB1555 (Punch-Through).

Note: Its up to the artist/developers to make sure that the art assets meet the BPP requirements (OPAQUE and TRANSPARENT are 16BPP) and that the romdisk/textures/sound files will all fit in the Dreamcast's RAM nicely (Not crash)

The Crayon/code/ directory contains all the .c and .h crayon files used by the program, and every .c/.cpp file in "Your Project"s directory will also be compiled

### In assets

assets/

```
|-> plants.crayon_img/
      |-> flowers.PAL4BPP.crayon_packer_sheet/
      |     |-> rose.png
      |     |-> rose.crayon_anim
      |     |-> tulip.png
      |     |-> tulip.crayon_anim
      |-> lol.txt
```

### In cdfs (If we could explore .img's)

```
cdfs/
|-> plants.img/
      |-> flowers.dtex
      |-> flowers.dtex.dpal
      |-> flowers.txt
      |-> lol.txt
```

### In Dreamcast/KOS VFS

```
cd/(mount name)	#Note, this might be attached to the sd dir instead
  |-> flowers.dtex
  |-> flowers.dtex.dpal
  |-> flowers.txt
  |-> lol.txt
```

Notes:

	+ crayon_x field is a hint to do something with the file/dir in preprocessing
	+ The crayon_anim found in crayon_packer_sheet dirs contains "frame_width frame_height number_of_sprites" and is used to help make the final txt for that dir. It also assumes each frame has the same width and height and all the frames are tightly packed in an 2D representation of a 1D array-like grid
	+ A dir with the crayon_packer_sheet field may only contain n png's and n or less crayon_anim's (Each crayon_anim has a png of the same name, but if a png doesn't have a corresponding crayon_anim then its assumed the png is a single frame/just a sprite). Any other files and directories will be ignored
	+ files that aren't involved in preprocessing (Except gziped ones) are hardlinked
	+ crayon_img means "Make this into a romdisk"
	+ crayon_gz means "GZ compress this file/romdisk", can be combined with crayon_img
	+ **TODO** Make crayon_ignore so the bash script doesn't explore those directories
	+ **TODO** Make crayon_PAL4BPP and crayon_8BPP to generate only the palette file everything in the dir (Think more how exactly this will work to make it intuitive so it can be used for easily swappable palettes)

### Code Structure

First some definitions:

+ A spritesheet (SS): Contains 1 or more animations
+ An animation: Contains 1 or more sprites/frames
+ A sprite is the individual sub-textures/UV-coords-of-texture that you see when playing the game

Ofc we have the basics, romdisk un/mount, setup_palette, all the draw functions for each format, load_texture(DTEX \*, DPAL \*) (Nothing is passed in for the DPAL pointer if its not paletted). Each texture it can load must be in the .dtex or .dtex.pal formats to work with the load texture functions.

Each .dtex is actually a SS that contains 1 or more animations. So here's my proposed structs for storing the SS info **NOTE TO SELF** This has probably changed a bit, update it later

```c
ss{
	pvr_ptr_t * ss_texture;
	anim * ss_anims;	//Assigned with dynamic array
	uint16_t ss_dims;	//Since a pvr texture must be a square, we don't need height/width
	uint8_t ss_format; //1 for 4BPP, 2 for 8BPP, 3 for RGB565 and 4 for ARGB4444 (0 for unknown)
	palette * ss_palette_id; //If it uses a palette, here is where you assign it's palette
}

anim{
	char * anim_name;
	uint8_t anim_frames;	//How many sprites make up the animation (Dunno if this should be a short or int yet)
	uint16_t anim_top_left_x_coord;	//Since the anims are designed to be in tiles
	uint16_t anim_top_left_y_coord;	//We can select an frame based off of the first frame's coords
	uint16_t anim_sheet_width;	//Width of the anim sheet
	uint16_t anim_sheet_height;
	uint16_t anim_frame_width;	//Width of each frame
	uint16_t anim_frame_height;
	//With these widths and heights, it might be possible to deduce some of them from other info, but idk
}
```

I'm not 100% sure about the data types for the anim struct (I don't think pairs exist in regular C). However this gets the job done. The ss structs will be stored in a linked list. There will also be rendering linked lists like the one below.

```c
object{
	ss * spritesheet;
	anim * animation;
	uint8_t currentFrame;
	int drawX;
	int drawY;
	int drawZ;
	uint8_t viewID;
	object * next;
	uint8_t objectLogicID;	//Might need to make this a uint16_t later, idk
}
```

It knows whether an object is opaque or transparent based on the ss->ss_format. currentFrame is used to tell it which sprite to draw from the animation, drawX/Y/Z are just the draw coordinates relative to the view. The viewID variable tells it which view we are drawing relative to. Prior to rendering we set up some view structs like so

```c
view{
	uint16_t top_left_x_coord;
	uint16_t top_left_y_coord;
	uint16_t height;
	uint16_t width;
}
```

This would be useful for splitscreen (And maybe views too?). I feel this view idea in its current state is very primative so I hope to see it develop over time.

objectLogicID is basically the general behaviour of a texture. So a background has none, but a sprite might have weight, immunity level, etc. The id is used to refer to the "logic list" that I'll decide on its specifics later.

**Note to self:** The view struct probably won't be a part of Crayon and instead part of the project code

### Pre-requisites

+ General knowledge of C
+ KallistiOS
+ TexturePacker
+ Texconv
	+ QT is required to build the Texconv executable
+ cdi4dc (Unix build)

### Installing Pre-requisites

Assuming you have a working clone of KOS.

For cdi4dc:
`git clone https://github.com/Kazade/img4dc.git`
`cd img4dc`
`cmake`
`make`

For texconv:
`sudo apt-get install qt5-default qtbase5-dev`
`git clone https://github.com/tvspelsfreak/texconv`
`cd texconv`
`qmake`
`make`

Copy the executables for texconv and cdi4dc into a directory whose path is listed in the ~.profile file (Or add you own directory path)

**CHECK** Do we also require the `convert` command?

For TexturePacker:
+ Go onto their website and download the free .deb file
+ Install it
+ Run it once, read the legal stuff (Or not) and type "agree"

If I remembered that correctly, now you should have an up and running Crayon framework.

### Random notes

Force vid_set_mode(DM_640x480_VGA, PM_RGB565); for VGA mode...I think RGB888 might not be compatible with the pvr system (Only RGB565)

Sprite mode might only be able to reliably do a maximum dimension of 256 by 256 textures. Any larger could cause "Jitter" effects

### Useful for identifying colour count

`identify -format %k "filename" && echo` (includes alphas as seperate colours)

