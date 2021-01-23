# Compression

This shows how the Dreamcast's VQ compression works

NOTE: This example won't work right in most emulators. Redream will mention a PVR error. It does work in the latest private DEMUL build, but not the 18th April 2018 public build, but it does work right on hardware. This is due to compressed paletted textures being unsupported. To stop most emulators from crashing, comment out the line that says `crayon_graphics_draw_sprites(&Logo_Draw, PVR_LIST_PT_POLY);` so it doesn't draw the compressed paletted texture.

