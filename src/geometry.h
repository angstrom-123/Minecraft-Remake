#ifndef GEOMETRY_H
#define GEOMETRY_H 

#if !defined(SOKOL_APP_INCLUDED) 
#include "sokol_app.h"
#endif

#if !defined(SOKOL_GFX_INCLUDED) 
#include "sokol_gfx.h"
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "extra_math.h"

#define PACK(a, b) ((uint32_t)(((uint32_t)(a * 0xFFFF) << 16) | ((uint32_t)(b * 0xFFFF) & 0xFFFF)))

#define TX(n) (0.0625 * n)
#define SIZ TX(1)

#define MAX_BLOCK_TYPES 8

#define CHUNK_SIZE 16 
#define CHUNK_HEIGHT 64

#define V_MAX 12288
#define I_MAX 18432

#define START_CANARY_VAL 0xDEADBEEF
#define MID_CANARY_VAL 0xFEEDBEAD
#define END_CANARY_VAL 0xCAFEBABE

typedef struct vertex { 
	uint8_t x;
	uint8_t y;
	uint8_t z;
	uint8_t n;
	uint32_t uv;
} vertex_t;

typedef struct chunk_data {
	int8_t types[CHUNK_SIZE][CHUNK_HEIGHT][CHUNK_SIZE];
} chunk_data_t;

typedef struct mesh {
	uint32_t v_rsrv;  // Size of buffer chunk reserved for vertices
	uint32_t i_rsrv;  // Size of buffer chunk reserved for indices
	uint32_t v_cnt;  // Actual length of vertex data 
	uint32_t i_cnt;  // Actual length of index data
	uint32_t __start_canary;
	vertex_t *v_buf; // Pointer to vertex data
	uint32_t __mid_canary;
	uint32_t *i_buf; // Pointer to index data
	uint32_t __end_canary;
} mesh_t;

typedef struct buf_offsets {
	size_t v_ofst;
	size_t i_ofst;
	size_t v_len;
	size_t i_len;
} buf_offsets_t;

typedef struct chunk {
	int32_t x;
	int32_t y;
	int32_t z;

	chunk_data_t *blocks;
	buf_offsets_t buf_data;
	mesh_t mesh;
	
	bool staged;
	bool visible;

	uint8_t age;
} chunk_t;

typedef struct quad_desc {
	uint8_t x;
	uint8_t y;
	uint8_t z;
	uint8_t type;
	uint8_t face;
} quad_desc_t;

typedef enum cube_face_idx {
	FACEIDX_BACK   = 0,
	FACEIDX_FRONT  = 1,
	FACEIDX_RIGHT  = 2,
	FACEIDX_LEFT   = 3,
	FACEIDX_BOTTOM = 4,
	FACEIDX_TOP    = 5
} cube_face_idx_e;

typedef enum cube_type {
	CUBETYPE_AIR   = 0,
	CUBETYPE_GRASS = 1,
	CUBETYPE_DIRT  = 2,
	CUBETYPE_STONE = 3,
	CUBETYPE_SAND  = 4,
	CUBETYPE_LOG   = 5,
	CUBETYPE_LEAF  = 6,
} cube_type_e;

typedef struct cube_uv_lookup {
	vec4 uv_rects[MAX_BLOCK_TYPES * 6];
} cube_uv_lookup_t;

static const vertex_t face_vertices[6][4] = {
	[FACEIDX_BACK] = {
		{2, 0, 0, 16, PACK(0.0, 0.0)},
		{0, 0, 0, 16, PACK(SIZ, 0.0)},
		{0, 2, 0, 16, PACK(SIZ, SIZ)},
		{2, 2, 0, 16, PACK(0.0, SIZ)}
	},
	[FACEIDX_FRONT] = {
		{0, 0, 2, 32, PACK(0.0, 0.0)},
		{2, 0, 2, 32, PACK(SIZ, 0.0)},
		{2, 2, 2, 32, PACK(SIZ, SIZ)},
		{0, 2, 2, 32, PACK(0.0, SIZ)}
	},
	[FACEIDX_LEFT] = {
		{0, 0, 0,  1, PACK(0.0, 0.0)},
		{0, 0, 2,  1, PACK(SIZ, 0.0)},
		{0, 2, 2,  1, PACK(SIZ, SIZ)},
		{0, 2, 0,  1, PACK(0.0, SIZ)}
	},
	[FACEIDX_RIGHT] = {
		{2, 0, 2,  2, PACK(0.0, 0.0)},
		{2, 0, 0,  2, PACK(SIZ, 0.0)},
		{2, 2, 0,  2, PACK(SIZ, SIZ)},
		{2, 2, 2,  2, PACK(0.0, SIZ)}
	},
	[FACEIDX_BOTTOM] = {
		{0, 0, 0,  8, PACK(0.0, 0.0)},
		{2, 0, 0,  8, PACK(SIZ, 0.0)},
		{2, 0, 2,  8, PACK(SIZ, SIZ)},
		{0, 0, 2,  8, PACK(0.0, SIZ)}
	},
	[FACEIDX_TOP] = {
		{0, 2, 2,  4, PACK(0.0, 0.0)},
		{2, 2, 2,  4, PACK(SIZ, 0.0)},
		{2, 2, 0,  4, PACK(SIZ, SIZ)},
		{0, 2, 0,  4, PACK(0.0, SIZ)}
	}
};

// static const vertex_t face_vertices[6][4] = {
// 	[FACEIDX_BACK] = {
// 		{1, 0, 0, 16, PACK(0.0, 0.0)},
// 		{0, 0, 0, 16, PACK(SIZ, 0.0)},
// 		{0, 1, 0, 16, PACK(SIZ, SIZ)},
// 		{1, 1, 0, 16, PACK(0.0, SIZ)}
// 	},
// 	[FACEIDX_FRONT] = {
// 		{0, 0, 1, 32, PACK(0.0, 0.0)},
// 		{1, 0, 1, 32, PACK(SIZ, 0.0)},
// 		{1, 1, 1, 32, PACK(SIZ, SIZ)},
// 		{0, 1, 1, 32, PACK(0.0, SIZ)}
// 	},
// 	[FACEIDX_LEFT] = {
// 		{0, 0, 0,  1, PACK(0.0, 0.0)},
// 		{0, 0, 1,  1, PACK(SIZ, 0.0)},
// 		{0, 1, 1,  1, PACK(SIZ, SIZ)},
// 		{0, 1, 0,  1, PACK(0.0, SIZ)}
// 	},
// 	[FACEIDX_RIGHT] = {
// 		{1, 0, 1,  2, PACK(0.0, 0.0)},
// 		{1, 0, 0,  2, PACK(SIZ, 0.0)},
// 		{1, 1, 0,  2, PACK(SIZ, SIZ)},
// 		{1, 1, 1,  2, PACK(0.0, SIZ)}
// 	},
// 	[FACEIDX_BOTTOM] = {
// 		{0, 0, 0,  8, PACK(0.0, 0.0)},
// 		{1, 0, 0,  8, PACK(SIZ, 0.0)},
// 		{1, 0, 1,  8, PACK(SIZ, SIZ)},
// 		{0, 0, 1,  8, PACK(0.0, SIZ)}
// 	},
// 	[FACEIDX_TOP] = {
// 		{0, 1, 1,  4, PACK(0.0, 0.0)},
// 		{1, 1, 1,  4, PACK(SIZ, 0.0)},
// 		{1, 1, 0,  4, PACK(SIZ, SIZ)},
// 		{0, 1, 0,  4, PACK(0.0, SIZ)}
// 	}
// };

static const vec2 uv_lookup[MAX_BLOCK_TYPES * 6] = {
	[CUBETYPE_AIR * 6 + FACEIDX_BACK]   = {0.0, 0.0},
	[CUBETYPE_AIR * 6 + FACEIDX_FRONT]  = {0.0, 0.0},
	[CUBETYPE_AIR * 6 + FACEIDX_RIGHT]  = {0.0, 0.0},
	[CUBETYPE_AIR * 6 + FACEIDX_LEFT]   = {0.0, 0.0},
	[CUBETYPE_AIR * 6 + FACEIDX_BOTTOM] = {0.0, 0.0},
	[CUBETYPE_AIR * 6 + FACEIDX_TOP]    = {0.0, 0.0},

	[CUBETYPE_GRASS * 6 + FACEIDX_BACK]   = {TX(1), TX(15)},
	[CUBETYPE_GRASS * 6 + FACEIDX_FRONT]  = {TX(1), TX(15)},
	[CUBETYPE_GRASS * 6 + FACEIDX_RIGHT]  = {TX(1), TX(15)},
	[CUBETYPE_GRASS * 6 + FACEIDX_LEFT]   = {TX(1), TX(15)},
	[CUBETYPE_GRASS * 6 + FACEIDX_BOTTOM] = {TX(2), TX(15)},
	[CUBETYPE_GRASS * 6 + FACEIDX_TOP]    = {TX(0), TX(15)},

	[CUBETYPE_DIRT * 6 + FACEIDX_BACK]   = {TX(2), TX(15)},
	[CUBETYPE_DIRT * 6 + FACEIDX_FRONT]  = {TX(2), TX(15)},
	[CUBETYPE_DIRT * 6 + FACEIDX_RIGHT]  = {TX(2), TX(15)},
	[CUBETYPE_DIRT * 6 + FACEIDX_LEFT]   = {TX(2), TX(15)},
	[CUBETYPE_DIRT * 6 + FACEIDX_BOTTOM] = {TX(2), TX(15)},
	[CUBETYPE_DIRT * 6 + FACEIDX_TOP]    = {TX(2), TX(15)},

	[CUBETYPE_STONE * 6 + FACEIDX_BACK]   = {TX(3), TX(15)},
	[CUBETYPE_STONE * 6 + FACEIDX_FRONT]  = {TX(3), TX(15)},
	[CUBETYPE_STONE * 6 + FACEIDX_RIGHT]  = {TX(3), TX(15)},
	[CUBETYPE_STONE * 6 + FACEIDX_LEFT]   = {TX(3), TX(15)},
	[CUBETYPE_STONE * 6 + FACEIDX_BOTTOM] = {TX(3), TX(15)},
	[CUBETYPE_STONE * 6 + FACEIDX_TOP]    = {TX(3), TX(15)},

	[CUBETYPE_SAND * 6 + FACEIDX_BACK]   = {TX(4), TX(15)},
	[CUBETYPE_SAND * 6 + FACEIDX_FRONT]  = {TX(4), TX(15)},
	[CUBETYPE_SAND * 6 + FACEIDX_RIGHT]  = {TX(4), TX(15)},
	[CUBETYPE_SAND * 6 + FACEIDX_LEFT]   = {TX(4), TX(15)},
	[CUBETYPE_SAND * 6 + FACEIDX_BOTTOM] = {TX(4), TX(15)},
	[CUBETYPE_SAND * 6 + FACEIDX_TOP] 	 = {TX(4), TX(15)},

	[CUBETYPE_LOG * 6 + FACEIDX_BACK]   = {TX(6), TX(15)},
	[CUBETYPE_LOG * 6 + FACEIDX_FRONT]  = {TX(6), TX(15)},
	[CUBETYPE_LOG * 6 + FACEIDX_RIGHT]  = {TX(6), TX(15)},
	[CUBETYPE_LOG * 6 + FACEIDX_LEFT]   = {TX(6), TX(15)},
	[CUBETYPE_LOG * 6 + FACEIDX_BOTTOM] = {TX(5), TX(15)},
	[CUBETYPE_LOG * 6 + FACEIDX_TOP]    = {TX(5), TX(15)},

	[CUBETYPE_LEAF * 6 + FACEIDX_BACK]   = {TX(7), TX(15)},
	[CUBETYPE_LEAF * 6 + FACEIDX_FRONT]  = {TX(7), TX(15)},
	[CUBETYPE_LEAF * 6 + FACEIDX_RIGHT]  = {TX(7), TX(15)},
	[CUBETYPE_LEAF * 6 + FACEIDX_LEFT]   = {TX(7), TX(15)},
	[CUBETYPE_LEAF * 6 + FACEIDX_BOTTOM] = {TX(7), TX(15)},
	[CUBETYPE_LEAF * 6 + FACEIDX_TOP]    = {TX(7), TX(15)},
};

extern bool canaries_failed(chunk_t *chunk);
extern bool is_transparent(cube_type_e type);
extern void chunk_generate_mesh(chunk_t *chunk);

#endif
