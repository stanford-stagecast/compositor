cmake_minimum_required ( VERSION 2.8.5 )
cmake_policy ( SET CMP0054 NEW )
project ( StageCast-Compositor )

include ( FindJPEG )

find_package ( JPEG REQUIRED )
find_package ( OpenGL REQUIRED )
find_package ( GLEW REQUIRED )
find_package ( glfw3 REQUIRED )


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
    GLEW
    GL
    GLU
    jpeg
    pthread
)

add_executable ( real-webcam src/frontend/real-webcam.cc )
target_link_libraries ( real-webcam ${COMPOSITOR_LIBS} )

add_executable ( test-harness src/frontend/test-harness.cc )
target_link_libraries ( test-harness ${COMPOSITOR_LIBS} )
