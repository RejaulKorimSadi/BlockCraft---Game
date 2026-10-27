# BlockCraft by Md Rejaul Korim Sadi (231-115-349)
cmake_minimum_required(VERSION 3.10)
project(BlockCraft VERSION 1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(OpenGL_GL_PREFERENCE LEGACY)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

set(BLOCKCRAFT_LIB lib/world.cpp lib/textures.cpp lib/player.cpp lib/game.cpp lib/sky.cpp lib/effects.cpp lib/hud.cpp)
add_executable(BlockCraft src/main.cpp ${BLOCKCRAFT_LIB})
target_include_directories(BlockCraft PRIVATE ${GLUT_INCLUDE_DIRS})
target_link_libraries(BlockCraft PRIVATE ${GLUT_LIBRARIES} OpenGL::GL OpenGL::GLU)

add_executable(world_test tests/world_test.cpp lib/world.cpp lib/textures.cpp)
target_include_directories(world_test PRIVATE ${GLUT_INCLUDE_DIRS})
target_link_libraries(world_test PRIVATE ${GLUT_LIBRARIES} OpenGL::GL OpenGL::GLU)

enable_testing()
add_test(NAME world_test COMMAND world_test)
