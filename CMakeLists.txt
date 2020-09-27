cmake_minimum_required (VERSION 3.0)

project (kdebase)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

# additional cmake files
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(Qt1 REQUIRED)
find_package(KDE1 REQUIRED)

include(Qt1Macros)

set(KDEBASE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

set(BOOTSTRAP OFF)

include_directories(common ${CMAKE_CURRENT_BINARY_DIR})

set(KDE1_XDMDIR "${KDE1_CONFDIR}")
set(KDE1_UTMPPATH "/var/run/utmp") ## todo, don't hardcode

add_definitions( -DHAVE_CONFIG_H )
configure_file(common/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -Wno-write-strings")
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fpermissive -std=c++98 -Wno-write-strings")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
endif()

find_package(OpenSSL REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(CURSES ncurses)

find_package(PulseAudio)

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
add_subdirectory(kwmcom)
add_subdirectory(kwmmodules)
add_subdirectory(krootwm)
#add_subdirectory(kdm)
add_subdirectory(kscreensaver)
add_subdirectory(kmenuedit)
add_subdirectory(kaudio)
add_subdirectory(kcontrol)
#add_subdirectory(kappfinder)
add_subdirectory(pics)
add_subdirectory(applnk)
add_subdirectory(doc)
add_subdirectory(config)
add_subdirectory(kstart)
add_subdirectory(kikbd)
add_subdirectory(konsole)
add_subdirectory(po)
