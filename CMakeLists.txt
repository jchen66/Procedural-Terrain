# the exercise name is nothing else than the directory
get_filename_component(EXERCISENAME ${CMAKE_CURRENT_LIST_DIR} NAME)
file(GLOB_RECURSE SOURCES "*.cpp")
file(GLOB_RECURSE HEADERS "*.h")

# these files are copied only once
file(GLOB TEXTURES
    cube/cube_texture.tga
    grid/grid_texture.tga
    grid/grass_texture.tga
    grid/rock_texture.tga
    grid/snow_texture.tga
    grid/sand_texture.tga
    grid/cloud_texture.tga
    sea/sea_texture.tga
    sea/waterdudv.tga
    sea/normalmap.tga)
copy_files_once(${TEXTURES})

# list all the files you want to copy everytime
# you build (ie, you want the shaders there)
file(GLOB SHADERS
	cube/cube_vshader.glsl
	cube/cube_fshader.glsl
	grid/grid_vshader.glsl
	grid/grid_fshader.glsl
	heightmap/heightmap_vshader.glsl
	heightmap/heightmap_fshader.glsl
	tree/tree_vshader.glsl
	tree/tree_fshader.glsl 
	water/water_vshader.glsl
	water/water_fshader.glsl  
    sea/sea_vshader.glsl
    sea/sea_fshader.glsl
    snow/snow_vshader.glsl
    snow/snow_fshader.glsl
    cloudmap/cloudmap_vshader.glsl
    cloudmap/cloudmap_fshader.glsl
)
deploy_shaders_to_build_dir(${SHADERS})

add_executable(${EXERCISENAME} ${SOURCES} ${HEADERS} ${SHADERS})
target_link_libraries(${EXERCISENAME} ${COMMON_LIBS})
