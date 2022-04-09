#!/bin/sh
register="-fvk-b-shift 0 0 -fvk-u-shift 128 0 -fvk-t-shift 256 0 -fvk-s-shift 384 0"
dxc -spirv $register -T vs_6_0 -E vert $1/shader.hlsl -Fo $2/shader.vert.spv
dxc -spirv $register -T ps_6_0 -E frag $1/shader.hlsl -Fo $2/shader.frag.spv
