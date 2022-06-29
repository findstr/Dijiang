#ifndef ENGINE_VARIABLES_H
#define ENGINE_VARIABLES_H

#include "engine_constant.inc.hlsl"

cbuffer engine_per_frame : register(b1, space1) {
	float4 light_pos;
	float4 light_dir;
	float4 light_color;
}

cbuffer engine_per_draw : register(b0, space1) {
	float4x4 engine_matrix_model;
	float4x4 engine_matrix_view;
	float4x4 engine_matrix_proj;
	float4x4 engine_skeleton_matrix[ENGINE_MAX_BONE_PER_SKELETON];
}
 
#endif

