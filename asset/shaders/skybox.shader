shader:
    stages:
        vert: asset/shaders/skybox.vert.spv
        frag: asset/shaders/skybox.frag.spv
    settings:
        zwrite: off
        ztest: on
        render_type: opaque
        render_queue: geometry
        light_mode: forward
    properties:
        -
            name: tex
            type: cubemap