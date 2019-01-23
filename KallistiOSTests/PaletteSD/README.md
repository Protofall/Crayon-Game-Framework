# This is the paletteTest example, but bootable with the SD adapter

Once you have all the needed hardware compile this program and place the romdisk image (rom0.img) and the unscrambled program binary into the root of your ext2 formatted SD card. Then eject your SD card, insert it into the adapter and insert that into the dreamcast's serial port just below the power plug. Insert your booting CD-R (bootdreams can do the job, but I use a program called "sdloader". Will eventually update this to give info on how to find that cdi). Both bootdreams and sdloader have VFS navigators that will let you boot the unscrambled program binary. Once found choose to boot and the program will appear to run just like the normal version of paletteTest.

The reason you might want to boot your programs from the SD card is that it saves on CD-Rs (Only 1 CD needed), it allows you to easily change assets/romdisks and it can be outputted to (For stuff like screenshots or re-directing printf's to a text file).

The main disadvantage of using this SD method as oppose to the CD-Rs is that the serial port is slower than the disk drive. This means high speed stuff like FMVs and some sound can appear laggy. However its still a good tool for small quick programs.

If you want to test programs on real hardware without burning CD-Rs or slow speeds, then the BBA method is for you (And its faster than the disk drive too). However I don't own one since they are a few hundred dollars so I can't help you set that up.

### Dependencies

+ KallistiOS
+ cdi4dc (Unix build) (Only needed to automatically make a cdi)
+ Texconv
	+ QT (Gives qmake which is only needed to build the Texconv executable)
+ ext2 formatted SD card
+ Dreamcast serial/SD adapter (Incase you wonder, I got mine from dc-sd.com)
+ An official Sega Dreamcast Console (Model 0 or 1. NOT 2)
+ A program burnt onto a CD-R that can boot the executable on the SD card


### NOTE FOR FUTURE SELF

sdloader appears to not be able to boot ext2 format, but can do fat32 :/

Also this program hasn't been tested yet since I don't have an ext2 booter yet