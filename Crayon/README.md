# Crayon, framework for a new KOS project

Note: texconv doesn't work with 2048 by 2048 textures :(

(But at least it works with 1024 by 1024 textures)

Reminder: Don't forget to ask about video mode's 2nd param

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

For the directories in romX, I think the best approach is to store all PNG assets in the "sprites" dir where each of the sprite sheet assets are in the folder "name.format" where name is going to be the name of the resultant spritesheet and format is 4BPP, 8BPP, RGB565 (Opqaue) or ARGB4444 (Transparent). This will allow for the developer to easily have 1 to many spritesheets of each format in just 1 romdisk while still making it easy to navigate. The makefile will be able to look at each folder name and determine its format and will output the "name.format.png" and "name.format.txt" files into the "other" directory. So if you want to make a 4BPP spritesheet you'd dump your sprites into assets/romX/sprites/name.4BPP/ and the makefile will compile a SS out of that. (Again, artist/devs need to make sure when making a paletted spritesheet that the png doesn't go over 16/256 colours)

The "other" directory will contain the SS PNG and TXT files (If any were made) aswell as any other files you want in your romdisk. That could be other text files used for dialogue, music files and anything else I'm forgetting. I don't think we can store more PNGs here otherwise texconv wouldn't know what type you want to convert it to, so just make a new spritesheet with just 1 texture in it.

The pack directory will contain all the contents of the "other" directory/s except the PNGs will be converted to DTEXs (and DTEX.PALs if it was called 4BPP.png or 8BPP.png)

### Code Structure

First some definitions:

+ A spritesheet (SS): Contains 1 to many animations
+ An animation: Contains 1 to many sprites/frames
+ A sprite is the individual textures that you see when playing the game

Ofc we have the basics, romdisk un/mount, setup_palette, all the draw/render functions for each format, load_texture(DTEX \*, DPAL \*) (Nothing is passed in for the DPAL pointer if its not paletted). Each texture it can load must be in the .dtex or .dtex.pal formats to work with the load texture functions.

Each .dtex is actually a SS that contains 1 to many animations. So here's my proposed structs for storing the SS info

```c
SS{
	pvr_ptr_t \* ss_texture;
	Anim \* ss_anims;	//Assigned with dynamic array
	int ss_dims;	//Since a pvr texture must be a square, we don't need height/width
	short ss_format; //0 for 4BPP, 1 for 8BPP, 2 for RGB565 and 3 for ARGB4444 (-1 for unknown)
	palette \* ss_palette_id; //If it uses a palette, here is where you assign it's palette
}

Anim{
	char \* anim_name;
	short anim_frames;	//How many sprites make up the animation (Dunno if this should be a short or int yet)
	int \* anim_top_left_x_coord;	//Assigned with dynamic array
	int \* anim_top_left_y_coord;
	int \* anim_width;
	int \* anim_height;
}
```

I'm not 100% sure about the data types for the Anim struct (I don't think pairs exist in regular C). However this gets the job done. The SS structs will be stored in a linked list. There will also be rendering linked lists like the two below.

```c
renderOpaque{
	SS \* spritesheet;
	Anim \* animation;
	int currentFrame;
	int drawX;
	int drawY;
	int drawZ;
	short camID;
	renderOpaque \* next;
}

renderTransparent{
	SS \* spritesheet;
	Anim \* animation;
	int currentFrame;
	int drawX;
	int drawY;
	int drawZ;
	short camID;
	renderTransparent \* next;
}
```

Even though they appear the same and could easily be reduced into one list, I feel this is better since the Dreamcast renders all opaque and transparent stuff separate from one another (I'm ignoring punchthru since I don't plan to support it). So first it draws all the opaque textures then all the transparent textures. currentFrame is used to tell it which sprite to draw from the animation, drawX/Y/Z are just the draw coordinates relative to the camera. The camID variable tells it which camera we are drawing relative to. Prior to rendering we set up some camera structs like so

```c
camera{
	int top_left_x_coord;
	int top_left_y_coord;
	int height;
	int width;
}
```

This would be useful for splitscreen (And maybe views too?). I feel this camera idea in its current state is very primative so I hope to see it develop over time.

### Pre-requisites

+ General knowledge of C

+ KallistiOS

+ TexturePacker

+ Texconv
	+ QT is required to build the Texconv executable

+ cdi4dc (Unix build)

### Alternative Directory Structure

Since each SS contains 1 to many animations and an animation could be 1 to many frames/sprites, it would be nice for the code to easily know which sprites belong to a certain animation rather than us having to manually make a seperate txt file that groups them. So maybe within each format directory you have another directory per animation. So for example we have romX/8bpp/tulip and romX/8bpp/rose each of them contain the sprites for those two animations. When texturepacker comes it puts them all into one big SS with the sprite coordinates, but something knows that certain sprites came from "tulip" and others from "rose" so when its loaded in it makes the SS and gives it two animation pointers, tulip and rose. Under each animation it knows the coords and dims of each sprite for that animation and hence its all done.