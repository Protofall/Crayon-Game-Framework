# This is a modified version of the "kos-ports" PNG example

Changes made:
* Converted it to C++ just cause (Needed to define zlib_getlength() under regular C though)
* Added in some comments to help understand whats going on
* Free-d temp_tex since it wasn't needed anymore
* Updated wfont.bin with a font that I can read

If you want to update/change wfont.bin then up need to add the pgm header at the top for the file (50 34 0A 38 20 34 30 39 36 0A) and change the extension to .pgm then open that in something like GIMP and you can edit it. To convert from pgm to bin do those steps in reverse. The characters are in ASCII order. I have included a png version of wfont.bin in the "extras" folder