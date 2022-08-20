shader:
    stages:
        vert: asset/shaders/shadowcaster.vert.spv
        frag: asset/shaders/shadowcaster.frag.spv
    settings:
        zwrite: on
        ztest: on
        render_type: opaque
        render_queue: geometry
        light_mode: shadow_caster
    properties:
        -
            name: albedo
            type: texture2d
        -
            name: roughness
            type: texture2d
        -
            name: normal
            type: texture2d