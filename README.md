# Crayon, framework for a new KOS project

### Priority:

+ ! = Can wait till after final Minesweeper build
+ !! = Can wait till after initial Minesweeper demo
+ !!! = Most important, needed for initial Minesweeper demo

### Requirements:

+ (DONE) Romdisk Swapping support (!!!)
+ (DONE) Spritesheet support (!!!)
+ (DONE) 4bpp loading/rendering support (!!!)
+ (DONE BY DEFAULT?) Maple/Controller support (!!!)
+ (DONE? (Added font support)) (Error_Freeze() Isn't enough) Error display support (!!!)
+ Video mode support (!!!)
+ Projection/view/camera support (!!!)
+ Rendering lists of ease of use (!!)
+ (DONE BY DEFAULT?) Threading support (!!)
+ (DONE) VGA/Progressive Scan support (!!)
+ (DONE) VMU save/load support (!!)
+ (DONE? Not greyscale yet) VMU draw to screen support (!!)
+ (DONE) Sound effect support (!!)
+ BG music support (!!)
+ (DONE) 8bpp loading/rendering support (!!)
+ (DONE) Non paletted RGB565 loading/rendering support (!!)
+ (DONE) Selector to easily choose between "sd" and "cd" dirs (cd for CD-Rs, sd for the sdloader method) (!)
+ (DONE) Non paletted ARGB4444 loading/rendering support (!)
+ Palette entry modification support (!)
+ RAM/VRAM (And SRAM?) space debug (!)

## Instructions on usage

As of now when making a new project, copy over this makefile, edit the "crayon_path" variable and make new asset, cdfs and code folders. Crayon currently doesn't behave like a real library, instead you must include the files you want with their correct path (Unlike in real library where you can just do "graphics.h")

Crayon's root:

	+ code	//Contains crayon library stuff
	+ makefile
	+ preprocess.sh

Your Project's root:

	+ assets/
	+ cdfs/
	+ code/		//Your project's code files
	+ makefile	//Same makefile from Crayon directory

assets can contain a normal file system, but files/folders with crayon_x fields will be pre-processed. Basically the stuff inside there dirs will get processed and the processed result will be the cdfs directory.

(Explain later what each field means)

The picture formats are PAL4BPP, PAL8BPP, RGB565 (Opqaue), ARGB4444 (Transparent) and ARGB1555 (Punch-Through).

Note: Its up to the artist/developers to make sure that the art assets meet the BPP requirements (OPAQUE and TRANSPARENT are 16BPP) and that the romdisk/textures/sound files will all fit in the Dreamcast's RAM nicely (Not crash)

The Crayon/code/ directory contains all the .c and .h crayon files used by the program, and every .c/.cpp file in your Project's directory will also be compiled

### In assets

assets/

```
|-> plants.crayon_img/
      |-> flowers.PAL4BPP.crayon_spritesheet/
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
	+ The crayon_anim found in crayon_spritesheet dirs contains "frame_width frame_height number_of_sprites" and is used to help make the final txt for that dir. It also assumes each frame has the same width and height and all the frames are tightly packed in an 2D representation of a 1D array-like grid
	+ A dir with the crayon_spritesheet field may only contain n png's and n or less crayon_anim's (Each crayon_anim has a png of the same name, but if a png doesn't have a corresponding crayon_anim then its assumed the png is a single frame/just a sprite). Any other files and directories will be ignored
	+ files that aren't involved in preprocessing (Except gziped ones) are hardlinked
	+ crayon_img means "Make this into a romdisk"
	+ crayon_gz means "GZ compress this file/romdisk", can be combined with crayon_img
	+ **TODO** Make crayon_ignore so the bash script doesn't explore those directories
	+ **TODO** Make crayon_PAL4BPP and crayon_8BPP to generate only the palette file everything in the dir (Think more how exactly this will work to make it intuitive so it can be used for easily swappable palettes)

### Code Structure (OUTDATED STUFF HERE)

First some definitions:

+ A crayon_spritesheet (SS): Contains 1 or more crayon_animations
+ A crayon_animation: Contains 1 or more sprites/frames
+ A sprite is the individual sub-textures/UV-coords-of-texture that you see when playing the game

Ofc we have the basics, romdisk un/mount, setup_palette, all the draw functions for each format, load_texture(DTEX \*, DPAL \*) (Nothing is passed in for the DPAL pointer if its not paletted). Each texture it can load must be in the .dtex format (And if paletted it comes with a dtex.pal file) to work with the load texture functions.

Here's my structs for storing the spritesheet info:

```c
typedef struct crayon_palette{
	uint32_t *palette;		//Pointer to heap allocated palette (Its treated like an array of size "colour_count")
	uint16_t colour_count;	//Number of colours in the palette
} crayon_palette_t;

typedef struct crayon_animation{
	char *animation_name;
	uint16_t animation_x;	//Since the animations are designed to be in tiles
	uint16_t animation_y;	//We can select an frame based off of the first frame's coords
	uint16_t animation_sheet_width;	//Width of the animation sheet
	uint16_t animation_sheet_height;
	uint16_t animation_frame_width;	//Width of each frame
	uint16_t animation_frame_height;
	uint8_t animation_frames;	//How many sprites make up the animation (Dunno if this should be a short or int yet)
	//With these widths and heights, it might be possible to deduce some of them from other info, but idk
} crayon_animation_t;

typedef struct crayon_spritesheet{
	pvr_ptr_t *spritesheet_texture;
	char *spritesheet_name;	//Might be useful for when it comes time to un-mount a romdisk, otherwise I don't think its needed
	uint16_t spritesheet_dims;	//Since a pvr texture must be a square, we don't need height/width
	uint8_t spritesheet_format;	//1 for 4BPP, 2 for 8BPP, 3 for RGB565 and 4 for ARGB4444 (0 for unknown)
	crayon_palette_t *palette_data;	//Right now we still malloc this regardless if we use it or not. Change that

	crayon_animation_t *spritesheet_animation_array;	//Allows me to make an array of animation_t pointers
	uint8_t spritesheet_animation_count;	//The number of animations per spritesheet
} crayon_spritesheet_t;
```
Later I will have a draw struct for sprites/animations
```c
//Fill in later
```

There will also be rendering linked lists like the one below.

```c
typedef struct crayon_render_object{
	ss * spritesheet;
	anim * animation;
	uint8_t currentFrame;
	int drawX;
	int drawY;
	int drawZ;
	uint8_t cameraID;
	object * next;
	uint8_t objectLogicID;	//Might need to make this a uint16_t later, idk
} crayon_render_object_t;
```

It knows whether an object is opaque or transparent based on the spritesheet's format parameter. currentFrame is used to tell it which sprite to draw from the animation, drawX/Y/Z are just the draw coordinates relative to the view. The viewID variable tells it which view we are drawing relative to. Prior to rendering we set up some view structs like so

```c
typedef struct crayon_camera{
	uint16_t top_left_x_coord;
	uint16_t top_left_y_coord;
	uint16_t height;
	uint16_t width;
} crayon_camera_t;
```

This would be useful for splitscreen, multiple layers (HUD vs main world vs Parallax)

objectLogicID is basically the general behaviour of a texture. So a background has none, but a sprite might have weight, immunity level, etc. The id is used to refer to the "logic list" that I'll decide on its specifics later.

### Pre-requisites

+ General knowledge of C
+ KallistiOS
	+ As well as the KOS Port for "zlib"
+ TexturePacker
+ Texconv
	+ QT is required to build the Texconv executable
+ cdi4dc (Kazade's Unix build)
+ (NOT NEEDED RIGHT NOW) SCons (Makefile alternative)

### Installing Pre-requisites

Here are some setup guides for installing KOS. I recommend downloading the latest development versions instead of the latest "Release version" so you can use the FAT file system support:
+ http://gamedev.allusion.net/softprj/kos/setup-linux.php (Mac setup is very similar to this)
+ http://gamedev.allusion.net/softprj/kos/setup-wsl.php

For cdi4dc:

+ `git clone https://github.com/Kazade/img4dc.git` OR `git@github.com:Kazade/img4dc.git`
+ `cd img4dc`
+ `cmake .`
+ `make`


For texconv:

+ `sudo apt-get install qt5-default qtbase5-dev`
+ `git clone https://github.com/tvspelsfreak/texconv` OR `git clone git@github.com:tvspelsfreak/texconv.git`
+ `cd texconv`
+ `qmake`
+ `make`

Copy the executables for texconv and cdi4dc into a directory whose path is listed in the `~/.profile` file (Or add you own directory path)


NOTE: SCONS ISN'T REQUIRED RIGHT NOW AND MAY BE DUMPED IN THE FUTURE. DON'T BOTHER INSTALLING FOR NOW
Crayon also uses SConstruct as an alternative to makefiles due to its enhanced ability. If you don't already have it installed, run this command

`sudo apt-get install scons`


**CHECK** Do we also require the `convert` command? Its used for changing the png's size to be power of 2 by power of 2

For TexturePacker:
+ Go onto their website https://www.codeandweb.com/texturepacker/download
+ Download the free installer file (For WSL/Dreamcast choose the Linux .deb file)
+ Install it
+ Run it once, read the legal stuff (Or not) and type "agree"
+ You can now use the program

If I remembered that correctly, now you should have an up and running Crayon framework.

### Random notes

Force vid_set_mode(DM_640x480_VGA, PM_RGB565); for VGA mode...I think RGB888 might not be compatible with the pvr system (Only RGB565)

Sprite mode might only be able to reliably do a maximum dimension of 256 by 256 textures. Any larger could cause "Jitter" effects due to 16-bit UV inaccuracy

### Useful for identifying colour count

`identify -format %k "filename" && echo` (includes alphas as seperate colours)

`sudo apt install imagemagick`


### Final notes

KallistiOS (KOS) is a free, BSD license-based development system for the Sega Dreamcast game console. Legal definitions really aren't my strength, they fly high above my head, so I'm probably forgetting to say something that the BSD license requires me to say. Just pretend that missing stuff is here or google what the BSD license is for yourself I guess. Also obligatory *If you use this and break something its not my fault, don't sue me ok?* message, I think thats a part of the BSD.

In Crayon, some of the code to load in the GZ compressed romdisk images came from dcemulation.org user "BlackAura" and a few other users forum posts.
