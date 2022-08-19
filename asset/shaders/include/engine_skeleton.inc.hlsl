#ifndef ENGINE_SKELETON_H
#define ENGINE_SKELETON_H

#include "engine_constant.inc.hlsl"
#include "include/engine_variables.inc.hlsl"

struct engine_skeleton_vertex {
	int4 bone_indices : BLENDINDICES0;
	float4 bone_weights : BLENDWEIGHT0;
};

float4x4 engine_skeleton_vertex_blend(uint instanceid, engine_skeleton_vertex blend)
{
	float4x4 bone_blending = {
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
	};	
	for (int i = 0; i < 4; i++)
		bone_blending += engine_bindless_objects[instanceid].engine_skeleton_matrix[blend.bone_indices[i]] * blend.bone_weights[i];
	return bone_blending;
}
 
#endif

