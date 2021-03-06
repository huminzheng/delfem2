cmake_minimum_required(VERSION 3.12)

option(USE_HEADERONLY "USE_DFM2_HEADERONLY" OFF)

####################################

enable_language(CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
IF(MSVC)
  set(CMAKE_CXX_FLAGS "/W4 -O2 \
    /wd4100 /wd4458 /wd4577 /wd4267 /wd4244 /wd4505 /wd4838 \
    /wd4800 /wd4996 /wd4530 /wd4245 /wd4505 /wd4505 /wd4456 ")
ELSE()
  set(CMAKE_CXX_FLAGS "-Wall -Wno-deprecated-declarations -g")
ENDIF()

####################################

project(625_DefLaplacianFitProj)

add_definitions(-DPATH_INPUT_DIR="${PROJECT_SOURCE_DIR}")

# glad
get_filename_component(GLAD_INCLUDE_DIR ../../3rd_party/glad3/include ABSOLUTE)

# glfw
IF(MSVC)
  set(GLFW_LIBRARY    ../../3rd_party/glfw/src/Release/glfw3.lib)
ELSE()
  set(GLFW_LIBRARY    ../../3rd_party/glfw/src/libglfw3.a)
ENDIF()
get_filename_component(GLFW_LIBRARY ${GLFW_LIBRARY} ABSOLUTE)
get_filename_component(GLFW_INCLUDE_DIR ../../3rd_party/glfw/include ABSOLUTE)

# opengl
find_package(OpenGL REQUIRED)

# dfm2
IF(USE_HEADERONLY)
  add_definitions(-DDFM2_HEADER_ONLY=ON)
else() 
  if(MSVC)
    set(DFM2_LIBRARY
      ${PROJECT_SOURCE_DIR}/../dfm2_static_glfwold/dfm2_static_glfwold.lib )
  else()
    set(DFM2_LIBRARY
      ${PROJECT_SOURCE_DIR}/../dfm2_static_glfwold/libdfm2_static_glfwold.a )
  endif()
ENDIF()
set(DELFEM2_INCLUDE_DIR "../../include")

#####################################################

include_directories(
    ${OPENGL_INCLUDE_DIR}
    ${GLAD_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIR}
    ${DELFEM2_INCLUDE_DIR}
)
set(3RD_PARTY   "../../3rd_party")
add_executable(${PROJECT_NAME}
    ${3RD_PARTY}/glad3/include/glad/glad.h
    ${3RD_PARTY}/glad3/src/glad.c
    main.cpp
)

#####################################

IF(APPLE)
  find_library( COCOA_LIBRARY Cocoa )
  find_library( IOKIT_LIBRARY IOKit )
  find_library( COREVIDEO_LIBRARY CoreVideo )
  target_link_libraries(${PROJECT_NAME}
    ${OPENGL_LIBRARY}
    ${GLFW_LIBRARY}
    ${COCOA_LIBRARY}
    ${IOKIT_LIBRARY}
    ${COREVIDEO_LIBRARY}
    ${DFM2_LIBRARY}
    )
ELSEIF(UNIX)
  message(STATUS "opengl: ${OPENGL_LIBRARY}")
  message(STATUS "glfw: ${GLFW_LIBRARY}")
  message(STATUS "dfm2: ${DFM2_LIBRARY}")
  target_link_libraries(${PROJECT_NAME}
    ${DFM2_LIBRARY}
    ${OPENGL_LIBRARY}
    ${GLFW_LIBRARY}
    -lrt -lm -ldl -lX11 -lpthread -lxcb -lXau -lXdmcp
    )
ELSEIF(MSVC)
  target_link_libraries(${PROJECT_NAME}
      ${GLFW_LIBRARY}
      ${OPENGL_LIBRARY}
      ${DFM2_LIBRARY})
ENDIF()
