cmake_minimum_required(VERSION 3.10.2)

project(WarcraftHelper)

if(!WIN32)
	message(FATAL_ERROR "Only Windows 32-bit is supported")
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "Only win32 is supported, maybe you've forgot to specify a platform")
endif()

if(MSVC)
    # Sources are UTF-8 without a BOM; without this flag MSVC reads them using
    # the system ANSI codepage (CP936 on Chinese Windows), where a comment's
    # last UTF-8 byte can pair with the newline and swallow the next line.
    # Must come before add_subdirectory, add_compile_options only affects
    # targets created after it.
    add_compile_options(/utf-8)
endif()

add_subdirectory(WarcraftHelper)
add_subdirectory(WHLoader)
add_subdirectory(d3d9)
