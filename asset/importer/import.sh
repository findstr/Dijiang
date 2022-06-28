 ./lua import-skin.lua "mesh" asset/objects/character/player/components/animation/data/robot.mesh_bind.json > ../models/robot.obj
 ./lua import-skin.lua "skin" asset/objects/character/player/components/animation/data/robot.mesh_bind.json > ../models/robot.skin
 ./lua import-clip.lua  asset/objects/character/player/components/animation/data/W2_CrouchWalk_Aim_F_Loop_IP.animation_clip.json  asset/objects/character/player/components/animation/data/anim.skeleton_map.json > ../animation/robot/run.anim
 ./lua import-level.lua asset/level/1-1.level.json  > ../main.level
 ./lua import-skeleton.lua asset/objects/character/player/components/animation/data/skeleton_data_root.skeleton.json > ../skeleton/avatar.skeleton