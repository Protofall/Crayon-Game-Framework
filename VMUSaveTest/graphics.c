#include "graphics.h"

//There are 4 palettes for 8BPP and 64 palettes for 4BPP
extern int graphics_setup_palette(const crayon_palette_t *cp){
	if(cp->palette_id < 0 || cp->bpp < 0 || cp->bpp * cp->palette_id >= 1024){	//Invalid format/palette not properly set
		return 1;
	}
	int entries;
	if(cp->bpp == 4){
		entries = 16;
	}
	else if(cp->bpp == 8){
		entries = 256;
	}
	else{	//When OpenGL port arrives, this might change if I allow more than 64 entries
		return 2;
	}

	pvr_set_pal_format(PVR_PAL_ARGB8888);
	uint16_t i; //Can't this be a uint8_t instead? 0 to 255 and max 256 entries per palette
	//...but then again how would the loop be able to break? since it would overflow back to 0
	for(i = 0; i < cp->colour_count; ++i){
		pvr_set_pal_entry(i + entries * cp->palette_id, cp->palette[i]);
	}
	return 0;
}

extern uint8_t graphics_draw_text_mono(const struct crayon_font_mono *fm, uint8_t poly_list_mode, float draw_x,
	float draw_y, float draw_z, float scale_x, float scale_y, uint8_t paletteNumber, char * string){

	float x0 = draw_x;
	float y0 = draw_y;
	const float z = draw_z;

	//x1 and y1 depend on the letter
	float x1 = draw_x + fm->char_width * scale_x;
	float y1 = draw_y + fm->char_height * scale_y;

	float u0, v0, u1, v1;

	pvr_sprite_cxt_t context;

	uint8_t texture_format = (((1 << 3) - 1) & (fm->texture_format >> (28 - 1)));	//Gets the Pixel format
																												//https://github.com/tvspelsfreak/texconv
	int textureformat = fm->texture_format;
	if(texture_format == 5){	//4BPP
			textureformat |= ((paletteNumber) << 21);	//Update the later to use KOS' macros
	}
	if(texture_format == 6){	//8BPP
			textureformat |= ((paletteNumber) << 25);	//Update the later to use KOS' macros
	}
	pvr_sprite_cxt_txr(&context, poly_list_mode, textureformat, fm->fontsheet_dim,
		fm->fontsheet_dim, fm->fontsheet_texture, PVR_FILTER_NONE);

	pvr_sprite_hdr_t header;
	pvr_sprite_compile(&header, &context);
	pvr_prim(&header, sizeof(header));

	int i = 0;
	float prop_width = (float)fm->char_width / fm->fontsheet_dim;
	float prop_height = (float)fm->char_height / fm->fontsheet_dim;
	while(1){	//First char seems to be drawn higher than others
		if(string[i] == '\0'){
			break;
		}
		if(string[i] == '\n'){	//This should be able to do a new line (Doesn't seem to work right)
			x0 = draw_x;
			x1 = draw_x + fm->char_width * scale_x;
			y0 = y1;
			y1 += fm->char_height * scale_y;
			i++;
			continue;
		}
		uint8_t distance_from_space = string[i] - ' ';

		uint8_t row = distance_from_space / fm->num_columns;
		uint8_t column = distance_from_space - (row * fm->num_columns);

		u0 = column * prop_width;
		v0 = row * prop_height;
		u1 = u0 + prop_width;
		v1 = v0 + prop_height;

		pvr_sprite_txr_t vert = {
			.flags = PVR_CMD_VERTEX_EOL,
			.ax = x0, .ay = y0, .az = z, .auv = PVR_PACK_16BIT_UV(u0, v0),
			.bx = x1, .by = y0, .bz = z, .buv = PVR_PACK_16BIT_UV(u1, v0),
			.cx = x1, .cy = y1, .cz = z, .cuv = PVR_PACK_16BIT_UV(u1, v1),
			.dx = x0, .dy = y1
		};
		pvr_prim(&vert, sizeof(vert));

		x0 = x1;
		x1 += fm->char_width * scale_x;
		i++;
	}

	return 0;
}

