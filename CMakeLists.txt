cmake_minimum_required (VERSION 3.0)

project (kdebase)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

# additional cmake files
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(KDE1 REQUIRED)

set(KDEBASE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

set(BOOTSTRAP OFF)

include_directories(common)

add_definitions( -DHAVE_CONFIG_H )

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -Wno-write-strings")
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fpermissive -std=c++98")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(CURSES ncurses)
pkg_check_modules(JPEG jpeg)

add_subdirectory(kcheckpass)
add_subdirectory(kfm)
add_subdirectory(mimetypes)
add_subdirectory(kdehelp)
add_subdirectory(kfontmanager)
add_subdirectory(kfind)
if(CURSES_FOUND)
add_subdirectory(kvt)
endif()
add_subdirectory(kwm)
add_subdirectory(kpanel)
#add_subdirectory(kwmcom)
#add_subdirectory(kmmodules)
#add_subdirectory(krootwm)
#add_subdirectory(kdm)
#add_subdirectory(kscreensaver)
#add_subdirectory(kmenuedit)
#add_subdirectory(kaudio)
add_subdirectory(kcontrol)
#add_subdirectory(kappfinder)
add_subdirectory(pics)
#add_subdirectory(applnk)
#add_subdirectory(doc)
#add_subdirectory(config)
#add_subdirectory(kstart)
#add_subdirectory(kikbd)
add_subdirectory(konsole)
#add_subdirectory(po)
