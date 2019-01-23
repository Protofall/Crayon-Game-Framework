# This is a modified version of the bogglez's Spritesheet tutorial using texconv

However it has been simplified to use sprites instead of spritesheets and more so show off the palette modification effect (Modifying palette entries as oppose to swapping palettes). When compiling it uses texconv to convert the .png assets into .dtex and .dtex.pal files which are 4 or 8 bit per pixel equivalents of our original source .png's. Heres a general overview of the advantages and disadvantages of paletted textures:

+ Programs like the PNG example use 16bpp textures, 8bpp takes up half as much vram and 4bpp takes up a quarter as much saving much needed space

+ Each pixel doesn't contain a colour value, instead an id. This id is compared against the palette (like an array) and returns a colour. In other words we can use 4bpp or 8bpp to represent colours in any spectrum including ARGB8888 mode (Without paletted textures you'd need a 32bpp texture)

+ We can directly modify these palette entries to change a colour used in a texture. Or we could do a "palette swap" and give the texture a new set of colours

+ However we only have 1024 palette entries, 4bpp is limitted to 16 colours per palette (2^4 = 16, and has 64 available palettes) and 8bpp is limitted to 256 colours per palette (2^8 = 256, and has 4 avaliable palettes)

By default we use 4bpp mode (But you can remove a flag in paletteTest.c and change the texconv command in the makefile to use 8bpp mode instead). This program starts by loading a romdisk (See romdisk swapping examples) and reading the .dtex and .dtex.pal files to contruct our sprite information. We store this in the sprite, dtex_header and dpal_header structs and load our 4bpp textures into vram (Pointed to by global pointers).

Every frame we must setup each palette and then we use our draw_sprite() function to draw our textures with the right palettes. After every pvr scene we get the current frame count and read the last 8 bits. We use that to incrementally modify Fade's background colour (palette[0]) which will give it the effect of changing from green to blue and every 256 frames we switch Insta's background colour between the original green and solid blue.

This program also contains a debug tool, error_freeze(), that sorta replaces printf(). You pass a message and a variable into it and then it shuts down the pvr system and uses the BIOS font to render you message on the screen and indefinately hangs. Quite useful for users of .cdi's. Its limited compared to printf(), but later I'll make a new tool for live debugging display.

### Dependencies

+ KallistiOS
+ cdi4dc (Unix build) (Only needed to automatically make a cdi)
+ Texconv
	+ QT (Gives qmake which is only needed to build the Texconv executable)