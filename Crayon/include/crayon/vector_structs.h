#ifndef VECTOR_STRUCTS_CRAYON_H
#define VECTOR_STRUCTS_CRAYON_H

#include <stdint.h> // For the uintX_t types

typedef struct vec2_f{
	float x, y;
} vec2_f_t;

typedef struct vec3_f{
	float x, y, z;
} vec3_f_t;

typedef struct vec2_u8{
	uint8_t x, y;
} vec2_u8_t;

typedef struct vec3_u8{
	uint8_t x, y, z;
} vec3_u8_t;

typedef struct vec2_u16{
	uint16_t x, y;
} vec2_u16_t;

typedef struct vec3_u16{
	uint16_t x, y, z;
} vec3_u16_t;

typedef struct vec2_u32{
	uint32_t x, y;
} vec2_u32_t;

typedef struct vec3_u32{
	uint32_t x, y, z;
} vec3_u32_t;

typedef struct vec2_s8{
	int8_t x, y;
} vec2_s8_t;

typedef struct vec3_s8{
	int8_t x, y, z;
} vec3_s8_t;

typedef struct vec2_s16{
	int16_t x, y;
} vec2_s16_t;

typedef struct vec3_s16{
	int16_t x, y, z;
} vec3_s16_t;

typedef struct vec2_s32{
	int32_t x, y;
} vec2_s32_t;

typedef struct vec3_s32{
	int32_t x, y, z;
} vec3_s32_t;

#endif
