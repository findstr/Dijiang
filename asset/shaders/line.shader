shader:
    stages:
        vertex: asset/shaders/line.vert.spv
        fragment: asset/shaders/line.frag.spv
    settings:
        zwrite: off
        ztest: off
        render_type: transparent
        render_queue: geometry
        topology: line