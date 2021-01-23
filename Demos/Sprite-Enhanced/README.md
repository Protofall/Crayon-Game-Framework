# Sprite Enhanced

This demo shows off drawing with `crayon_graphics_draw()` in `CRAYON_DRAW_ENHANCED` mode.

Significant features:
	- Can use the colour and fade parameters in "ADD" or "BLEND" mode to shade texture
	- Full 360 degree rotation support

Dreamcast factors:
	- UVs are stored in 32-bit and hence we don't lose any accuracy (Since DC textures only go up to 2048 wide/high)
	- Uses polygons to draw stuff instead of the Dreamcast's sprite mode. Therefore this version is usually slower than the simple version
