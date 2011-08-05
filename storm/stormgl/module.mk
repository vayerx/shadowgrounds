sp             := $(sp).x
dirstack_$(sp) := $(d)
d              := $(dir)

renderer_opengl:=stormgl

FILES:=IStorm3D.cpp Storm3D.cpp Storm3D_ProceduralManager.cpp \
	   Storm3D_Scene.cpp Storm3D_Scene_PicList.cpp \
	   Storm3D_Scene_PicList_Texture.cpp \
	   storm3d_resourcemanager.cpp Storm3d_Texture.cpp \
	   Storm3D_Material.cpp Storm3D_Material_TextureLayer.cpp \
	   storm3d_terrain_utils.cpp Storm3D_ParticleSystem.cpp \
	   Storm3D_ParticleSystem_PMH.cpp Storm3D_Camera.cpp \
	   Storm3D_Terrain.cpp storm3d_terrain_models.cpp \
	   storm3d_fakespotlight.cpp Storm3D_Mesh.cpp Storm3D_Face.cpp \
	   Storm3D_ShaderManager.cpp Storm3D_Mesh_CollisionTable.cpp \
	   Storm3D_Model.cpp storm3d_terrain_decalsystem.cpp \
	   storm3d_terrain_groups.cpp storm3d_spotlight.cpp \
	   storm3d_spotlight_shared.cpp Storm3D_Bone.cpp Storm3D_Model_Object.cpp \
	   storm3d_terrain_heightmap.cpp storm3d_terrain_renderer.cpp \
	   Storm3D_Helpers.cpp storm3d_terrain_lightmanager.cpp \
	   storm3d_terrain_lod.cpp Storm3D_Vertex.cpp Storm3D_Helper_Animation.cpp \
	   Storm3D_Helper_AInterface.cpp Storm3D_KeyFrames.cpp Storm3D_Font.cpp \
	   Storm3D_Scene_PicList_Font.cpp render.cpp Storm3D_Line.cpp \
	   Clipper.cpp Storm3D_Texture_Video.cpp storm3d_videostreamer.cpp \
	   storm3d_video_player.cpp treader.cpp igios.cpp


SRC_$(d):=$(addprefix $(d)/,$(FILES))

CLEANDIRS+=$(d)

-include $(foreach FILE,$(FILES),$(d)/$(FILE:.cpp=.d))

d  := $(dirstack_$(sp))
sp := $(basename $(sp))
