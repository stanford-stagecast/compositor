cmake_minimum_required ( VERSION 2.8.5 )
cmake_policy ( SET CMP0054 NEW )
cmake_policy ( SET CMP0072 NEW )
project ( StageCast-Compositor )

include ( FindJPEG )

find_package ( JPEG REQUIRED )
find_package ( OpenGL REQUIRED )
find_package ( GLEW REQUIRED )
find_package ( glfw3 3.3 REQUIRED )


include ( etc/build_defs.cmake )
include ( etc/build_type.cmake )
include ( etc/cflags.cmake )

include ( etc/clang_format.cmake )
include ( etc/clang_tidy.cmake )

file ( GLOB COMPOSITOR_SRC
    src/util/*
    src/display/*
    src/input/*
)

include_directories ( src )

add_library ( compositor STATIC
    ${COMPOSITOR_SRC}
)

set ( COMPOSITOR_LIBS
    compositor
    glfw
    GLEW::GLEW
    OpenGL::GL
    OpenGL::GLU
    JPEG::JPEG
)

add_executable ( real-webcam src/frontend/real-webcam.cc )
target_link_libraries ( real-webcam ${COMPOSITOR_LIBS} )
