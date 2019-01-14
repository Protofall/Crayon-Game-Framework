# General about

This repository is basically a bunch of my projects I've worked on for KallistiOS (KOS). KallistiOS is a free, BSD license-based development system for the Sega Dreamcast game console. Legal definitions really aren't my strength, they fly high above my head, so I'm probably forgetting to say something that the BSD license requires me to say. Just pretend that missing stuff is here or google what the BSD license is for yourself I guess. Also obligatory *If you use this and break something its not my fault, don't sue me ok?* message, I think thats a part of the BSD.

Here is a list of all the projects and how much of them came from other sources or were original:

### curiousGZTest
The code to load in the GZ compressed romdisk images came from dcemulation.org user "BlackAura"

### fadeTest
The fade effect is unique. Might not be the best way to do it, but the only way I know how (Without palettes)

### kmgTest
Contains code from this guide: http://dcemulation.org/?title=KMG_Textures

### myC++PNG
Its just the PNG example included with the kos-ports converted to C++ with a few minor changes

### paletteTest
It heavily borrows from bogglez's Spritesheet tutorial using texconv to make the paletted texture files, code to load them in and drawing sprites instead of polygons. I removed all the Spritesheet stuff to simplify this example, added swappable romdisks and included a function that can display messages and stops the program (Useful for error debugging). Bogglez's tutorial on spritesheets/paletted textures can be found here: http://dcemulation.org/?title=PVR_Spritesheets

### picSizeTest
The dino textures are from Google Chrome's Dino Runner Game

### ROMDISK_SWAPPING
The code to load in the GZ compressed and non-compressed romdisk images came from BlackAura. The fork of img4dc was made by Kazade. The makefiles for the "JamoHTP-Protofall" versions were made by JamoHTP

### VMUSaveTest
Contains modified code from this guide: http://dcemulation.org/?title=Filesystem
and also BlueCrab's CrabEmu
