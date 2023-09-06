# IMPORTANT

This repo is no longer active, instead I have moved to gitlab. Therefore I will no longer be updating this repository.

To get the latest changes, go to: https://gitlab.com/Protofall/Crayon-Game-Framework

-----------------------------------------

# Crayon, framework for a new KOS project

Hello! This is my main Dreamcast Homebrew project. When I started Dreamcast homebrew a few years ago, there wasn't really any existing frameworks/engines that I could use or learn from. There's a few old abandoned ones in KOS ports, an older software-rendered OpenGL port and some other engines that weren't publicly available, so I started making my own thing. The goal of Crayon was to learn more about using the Dreamcast hardware and share it with the community so they could learn from it. It's not designed to be the best or well experience, that's why it is called Crayon.

`"Before you learn how to use a pencil, you use Crayons" - JamoHTP`

Crayon is a 2D game framework. It uses a combination of regular polygons and the Dreamcast's "sprite" polygons. You can set a layer for the sprites though, so as long as overlapping sprites have different layers then you don't need to worry about Z-fighting.

Of course now we have Simulant, Kazade's GLdc, MrNeo240's Integrity Engine and nuQuake so we aren't short of alternatives anymore. Even so, maybe something here is of use to you. If you have any questions, you can find me on the Simulant discord or leave an issue on the GitHub if you find a bug.

### How to setup and Prerequisites

**NOTE:** So far this has only been tested on Native Ubuntu, MacOSX and WSL, but other platforms may work for you

Before starting to use Crayon, you must have these prerequisites met:
+ General knowledge of C
+ KallistiOS
	+ As well as the KOS Port for "zlib"
+ Basic knowledge of Makefile (Later this will be SConstruct)
+ SoX for changing audio file sample rate

### Installing Prerequisites

Here are some setup guides for installing KOS. I recommend downloading the latest development versions instead of the latest "Release version" so you can use the FAT file system support:
+ http://gamedev.allusion.net/softprj/kos/setup-linux.php (Mac setup is very similar to this)
+ http://gamedev.allusion.net/softprj/kos/setup-wsl.php

Once that's done, go into the `Crayon` folder and run `./init.sh` which installs these prerequisites for us:
+ ALdc2, an OpenAL implementation on Dreamcast
+ cdi4dc (Kazade's fork)
+ Texconv
	+ Qt is required to build the Texconv executable
+ TexturePacker
+ Crayon-Utilities (VMU bitmap tool, Dreamcast Savefile icon generator and Dreamcast Eyecatcher generator)

**NOTE:** You will need to change the bash script slightly if your terminal doesn't source from `~/.profile` or use the `apt-get` package manager.

SoX is used for making sure WAV files' sample rates are no greater than 44.1kHz and if so it will down sample it. This is because ALdc only allows up to 44.1kHz since that's the best the Dreamcast can do. SoX can be installed via your package manager, for example on Debian you can run.

`sudo apt-get install sox`

Texturepacker should be installed with the `init.sh` script, but if it isn't then follow these steps
+ Go onto their website https://www.codeandweb.com/texturepacker/download
+ Download the free installer file (For WSL/Dreamcast choose the Linux .deb file)
+ Install it
+ Run it once, read the legal stuff (Or not) and type "agree"
+ You can now use the program

### Future Prerequisites

Crayon will use SConstruct as an alternative to Makefiles due to its enhanced ability. If you don't already have it installed, run this command

`pip3 install scons`

Don't use `apt` or other package managers as those use outdated versions.


### Features list
+ Sprite Arrays for easily rendering one or many of the same kind of sprites
+ Paletted textures and ways to modify the palettes at runtime
+ An assets pre-processing script that analyses the tags on files/folders and processes them
	+ PNGs can be converted to DTEX files (Native Dreamcast format) and support the main formats for 2D
	+ Audio files can be converted so the master can be 48kHz whereas on the disc its 44.1kHz
	+ Ability to make a romdisk with a folder
+ Uses KOS's Virtual File System and supports multiple romdisks being loaded and unloaded
+ Viewports/camera that can be moved around the world
+ A combination of software and hardware viewport cropping
+ Crayon Savefile for managing save files with version control
+ Can render bitmaps to VMU
+ Uses Kazade's ALdc port for audio (\*Not actually integrated into Crayon yet)


### Starting a new project
Firstly I'd recommend looking at the demos provided to see what Crayon is capable of. Do note that some of them aren't using the latest and best Crayon methods (e.g., you should always start your Crayon program with `crayon_init()`, however some programs have old code that skips that).

Once you feel satisfied with understanding what Crayon is capable of, copy the `New-Project` demo. You can rename the project at the top of the Makefile.

To find a full list of the functions/variables exposed by libCrayon navigate to `[CRAYON_ROOT]/Crayon/include/crayon/` where you will find multiple `.h` files. There are multiple comments explaining what the functions do, hopefully they make sense to someone who isn't me, but if you have trouble understanding it, make a GitHub issue and I'll address it.

### How does the asset pre-processing script work?

This will be completely overhauled in v0.3, but at the moment this is how it works. The script will scan through the `assets/` folder and output the processed output into the `cdfs/` folder. Folders/files in the assets directory can have "tags" to them. For example a directory called `soldiers.PAL4BPP.crayon_spritesheet` has 2 tags, `PAL4BPP` and `crayon_spritesheet` and in this case the name is `soldiers` so the final output file will be `soldiers.png`. Here is a list of all tags and what they do.

+ `crayon_spritesheet`: Only works on directories. Will take all PNG files inside the folder and turn them into a spritesheet. It makes sure the spritesheet's width and height are both powers of two so the Dreamcast can read them
+ `PAL4BPP`, `PAL8BPP`, `ARGB1555`, `ARGB4444`, `RGB565`, `YUV422`: Works directories that have the `crayon_spritesheet` tag, as well as any PNG file. These tags will take the PNG and make a "DTEX" output file which is the native Dreamcast output format. The specific DTEX format will be the one that matches the tag you gave. E.g. `RGB565` will output a DTEX file where every 16-bits is a pixel with 5-bit red and blue channels and 6-bit green channel. Note that "A" stands for alpha so `ARGB1555` is just punchthrough (Similar to all GBA sprites) and `YUV422` is its own weird format.
+ `crayon_img`: Only works on directories. Once everything inside this directory has been processed, it will then turn it into a `.img` image. Used to load directories as "romdisks" on the Dreamcast
+ `crayon_gz`: Works on any file/folder. Will gz-compress it. It seems to be buggy on Dreamcast though, so don't use it.
+ `crayon_anim`: Text files only. They just describe how a texture can be an animation sheet. The file states how many frames there are and each frame's width/height (Every frame has the same width/height)
+ `crayon_temp`: Internal only, it is a tag given to intermediate files. You can see this if you run the asset pre-processing script with `-noRM` and is useful to see if assets are being built right.

Although not a tag, any WAV file it sees will be processed. If it's > 44.1kHz `sox` will lower the sample rate. It will also make all sound files mono.

Any file within a tag and isn't inside a `crayon_img`, `crayon_gz` or `crayon_spritesheet` will instead be symlinked into the right part of the `cdfs/` directory to help reduce space usage on your computer.

### Processed assets visualisation

`assets/`

```
|-> plants.crayon_img/
      |-> flowers.PAL4BPP.crayon_spritesheet/
      |     |-> rose.png
      |     |-> rose.crayon_anim
      |     |-> tulip.png
      |     |-> tulip.crayon_anim
      |-> lol.txt
```

In `cdfs/` (If we could explore .img's)

```
cdfs/
|-> plants.img/
      |-> flowers.dtex
      |-> flowers.dtex.dpal
      |-> flowers.txt
      |-> lol.txt
```

In Dreamcast/KOS VFS

```
cd/[MOUNT_NAME]	# Note, this might be attached to the sd or pc dir instead
  |-> flowers.dtex
  |-> flowers.dtex.dpal
  |-> flowers.txt
  |-> lol.txt
```

### Random tips and tricks

For identifying colour count, install imagemagick

`sudo apt install imagemagick`

And run this command to see how many different colours are in the image

`identify -format %k "FILENAME" && echo`

**NOTE:** This does include alphas as separate colours. This is useful if you are working with palettes and are curious how much wiggle room you may have.

**NOTE:** It's up to the artist/developers to make sure that the art assets meet the BPP requirements and that the romdisk/textures/sound files will all fit in the Dreamcast's RAM nicely (Not crash). PALXBPP is X-bits per pixel and every other texture format is 16-bits per pixel.

### Credits

Big thanks to dcemulation.org and dc-talk forums as well as the Simulant discord group for help me understand how the Dreamcast works and techniques that can be used. Thanks to BlueCrab, MrNeo240, Kazade, Cooljerk, TVSpelsFreak, Falco Girgis, Sizious, Lerabot, MoopTheHedgehog and Ian Michael to name a few.

Thanks to JamoHTP for the Makefile system overhaul a few years ago as well as multiple sprites for various Dreamcast accessories.

Thanks Airofoil for the "dwarfy" and "James" sprites used in multiple demos.

Lastly, multiple demos use the Microsoft "Tahoma" font to demonstrate the "proportionally spaced fonts". This isn't free to use in commercial products and needs to be replaced at a later date. Same goes for the "wololo" sound effect in the `Audio-Test` demo. BIOS font is as old as the ages. I don't remember who made it or what license it has.

### Final notes

KallistiOS (KOS) is a free, BSD license-based development system for the Sega Dreamcast game console. Legal definitions really aren't my strength, they fly high above my head, so I'm probably forgetting to say something that the BSD license requires me to say. Just pretend that missing stuff is here or Google what the BSD license is for yourself I guess. Also obligatory *If you use this and break something its not my fault, don't sue me ok?* message, I think that's a part of the BSD.

In Crayon, some of the code to load in the GZ compressed romdisk images came from dcemulation.org user "BlackAura". Crayon is a major evolution of the [PVR Spritesheet demo](https://dreamcast.wiki/PVR_Spritesheets), it has greatly expanded from that demo so much so I would say they aren't the same thing.
