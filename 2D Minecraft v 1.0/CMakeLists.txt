cmake_minimum_required(VERSION 3.26)
project(minecraft2d LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(FETCHCONTENT_QUIET OFF)

include(FetchContent)
FetchContent_Declare(
	SFML
	GIT_REPOSITORY https://github.com/SFML/SFML.git
	GIT_TAG 2.6.x
)
FetchContent_MakeAvailable(SFML)

add_executable(
	minecraft2d
	src/main.cpp
	src/Game.cpp
	src/audio/Audio.cpp
	src/audio/AudioEnviroment.cpp
	src/entity/Entity.cpp
	src/entity/Gravity.cpp
	src/entity/ItemEntity.cpp
	src/entity/Mob.cpp
	src/entity/Model.cpp
	src/entity/Particle.cpp
	src/entity/Player.cpp
	src/entity/ProjectileEntity.cpp
	src/entity/Spawn.cpp
	src/graphics/Render.cpp
	src/graphics/UI.cpp
	src/graphics/Window.cpp
	src/graphics/lighting/Light.cpp
	src/graphics/lighting/Lighting.cpp
	src/inventory/Crafting.cpp
	src/inventory/Item.cpp
	src/inventory/LootTable.cpp
	src/inventory/Recipe.cpp
	src/util/Input.cpp
	src/util/Loader.cpp
	src/util/Random.cpp
	src/world/Biome.cpp
	src/world/Chunk.cpp
	src/world/GameTime.cpp
	src/world/Generation.cpp
	src/world/Generator.cpp
	src/world/PathFinding.cpp
	src/world/Structure.cpp
	src/world/Tile.cpp
	src/world/TileEntity.cpp
	src/world/World.cpp
	vendor/Perlin.cpp
	vendor/FractalNoise.cpp
)
target_include_directories(
	minecraft2d
	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/hdr>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor>
	$<INSTALL_INTERFACE:include/minecraft2d>
)
target_link_libraries(
	minecraft2d
	PUBLIC
	sfml-graphics
	sfml-audio
)
target_compile_features(minecraft2d PRIVATE cxx_std_20)
