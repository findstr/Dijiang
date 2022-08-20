shader:
    stages:
        vert: asset/shaders/shader.vert.spv
        frag: asset/shaders/shader.frag.spv
    settings:
        zwrite: on
        ztest: on
        render_type: opaque
        render_queue: geometry
        light_mode: forward
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