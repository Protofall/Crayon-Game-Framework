#include "custom_polys.h"

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
	// vert.y = y - thickness;
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
	// vert.x = x;
	vert.y = y - thickness;
	pvr_prim(&vert, sizeof(vert));

	//Top right
	vert.x = x + dim_x + thickness - 1;
	// vert.y = y - thickness;
	pvr_prim(&vert, sizeof(vert));

	//Bottom left
	vert.x = x;
	vert.y = y;
	pvr_prim(&vert, sizeof(vert));

	//Bottom right
	vert.x = x + dim_x;
	// vert.y = y;
	vert.flags = PVR_CMD_VERTEX_EOL;
	pvr_prim(&vert, sizeof(vert));



	//Right side
	vert.flags = PVR_CMD_VERTEX;
	vert.argb = colour2;
	//Top left
	// vert.x = x + dim_x;
	// vert.y = y;
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
	// vert.y = y + dim_y + thickness;
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
	vert.x = x - thickness;
	vert.y = y + dim_y + thickness;
	pvr_prim(&vert, sizeof(vert));

	//Bottom right
	vert.x = x + dim_x + thickness;
	// vert.y = y + dim_y + thickness;
	vert.flags = PVR_CMD_VERTEX_EOL;
	pvr_prim(&vert, sizeof(vert));

	return;
}

void custom_poly_2000_topbar(uint16_t x, uint16_t y, uint16_t dim_x, uint16_t dim_y){
	return;
}