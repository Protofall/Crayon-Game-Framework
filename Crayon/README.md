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

### Developer's Directory Design (Old-ish)

I'm thinking we have 3 directories:

+ assets
+ pack
+ cdfs

assets contains the original game assets in their original formats. pack will have pre-processed versions of the assets in their respective romdisks (So \*.png vecomes \*.dtex and \*.dtex.pal) and cdfs will contain \*.img's or \*.img.gz's of each romdisk dir from the pack dir aswell as 1st_read.bin and is used to make the .cdi program

More on assets, I want to set up the path to be something the developer/s can easily navigate, but still allow for automatic contruction. So heres my planned layout:

assets -> rom0/rom1/.../romN-1 (Total of N romdisks)
romX -> sprites/other
sprites -> name.format

Incase thats not clear, assets contains those N directories (Not just rom0 that contains rom1 etc.) Same logic goes for each romdisk's contents

Note: Its up to the artist/developers to make sure that the art assets meet the BPP requirements (OPAQUE and TRANSPARENT are 16BPP) and that the romdisk/textures/sound files will all fit in the Dreamcast's RAM nicely (ie. Not crash)

For the directories in romX, I think the best approach is to store all PNG assets in the "sprites" dir where each of the sprite sheet assets are in the folder "name.format" where name is going to be the name of the resultant spritesheet and format is PAL4BPP, PAL8BPP, RGB565 (Opqaue) or ARGB4444 (Transparent). This will allow for the developer to easily have 1 to many spritesheets of each format in just 1 romdisk while still making it easy to navigate. The makefile will be able to look at each folder name and determine its format and will output the "name.format.png" and "name.format.txt" files into the "other" directory. So if you want to make a 4BPP spritesheet you'd dump your sprites into assets/romX/sprites/name.PAL4BPP/ and the makefile will compile a SS out of that. (Again, artist/devs need to make sure when making a paletted spritesheet that the png doesn't go over 16/256 colours)

The "other" directory will contain the SS PNG and TXT files (If any were made) aswell as any other files you want in your romdisk. That could be other text files used for dialogue, music files and anything else I'm forgetting. I don't think we can store more PNGs here otherwise texconv wouldn't know what type you want to convert it to, so just make a new spritesheet with just 1 texture in it.

The pack directory will contain all the contents of the "other" directory/s except the PNGs will be converted to DTEXs (and DTEX.PALs if it was called PAL4BPP.png or PAL8BPP.png)

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
+ Something for SConstruct? (Not sure if I'll use that yet)

### Random notes

Force vid_set_mode(DM_640x480_VGA, PM_RGB565); for VGA mode...I think RGB888 might not be compatible with the pvr system (Only RGB565)

Jamo/Proto convo:

Having anims in one png in effect guarantees the layout of the frames in the texture, and gives you the location of the first frame and size of all the frames put together
you can still make it easy for the developer

The animation frames will be in a grid, so it's easy to find the coords of a frame given the coords of the grid, the size of the grid and the size of the frames and when someone is working on an animated sprite, they are most likely to be working on all the frames at once in a grid in their image editor, so it's easier for the artist (they don't have to split it up into frames every time they edit it)

the only information that needs to be provided is the width and height of a frame within an animation all the frames will have the same size anything else is overcomplicated and an artist wouldn't bother with it

I guess so. So txt file with width and height and in the makefile it combines all of those dim files into one big txt file and thats used along with texturepacker's txt file to help build the struct info

I still feel frame number will be needed for when the anim sheet is inserted into the sprite sheet...

SConstruct might be better than makefile. It uses python

### New dir system

root:

	+ assets/
	+ pack/
	+ cdfs/
	+ crayon/	//Contains crayon library stuff
	+ main.c
	+ makefile/SConstruct_file

assets will contain 2 different kinds of folders (Maybe more later like a direct cd dir) name.img, name.img.gz. Basically the stuff inside there dirs will get processed and eventually become romdisks that are etither GZ compressed or not. Inside the romdisk dirs we will have "name" and "name.format.crayon_sheet" dirs. The first won't have any kind of processing done to it whereas the 2nd one will grab all PNG files in it and convert that into a spritesheet. It will also output a TXT file specifying where each PNG is in the spritesheet. Some work would need to be done to integrate the name.crayon_anim text files into the spritesheet TXT file to have the full data.

The resultant spritesheet would be converted into a dtex file (+ dtex.pal depending on which format was chosen) The dtex/s and TXT file go into a romdisk dir.

Right now the idea is that the pack dir looks just like the assets dir except name.format.crayon_sheet and name.crayon_anim has been processed and is now just a sprite sheet/anim sheet called "name.dtex" (+ name.dtex.pal for paletted). Might do a symlink solution later to remove asset duplication.

Each romdisk will go into the cdfs dir (make clean just deletes all .img and .img.gz, it won't delete other stuff if you manually put stuff there)

crayon/ contains all the .c and .h crayon files used by the program