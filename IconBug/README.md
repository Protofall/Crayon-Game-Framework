First of all, I apologise for the lack of quality of this makefile. The library (Crayon) I've made for Minesweeper isn't ready to be made public, so I had to decouple this extract from Crayon and was rushed for time so its abit messy. I can't get SD builds to work right now, so I just removed the command to make them. It shouldn't be too hard to build one though.

The makefile depends on Kazade's unix port of cdi4dc from making a cdi, however using bootdreams you can build the scrambled rule and use Bootdreams on Windows to make a cdi.

Tested on lxdream and real hardware.




The issue:

LCD.bin is the icon data to be displayed on the VMU LCDs, Its a 48 by 32 1bpp icon (Where every 6 bytes is reversed, similar to little endian). You can see the png source icon in the **Ref** folder. The first row has no info so therefore we would expect the first 6 bytes to be all `0` (0 means pixel off/clear/white). The variable `debug_first_row` stores this value.

There is an if/else define section that depends on the `LOAD_CODE` macro. Currently its set to use my function `setup_vmu_icon_load()`, but if you change LOAD_CODE then it will use a section of code which should have the same effect as my setup function, however the program will change depending on which code you use. If you use the `LOAD_CODE != 1` code then everything performs as I intended. LCD.bin is loaded correctly, `debug_first_row` is equal to `0` and the icon data is later displayed on all connected VMU LCD screens.

However if you use the `LOAD_CODE == 1` code, stuff starts messing up. When the program tries to load LCD.bin it doesn't load it correctly. You can see this because the icon on all VMU LCDs looks like garbage info and `debug_first_row` is equal to 746 suggesting there is info on the first row, which as mentioned before, there is not for this particular icon.

The issue I have is that both code sections should be the same, but aren't and as far as I can tell I'm doing everything right so I don't know what could be wrong.

Note:

I hard coded the size of the file to be read at 192 bytes since the icon is always the size (`48 * 32 / 8 = 192 bytes`) and I wanted to make sure it was reading the right amount
