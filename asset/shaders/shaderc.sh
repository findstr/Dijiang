compile() {
	echo "compile $1"
	dxc -Zi -spirv -fvk-b-shift 0 1 -fvk-u-shift 128 1 -fvk-t-shift 256 1 -fvk-s-shift 384 1 -T vs_6_0 -E vert $1.hlsl -Fo $1.vert.spv
	dxc -Zi -spirv -fvk-b-shift 0 1 -fvk-u-shift 128 1 -fvk-t-shift 256 1 -fvk-s-shift 384 1 -T ps_6_0 -E frag $1.hlsl -Fo $1.frag.spv
}

for file in *.hlsl
do
	filename=${file%.*}
	compile "$filename"
done
