#ifndef ENGINE_LIGHTING_H
#define ENGINE_LIGHTING_H

#include "engine_constant.inc.hlsl"
#include "engine_variables.inc.hlsl"
 
float3 engine_world_view_dir(float3 vertex_world_position)
{
	return engine_camera_pos - vertex_world_position;
}
 
float3 engine_light_blin_phong(float3 albedo, float smooth, float3 ldir, float3 vdir, float3 normal)
{
	ldir *= -1; //convert light dir from fragment to light
	float diff = max(0, dot(normal, ldir));
	float3 h = normalize(vdir + ldir);
	float nh = max(0, dot(normal, h));
	float3 spec = pow(nh, 128)  * smooth * float3(1,1,1);

	return ((diff + spec) * engine_light_intensity + engine_light_ambient) * albedo;
}


float3 engine_light_pbs(float3 albedo, float smooth, float3 ldir, float3 vdir, float3 normal)
{

} 
 
#endif

