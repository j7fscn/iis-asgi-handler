# ProcessPoolLib.lib - For use in ProcessPool.dll, but compiled as a lib so that it can also be used by tests.

set(SOURCES
    GlobalModule.cpp

    ProcessPool.cpp
    JobObject.cpp
)


set(HEADERS
    GlobalModule.h

    ProcessPool.h
    JobObject.h
)


add_library(ProcessPoolLib STATIC
    ${SOURCES}
    ${HEADERS}
)

target_link_libraries(ProcessPoolLib
    SharedUtils
)

target_include_directories(ProcessPoolLib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:.>
)

