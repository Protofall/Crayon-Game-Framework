# Protofall's VMU save file demo

A basic demo that will create/save VMU files and also load them too. There is a compressed and an uncompressed version

The save file function code is a modified version of BlueCrab's CrabEmu savefile code. My version use a struct instead of a file for the loaded save. The font is a modified version of wfont.bin from KOS's PNG example. The code that loads and draws this is also from that PNG example, its slightly modified to skip the 265 byte header of the new font file. The header contains some information I'll copy here:

```
Fixed Fiction v1.0 (8x16 bitmap font)
Copyright (c) 2018 James Kirkwood

This work is licensed under the Creative Commons Attribution 4.0.
International License. To view a copy of the license, visit.
http://creativecommons.org/licenses/by/4.0/
```

Here is a more in-depth discussion talking about this: http://dcemulation.org/phpBB/viewtopic.php?f=34&t=103666&p=1047688&hilit=vmu+save+file#p1047688
