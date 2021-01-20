# Crayon, framework for a new KOS project

Hello! This is my main Dreamcast Homebrew project. When I started Dreamcast homebrew a few years ago, there wasn't really any existing frameworks/engines that I could use or learn from. Theres a few old abandoned ones in KOS ports, an older software-rendered OpenGL port and some other enginies that weren't publically available, so I started making my own thing. The goal of Crayon was to learn more about using the Dreamcast hardware and share it with the community so they could learn from it. Its not designed to be the best or well experience, that's why its called Crayon.

"Before you learn how to use a pencil, you use Crayons" - JamoHTP

Crayon is a 2D game framework, it uses a combination of regular polygons and the Dreamcast's "sprite" polygons. You can set a layer for the sprites though, so as long as overlapping sprites have different layers then you don't need to worry about Z-fighting.

Of course now we have Simulant, Kazade's GLdc, MrNeo240's Integrity Engine and nuQuake so we aren't short of alternatives anymore. Even so, maybe something here is of use to you. If you have any questions, you can find me on the Simulant discord or leave an issue on the github if you find a bug.

### How to setup and Pre-requisites

NOTE: So far this has only been tested on Native Ubuntu, MacOSX and WSL, but other platforms may work for you

Before starting to use Crayon, you must have these pre-requisites met:
+ General knowledge of C
+ KallistiOS
	+ As well as the KOS Port for "zlib"
+ Basic knowledge of Makefile (Later this will be SConstruct)
+ SoX for changing audio file sample rate

### Installing Pre-requisites

Here are some setup guides for installing KOS. I recommend downloading the latest development versions instead of the latest "Release version" so you can use the FAT file system support:
+ http://gamedev.allusion.net/softprj/kos/setup-linux.php (Mac setup is very similar to this)
+ http://gamedev.allusion.net/softprj/kos/setup-wsl.php

Once that's done, go into the `Crayon` folder and run `./init.sh` which installs these pre-requisites for us:
+ ALdc2, an OpenAL implementation on Dreamcast
+ cdi4dc (Kazade's fork)
+ Texconv
	+ QT is required to build the Texconv executable
+ TexturePacker
+ Crayon-Utilities (VMU bitmap tool, Dreamcast Savefile icon generator and Dreamcast Eyecatcher generator)

NOTE: You will need to change the bash script slightly if your terminal doesn't source from `~/.profile` or use the `apt-get` package manager.

SoX is used for making sure WAV files' sample rates are no greater than 44.1khz and if so it will down sample it. This is because ALdc only allows up to 44.1khz since thats the best the Dreamcast can do. SoX can be installed via your package manager, for example on Debain you can run.

`sudo apt-get install sox`

Texturepacker should be installed with the `init.sh` script, but if it isn't then follow these steps
+ Go onto their website https://www.codeandweb.com/texturepacker/download
+ Download the free installer file (For WSL/Dreamcast choose the Linux .deb file)
+ Install it
+ Run it once, read the legal stuff (Or not) and type "agree"
+ You can now use the program

### Future Pre-requisites

Crayon will uses SConstruct as an alternative to makefiles due to its enhanced ability. If you don't already have it installed, run this command

`pip3 install scons`

Don't use `apt` or other package managers as those use outdated versions.


### Features list
+ Sprite Arrays for easily rendering one or many of the same kind of sprites
+ Paletted textures and ways to modify the palettes at runtime
+ An assets pre-processing script that analyses the tags on files/folders and processes them
	+ PNGs can be converted to DTEX files (Native Dreamcast format) and support the main formats for 2D
	+ Audio files can be converted so the master can be 48khz whereas on the disc its 44.1khz
	+ Ability to make a romdisk with a folder
+ Uses KOS's Virtual File System and supports multiple romdisks being loaded and unloaded
+ Viewports/camera that can be moved around the world
+ A combination of software and hardware viewport cropping
+ Crayon Savefile for managing savefile with version control
+ Can render bitmaps to VMU
+ Uses Kazade's ALdc port for audio (\*Not actually integrated into Crayon yet)


### Starting a new project
Firstly I'd recommend looking at the demos provided to see what Crayon is capable of.

Once you feel satisfied with understanding what Crayon is capable of, copy the `New-Project` demo. You can rename the project at the top of the makefile.

To find a full list of the functions/variables exposed by libCrayon navigate to `[CRAYON_ROOT]/Crayon/include/crayon/` where you will find multiple `.h` files. There are mutliple comments explaining what the functions do, hopefully they make sense to someone who isn't me, but if you have trouble understanding it, make a Github issue and I'll address it.

### Random tips and tricks

For identifying colour count, install image magick

`sudo apt install imagemagick`

And run this command to see how many different colours are in the image

`identify -format %k "FILENAME" && echo`

NOTE: This does include alphas as seperate colours. This is useful if you are working with palettes and are curious how much wiggle room you may have.

### Final notes

KallistiOS (KOS) is a free, BSD license-based development system for the Sega Dreamcast game console. Legal definitions really aren't my strength, they fly high above my head, so I'm probably forgetting to say something that the BSD license requires me to say. Just pretend that missing stuff is here or google what the BSD license is for yourself I guess. Also obligatory *If you use this and break something its not my fault, don't sue me ok?* message, I think thats a part of the BSD.

In Crayon, some of the code to load in the GZ compressed romdisk images came from dcemulation.org user "BlackAura". Crayon is a major evolution of the [PVR Spritesheet demo](https://dreamcast.wiki/PVR_Spritesheets), it has greatly expanded from that demo so much so I would say they aren't the same thing.





# BELOW IS OLD STUFF

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

