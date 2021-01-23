# Sprite Simple

This demo shows off drawing with `crayon_graphics_draw()` in `CRAYON_DRAW_SIMPLE` mode.

Significant features:
	- The colour and fade parameters are ignored
	- Rotations lock to nearest 90 degree increment

Dreamcast factors:
	- If using spritesheets larger than 256 by 256 and try setting UVs to outside the top left 256 squared section, the UVs might be inaccurate and yield undesired results
	- Uses "Sprite mode" to draw stuff instead of normal polys. Therefore this version is usually faster than the enhanced version
