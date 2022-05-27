add_custom_target(shaders)
file(GLOB SHADER_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.hlsl")
foreach (FILE ${SHADER_FILES})
	get_filename_component(FILE_WE ${FILE} NAME_WE)
	add_custom_command(TARGET shaders
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMAND dxc -spirv -fvk-b-shift 0 0 -fvk-u-shift 128 0 -fvk-t-shift 256 0 -fvk-s-shift 384 0 -T vs_6_0 -E vert shaders/${FILE_WE}.hlsl -Fo shaders/${FILE_WE}.vert.spv
		COMMAND dxc -spirv -fvk-b-shift 0 0 -fvk-u-shift 128 0 -fvk-t-shift 256 0 -fvk-s-shift 384 0 -T ps_6_0 -E frag shaders/${FILE_WE}.hlsl -Fo shaders/${FILE_WE}.frag.spv
		MAIN_DEPENDENCY ${FILE}
		COMMENT "HLSL ${register} ${FILE}"
		VERBATIM)
endforeach(FILE)

