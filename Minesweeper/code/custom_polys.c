#include "custom_polys.h"

//Draws a boarder around the box you defined with that thickness. If thickness is > 1 then it does some cool stuff on top right/bottom left corners
void custom_poly_boarder(uint8_t thickness, uint16_t x, uint16_t y, uint8_t z, uint16_t dim_x,
	uint16_t dim_y, uint32_t colour1, uint32_t colour2){
	pvr_poly_cxt_t cxt;
	pvr_poly_hdr_t hdr;
	pvr_vertex_t vert;

	pvr_poly_cxt_col(&cxt, PVR_LIST_TR_POLY);
	pvr_poly_compile(&hdr, &cxt);
	pvr_prim(&hdr, sizeof(hdr));

	vert.argb = colour1;
	vert.oargb = 0;
	vert.flags = PVR_CMD_VERTEX;

	//Left side
	//Top left
	vert.x = x - thickness;
	vert.y = y - thickness;
	vert.z = z;
	pvr_prim(&vert, sizeof(vert));

	//Top right
	vert.x = x;
	pvr_prim(&vert, sizeof(vert));

	//Bottom left
	vert.x = x - thickness;
	vert.y = y + dim_y + thickness - 1;
	pvr_prim(&vert, sizeof(vert));

	//Bottom right
	vert.x = x;
	vert.y = y + dim_y;
	vert.flags = PVR_CMD_VERTEX_EOL;
	pvr_prim(&vert, sizeof(vert));



	//Top side
	vert.flags = PVR_CMD_VERTEX;
	//Top left
	vert.y = y - thickness;
	pvr_prim(&vert, sizeof(vert));

	//Top right
	vert.x = x + dim_x + thickness - 1;
	pvr_prim(&vert, sizeof(vert));

	//Bottom left
	vert.x = x;
	vert.y = y;
	pvr_prim(&vert, sizeof(vert));

	//Bottom right
	vert.x = x + dim_x;
	vert.flags = PVR_CMD_VERTEX_EOL;
	pvr_prim(&vert, sizeof(vert));



	//Right side
	vert.flags = PVR_CMD_VERTEX;
	vert.argb = colour2;
	//Top left
	pvr_prim(&vert, sizeof(vert));

	//Top right
	vert.x = x + dim_x + thickness;
	vert.y = y - thickness;
	pvr_prim(&vert, sizeof(vert));

	//Bottom left
	vert.x = x + dim_x;
	vert.y = y + dim_y + thickness;
	pvr_prim(&vert, sizeof(vert));

	//Bottom right
	vert.x = x + dim_x + thickness;
	vert.flags = PVR_CMD_VERTEX_EOL;
	pvr_prim(&vert, sizeof(vert));


	//Bottom side
	vert.flags = PVR_CMD_VERTEX;
	//Top left
	vert.x = x;
	vert.y = y + dim_y;
	pvr_prim(&vert, sizeof(vert));

	//Top right
	vert.x = x + dim_x + thickness;
	// vert.y = y;
	pvr_prim(&vert, sizeof(vert));

	//Bottom left
	vert.x = x - thickness + 1;
	vert.y = y + dim_y + thickness;
	pvr_prim(&vert, sizeof(vert));

	//Bottom right
	vert.x = x + dim_x + thickness;
	vert.flags = PVR_CMD_VERTEX_EOL;
	pvr_prim(&vert, sizeof(vert));

	return;
}

void custom_poly_2000_topbar(uint16_t x, uint16_t y, uint8_t z, uint16_t dim_x, uint16_t dim_y){
	pvr_poly_cxt_t cxt;
	pvr_poly_hdr_t hdr;
	pvr_vertex_t vert;

	pvr_poly_cxt_col(&cxt, PVR_LIST_TR_POLY);
	pvr_poly_compile(&hdr, &cxt);
	pvr_prim(&hdr, sizeof(hdr));

	vert.argb = (255 << 24) + 128;
	vert.oargb = 0;
	vert.flags = PVR_CMD_VERTEX;

	//Left side
	//Top left
	vert.x = x;
	vert.y = y;
	vert.z = z;
	pvr_prim(&vert, sizeof(vert));

	//Top right
	vert.argb = (255 << 24) + (166 << 16) + (202 << 8) + 240;
	vert.x = x + dim_x;
	pvr_prim(&vert, sizeof(vert));

	//Bottom left
	vert.argb = (255 << 24) + 128;
	vert.x = x;
	vert.y = y + dim_y;
	pvr_prim(&vert, sizeof(vert));

	//Bottom right
	vert.argb = (255 << 24) + (166 << 16) + (202 << 8) + 240;
	vert.x = x + dim_x;
	vert.flags = PVR_CMD_VERTEX_EOL;
	pvr_prim(&vert, sizeof(vert));
	return;
}

//This time the x, y and dim_x/dim_y are for the boarder itself...I might change that later
void custom_poly_2000_boarder(uint16_t x, uint16_t y, uint8_t z, uint16_t dim_x, int16_t dim_y){

	//The LG and B first boarder
	graphics_draw_untextured_poly(x, y, z + 1, dim_x - 1, dim_y - 1, (255 << 24) + (212 << 16) + (208 << 8) + 200);
	graphics_draw_untextured_poly(x, y, z, dim_x, dim_y, (255 << 24));

	//White + dark grey
	graphics_draw_untextured_poly(x + 1, y + 1, z + 3, dim_x - 3, dim_y - 3, (255 << 24) + (255 << 16) + (255 << 8) + 255);
	graphics_draw_untextured_poly(x + 1, y + 1, z + 2, dim_x - 2, dim_y - 2, (255 << 24) + (128 << 16) + (128 << 8) + 128);

	//Centre colour
	graphics_draw_untextured_poly(x + 2, y + 2, z + 4, dim_x - 4, dim_y - 4, (255 << 24) + (212 << 16) + (208 << 8) + 200);

	return;
}