include_directories (
    ${KDE1_INCLUDE_DIR}
)

set(METASOURCES
    main.moc saver.moc polygon.moc banner.moc
    blankscrn.moc rock.moc flame.moc lines.moc laser.moc
    pyro.moc hop.moc forest.moc swarm.moc bat.moc slip.moc
    lissie.moc morph3d.moc blob.moc space.moc bouboule.moc
    kgroupbox.moc science.moc pipes.moc attraction.moc
    slidescreen.moc bsod.moc matrix.moc
)

QT1_WRAP_MOC(MOC_FILES ${METASOURCES})

set(BASE_SRCS
    main.cpp xautolock.cpp saver.cpp passwd.cpp xlock.cpp
    helpers.cpp xs_colors.c xs_hsv.c xs_visual.c
    ${MOC_FILES}
    )

# I _could_ write a wrapper function, but I know vim better than cmake.
# Don't hate me because I'm beautiful.
set(kmatrix_kss_SOURCES ${BASE_SRCS} matrix.cpp)
set(kbsod_kss_SOURCES ${BASE_SRCS} bsod.cpp)
set(kpolygon_kss_SOURCES ${BASE_SRCS} polygon.cpp)
set(kswarm_kss_SOURCES ${BASE_SRCS} swarm.cpp)
set(klissie_kss_SOURCES ${BASE_SRCS} lissie.cpp)
set(kbanner_kss_SOURCES ${BASE_SRCS} banner.cpp  kgroupbox.cpp)
set(kblankscrn_kss_SOURCES ${BASE_SRCS} blankscrn.cpp)
set(kbat_kss_SOURCES ${BASE_SRCS} bat.cpp)
set(krock_kss_SOURCES ${BASE_SRCS} rock.cpp)
set(kslip_kss_SOURCES ${BASE_SRCS} slip.cpp)
set(kattraction_kss_SOURCES ${BASE_SRCS} attraction.cpp)
set(kslidescreen_kss_SOURCES ${BASE_SRCS} slidescreen.cpp)
set(kpyro_kss_SOURCES ${BASE_SRCS} pyro.cpp)
set(klines_kss_SOURCES ${BASE_SRCS} lines.cpp)
set(klaser_kss_SOURCES ${BASE_SRCS} laser.cpp)
set(khop_kss_SOURCES ${BASE_SRCS} hop.cpp)
set(kflame_kss_SOURCES ${BASE_SRCS} flame.cpp)
set(kforest_kss_SOURCES ${BASE_SRCS} forest.cpp)
set(kblob_kss_SOURCES ${BASE_SRCS} blob.cpp)
set(kbouboule_kss_SOURCES ${BASE_SRCS} bouboule.cpp)
set(kmorph3d_kss_SOURCES ${BASE_SRCS} morph3d.cpp)
set(kpipes_kss_SOURCES ${BASE_SRCS} pipes.cpp)
set(kspace_kss_SOURCES ${BASE_SRCS} space.cpp)
set(kscience_kss_SOURCES ${BASE_SRCS} science.cpp)

set(klock_SOURCES klock.cpp kscreensave.cpp)

add_executable(kmatrix.kss ${kmatrix_kss_SOURCES})
add_executable(kpolygon.kss ${kpolygon_kss_SOURCES})
add_executable(kswarm.kss ${kswarm_kss_SOURCES})
add_executable(klissie.kss ${klissie_kss_SOURCES})
add_executable(kbanner.kss ${kbanner_kss_SOURCES})
add_executable(klines.kss ${klines_kss_SOURCES})
add_executable(kblankscrn.kss ${kblankscrn_kss_SOURCES})
add_executable(kbat.kss ${kbat_kss_SOURCES})
add_executable(krock.kss ${krock_kss_SOURCES})
add_executable(kflame.kss ${kflame_kss_SOURCES})
add_executable(klaser.kss ${klaser_kss_SOURCES})
add_executable(kpyro.kss ${kpyro_kss_SOURCES})
add_executable(khop.kss ${khop_kss_SOURCES})
add_executable(kslip.kss ${kslip_kss_SOURCES})
add_executable(kforest.kss ${kforest_kss_SOURCES})
add_executable(kblob.kss ${kblob_kss_SOURCES})
add_executable(kbouboule.kss ${kbouboule_kss_SOURCES})
add_executable(kscience.kss ${kscience_kss_SOURCES})
add_executable(kattraction.kss ${kattraction_kss_SOURCES})
add_executable(kslidescreen.kss ${kslidescreen_kss_SOURCES})
add_executable(kbsod.kss ${kbsod_kss_SOURCES})

set(BASE_LIBRARIES
    Qt::Qt1
    X11::X11
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    crypt
    )

target_link_libraries(kmatrix.kss ${BASE_LIBRARIES} X11::Xpm)
target_link_libraries(kpolygon.kss ${BASE_LIBRARIES})
target_link_libraries(kswarm.kss ${BASE_LIBRARIES})
target_link_libraries(klissie.kss ${BASE_LIBRARIES})
target_link_libraries(kbanner.kss ${BASE_LIBRARIES})
target_link_libraries(klines.kss ${BASE_LIBRARIES})
target_link_libraries(kblankscrn.kss ${BASE_LIBRARIES})
target_link_libraries(kbat.kss ${BASE_LIBRARIES} X11::Xpm)
target_link_libraries(krock.kss ${BASE_LIBRARIES})
target_link_libraries(kflame.kss ${BASE_LIBRARIES})
target_link_libraries(klaser.kss ${BASE_LIBRARIES})
target_link_libraries(kpyro.kss ${BASE_LIBRARIES})
target_link_libraries(khop.kss ${BASE_LIBRARIES})
target_link_libraries(kslip.kss ${BASE_LIBRARIES})
target_link_libraries(kforest.kss ${BASE_LIBRARIES})
target_link_libraries(kblob.kss ${BASE_LIBRARIES})
target_link_libraries(kbouboule.kss ${BASE_LIBRARIES})
target_link_libraries(kscience.kss ${BASE_LIBRARIES})
target_link_libraries(kattraction.kss ${BASE_LIBRARIES})
target_link_libraries(kslidescreen.kss ${BASE_LIBRARIES})
target_link_libraries(kbsod.kss ${BASE_LIBRARIES} X11::Xpm)


find_package(OpenGL COMPONENTS OpenGL GLX)
if (OPENGL_FOUND)
    add_definitions(-DHAVE_GL)

    add_executable(kmorph3d.kss ${kmorph3d_kss_SOURCES})
    add_executable(kpipes.kss ${kpipes_kss_SOURCES})
    add_executable(kspace.kss ${kspace_kss_SOURCES})

    target_link_libraries(kmorph3d.kss ${BASE_LIBRARIES} OpenGL::GLX)
    target_link_libraries(kpipes.kss ${BASE_LIBRARIES} OpenGL::GLU)
    target_link_libraries(kspace.kss ${BASE_LIBRARIES} OpenGL::GLU OpenGL::GLX)
endif()

