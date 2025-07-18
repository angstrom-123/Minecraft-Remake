#include "geometry.h"

static float* _unpack(uint32_t packed)
{
	const float a = ((float) (packed >> 16)) / 65535.0;
	const float b = ((float) (packed & 0xFFFF)) / 65535.0;

	return (float[]) {a, b};
}

static cube_type_e _get_neighbour(uint8_t x, uint8_t y, uint8_t z,
						   		  cube_face_idx_e face, chunk_data_t *blocks)
{
	switch (face) {
	case FACEIDX_BACK:
		if (z == 0) 
			return CUBETYPE_AIR;
		return blocks->types[x][y][z - 1];

	case FACEIDX_FRONT:
		if (z == CHUNK_SIZE - 1) 
			return CUBETYPE_AIR;
		return blocks->types[x][y][z + 1];

	case FACEIDX_RIGHT:
		if (x >= CHUNK_SIZE - 1) 
			return CUBETYPE_AIR;
		return blocks->types[x + 1][y][z];

	case FACEIDX_LEFT:
		if (x == 0) 
			return CUBETYPE_AIR;
		return blocks->types[x - 1][y][z];

	case FACEIDX_BOTTOM:
		if (y == 0) 
			return CUBETYPE_AIR;
		return blocks->types[x][y - 1][z];

	case FACEIDX_TOP:
		if (y == CHUNK_HEIGHT - 1) 
			return CUBETYPE_AIR;
		return blocks->types[x][y + 1][z];

	};
}

static void _add_face_to_mesh(quad_desc_t *desc, 
					   		  uint32_t *i_cnt, uint32_t *v_cnt,
					   		  uint32_t *indices, vertex_t *vertices)
{
	vertex_t verts[4] = {
		face_vertices[desc->face][0],
		face_vertices[desc->face][1],
		face_vertices[desc->face][2],
		face_vertices[desc->face][3]
	};

	for (size_t i = 0; i < 4; i++)
	{
		float *uv = _unpack(verts[i].uv);
		uv[0] += uv_lookup[(desc->type * 6) + desc->face].x;
		uv[1] += uv_lookup[(desc->type * 6) + desc->face].y;

		verts[i].uv = PACK(uv[0], uv[1]);
		verts[i].x += (desc->x * 2);
		verts[i].y += (desc->y * 2);
		verts[i].z += (desc->z * 2);
	}

	const uint32_t v_ofst = *v_cnt;
	for (size_t i = 0; i < 4; i++) 
		vertices[v_ofst + i] = verts[i];

	*v_cnt += 4;

	/* 
	 * Verts are ordered counter-clockwise in the buffer, so this index 
	 * ordering has clockwise winding.
	 */
	const uint32_t i_ofst = *i_cnt;
	indices[i_ofst + 0] = v_ofst + 0;
	indices[i_ofst + 1] = v_ofst + 2;
	indices[i_ofst + 2] = v_ofst + 1;
	indices[i_ofst + 3] = v_ofst + 0;
	indices[i_ofst + 4] = v_ofst + 3;
	indices[i_ofst + 5] = v_ofst + 2;

	*i_cnt += 6;
}

bool canaries_failed(chunk_t *c)
{
	return ((c->mesh.__start_canary != START_CANARY_VAL) 
			|| (c->mesh.__mid_canary != MID_CANARY_VAL) 
			|| (c->mesh.__end_canary != END_CANARY_VAL));
}

bool is_transparent(cube_type_e type)
{
	switch (type) {
		case CUBETYPE_AIR: 
			return true;

		case CUBETYPE_LEAF: 
			return true;

		default: 
			return false;

	};
}

void chunk_generate_mesh(chunk_t *c)
{
	c->mesh.__start_canary = START_CANARY_VAL;
	c->mesh.__mid_canary = MID_CANARY_VAL;
	c->mesh.__end_canary = END_CANARY_VAL;

	/* 
	 * Allocating max required amount so all buffers are the same size in memory,
	 * and can replace eachother easily when needed.
	 */
	uint32_t v_cnt = 0;
	uint32_t i_cnt = 0;

	vertex_t *v_buf = malloc(V_MAX * sizeof(vertex_t));
	uint32_t *i_buf = malloc(I_MAX * sizeof(uint32_t));
	if (!v_buf || !i_buf)
	{
		fprintf(stderr, "Failed to allocate chunk buffers.\n");
		exit(1);
	}

	/* Iterating x, y, z, as one dimension in row-major order for less nesting. */
	int8_t *ptr = (int8_t *) c->blocks->types;
	for (size_t i = 0; i < CHUNK_SIZE * CHUNK_HEIGHT * CHUNK_SIZE; i++)
	{
		const cube_type_e type = *ptr;
		ptr++;

		if (type == CUBETYPE_AIR) 
		{
			continue; // Ignore empty space.
		}

		const uint8_t x = i / (CHUNK_HEIGHT * CHUNK_SIZE);
		const uint8_t y = (i / CHUNK_SIZE) % CHUNK_HEIGHT;
		const uint8_t z = i % CHUNK_SIZE;

		/* 
		 * Only block faces that are next to transparency (such as air or some 
		 * blocks) are meshed. Chunk borders are automatically meshed to avoid 
		 * looking up adjacent chunks.
		 */
		if (v_cnt + 24 >= V_MAX)
		{
			fprintf(stderr, "Chunk buffer overflow.\n");
			exit(1);
		}
		for (uint8_t face = 0; face < 6; face++)
		{
			const cube_type_e nhbr = _get_neighbour(x, y, z, face, c->blocks);
			if (!is_transparent(nhbr)) 
				continue; // Opaque neighbour on this side, block face not visible.

			_add_face_to_mesh(&(quad_desc_t) {
				.x = x,
				.y = y,
				.z = z,
				.type = type,
				.face = face
			}, &i_cnt, &v_cnt, i_buf, v_buf);
		}
	}

	if (canaries_failed(c))
	{
		printf("Chunk buffer overrun in generate mesh.\n");
		exit(1);
	}

	/* Ignore empty chunks. */
	if (i_cnt == 0)
	{
		c->mesh.v_rsrv = 0;
		c->mesh.i_rsrv = 0;
		c->mesh.v_cnt = 0;
		c->mesh.i_cnt = 0;
		c->mesh.v_buf = NULL;
		c->mesh.i_buf = NULL;
	}
	else 
	{
		c->mesh.v_rsrv = V_MAX;
		c->mesh.i_rsrv = I_MAX;
		c->mesh.v_cnt = v_cnt;
		c->mesh.i_cnt = i_cnt;
		c->mesh.v_buf = v_buf;
		c->mesh.i_buf = i_buf;
	}
}
