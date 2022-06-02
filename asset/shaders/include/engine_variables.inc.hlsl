#ifndef ENGINE_VARIABLES_H
#define ENGINE_VARIABLES_H

cbuffer engine_per_draw : register(b0, space1) {
	float4x4 engine_matrix_model;
	float4x4 engine_matrix_view;
	float4x4 engine_matrix_proj;
}

#endif

