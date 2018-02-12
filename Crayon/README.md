# Crayon, framework for a new KOS project

Note: texconv doesn't work with 2048 by 2048 textures :(

(But at least it works with 1024 by 1024 textures)

### Priority:

+ ! = Can wait till after final Dino runner build
+ !! = Can wait till after initial Dino runner demo
+ !!! = Most important, needed for initial Dino runner demo

### Requirements:

+ Romdisk Swapping support (!!!)
+ Spritesheet support (!!!)
+ Rendering lists of ease of use (!!!)
+ 4bpp loading/rendering support (!!!)
+ Maple/Controller support (!!!)
+ Palette entry modification support (!!!)
+ Error display support (!!!)
+ Video mode support (!!!)
+ Selector to easily choose between "sd" and "cd" dirs (cd for CD-Rs, sd for the sdloader method) (!!)
+ Threading support (!!)
+ VGA/Progressive Scan support (!!)
+ Non paletted ARGB4444 loading/rendering support (!!)
+ VMU save/load support (!!)
+ VMU draw to screen support (!!)
+ Sound support (!!)
+ 8bpp loading/rendering support (!)
+ Non paletted RGB565 loading/rendering support (!)
+ RAM/VRAM (And SRAM?) space debug (!)

### Developer's Directory Design

root:

	+ assets/
	+ cdfs/
	+ crayon/	//Contains crayon library stuff
	+ main.c
	+ makefile
	+ make.sh

assets can contain a normal file system, but files/folders with crayon_x fields will be pre-processed. Basically the stuff inside there dirs will get processed and the processed result will be the cdfs directory.

(Explain later what each field means)

The picture formats are PAL4BPP, PAL8BPP, RGB565 (Opqaue) or ARGB4444 (Transparent).

Note: Its up to the artist/developers to make sure that the art assets meet the BPP requirements (OPAQUE and TRANSPARENT are 16BPP) and that the romdisk/textures/sound files will all fit in the Dreamcast's RAM nicely (ie. Not crash)

The crayon/ directory contains all the .c and .h crayon files used by the program

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
	+ The crayon_anim found in crayon_packer_sheet dirs contains "height width number_of_sprites" and is used to help make the final txt for that dir
	+ A dir with the crayon_packer_sheet field may only contain n png's and n or less crayon_anim's (Each crayon_anim has a png of the same name, but if a png doesn't have a corresponding crayon_anim then its assumed the png is a single frame/just a sprite). Any other files and directories will be ignored
	+ files that aren't involved in preprocessing (Except gziped ones) are hardlinked
	+ crayon_img means "Make this into a romdisk"
	+ crayon_gz means "GZ compress this file/romdisk", can be combined with crayon_img

### Code Structure

First some definitions:

+ A spritesheet (SS): Contains 1 to many animations
+ An animation: Contains 1 to many sprites/frames
+ A sprite is the individual textures that you see when playing the game

Ofc we have the basics, romdisk un/mount, setup_palette, all the draw/render functions for each format, load_texture(DTEX \*, DPAL \*) (Nothing is passed in for the DPAL pointer if its not paletted). Each texture it can load must be in the .dtex or .dtex.pal formats to work with the load texture functions.

Each .dtex is actually a SS that contains 1 to many animations. So here's my proposed structs for storing the SS info

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

### Pre-requisites

+ General knowledge of C
+ KallistiOS
+ TexturePacker
+ Texconv
	+ QT is required to build the Texconv executable
+ cdi4dc (Unix build)

### Random note

Force vid_set_mode(DM_640x480_VGA, PM_RGB565); for VGA mode...I think RGB888 might not be compatible with the pvr system (Only RGB565)

Sprite mode might only be able to reliably do a maximum dimension of 256 by 256 textures. Any larger could cause "Jitter" effects