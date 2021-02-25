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
set(kscience_kss_SOURCES ${BASE_SRCS} science.cpp)

set(klock_SOURCES klock.cpp kscreensave.cpp)

add_executable(kmatrix.kss1 ${kmatrix_kss_SOURCES})
add_executable(kpolygon.kss1 ${kpolygon_kss_SOURCES})
add_executable(kswarm.kss1 ${kswarm_kss_SOURCES})
add_executable(klissie.kss1 ${klissie_kss_SOURCES})
add_executable(kbanner.kss1 ${kbanner_kss_SOURCES})
add_executable(klines.kss1 ${klines_kss_SOURCES})
add_executable(kblankscrn.kss1 ${kblankscrn_kss_SOURCES})
add_executable(kbat.kss1 ${kbat_kss_SOURCES})
add_executable(krock.kss1 ${krock_kss_SOURCES})
add_executable(kflame.kss1 ${kflame_kss_SOURCES})
add_executable(klaser.kss1 ${klaser_kss_SOURCES})
add_executable(kpyro.kss1 ${kpyro_kss_SOURCES})
add_executable(khop.kss1 ${khop_kss_SOURCES})
add_executable(kslip.kss1 ${kslip_kss_SOURCES})
add_executable(kforest.kss1 ${kforest_kss_SOURCES})
add_executable(kblob.kss1 ${kblob_kss_SOURCES})
add_executable(kbouboule.kss1 ${kbouboule_kss_SOURCES})
add_executable(kscience.kss1 ${kscience_kss_SOURCES})
add_executable(kattraction.kss1 ${kattraction_kss_SOURCES})
add_executable(kslidescreen.kss1 ${kslidescreen_kss_SOURCES})
add_executable(kbsod.kss1 ${kbsod_kss_SOURCES})
add_executable(klock ${klock_SOURCES})

set(BASE_LIBRARIES
    Qt::Qt1
    X11::X11
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    crypt
    )

if (PAM_FOUND)
    set(BASE_LIBRARIES ${BASE_LIBRARIES} ${PAM_LIBRARIES})
endif()

target_link_libraries(kmatrix.kss1 ${BASE_LIBRARIES} X11::Xpm)
target_link_libraries(kpolygon.kss1 ${BASE_LIBRARIES})
target_link_libraries(kswarm.kss1 ${BASE_LIBRARIES})
target_link_libraries(klissie.kss1 ${BASE_LIBRARIES})
target_link_libraries(kbanner.kss1 ${BASE_LIBRARIES})
target_link_libraries(klines.kss1 ${BASE_LIBRARIES})
target_link_libraries(kblankscrn.kss1 ${BASE_LIBRARIES})
target_link_libraries(kbat.kss1 ${BASE_LIBRARIES} X11::Xpm)
target_link_libraries(krock.kss1 ${BASE_LIBRARIES})
target_link_libraries(kflame.kss1 ${BASE_LIBRARIES})
target_link_libraries(klaser.kss1 ${BASE_LIBRARIES})
target_link_libraries(kpyro.kss1 ${BASE_LIBRARIES})
target_link_libraries(khop.kss1 ${BASE_LIBRARIES})
target_link_libraries(kslip.kss1 ${BASE_LIBRARIES})
target_link_libraries(kforest.kss1 ${BASE_LIBRARIES})
target_link_libraries(kblob.kss1 ${BASE_LIBRARIES})
target_link_libraries(kbouboule.kss1 ${BASE_LIBRARIES})
target_link_libraries(kscience.kss1 ${BASE_LIBRARIES})
target_link_libraries(kattraction.kss1 ${BASE_LIBRARIES})
target_link_libraries(kslidescreen.kss1 ${BASE_LIBRARIES})
target_link_libraries(kbsod.kss1 ${BASE_LIBRARIES} X11::Xpm)
target_link_libraries(klock ${BASE_LIBRARIES})

find_package(OpenGL COMPONENTS OpenGL GLX)
if (OPENGL_FOUND)
    add_definitions(-DHAVE_GL)

    set(kmorph3d_kss_SOURCES ${BASE_SRCS} morph3d.cpp)
    set(kpipes_kss_SOURCES ${BASE_SRCS} pipes.cpp)
    set(kspace_kss_SOURCES ${BASE_SRCS} space.cpp)

    add_executable(kmorph3d.kss1 ${kmorph3d_kss_SOURCES})
    add_executable(kpipes.kss1 ${kpipes_kss_SOURCES})
    add_executable(kspace.kss1 ${kspace_kss_SOURCES})

    target_link_libraries(kmorph3d.kss1 ${BASE_LIBRARIES} OpenGL::GLX)
    target_link_libraries(kpipes.kss1 ${BASE_LIBRARIES} OpenGL::GLU)
    target_link_libraries(kspace.kss1 ${BASE_LIBRARIES} OpenGL::GLU OpenGL::GLX)

    install(TARGETS kmorph3d.kss1 RUNTIME DESTINATION ${KDE1_BINDIR})
    install(TARGETS kpipes.kss1 RUNTIME DESTINATION ${KDE1_BINDIR})
    install(TARGETS kspace.kss1 RUNTIME DESTINATION ${KDE1_BINDIR})
endif()

install(TARGETS kmatrix.kss1 RUNTIME DESTINATION ${KDE1_BINDIR})
install(TARGETS kpolygon.kss1 RUNTIME DESTINATION ${KDE1_BINDIR})
install(TARGETS kswarm.kss1 RUNTIME DESTINATION ${KDE1_BINDIR})
install(TARGETS klissie.kss1 RUNTIME DESTINATION ${KDE1_BINDIR})
install(TARGETS kbanner.kss1 RUNTIME DESTINATION ${KDE1_BINDIR})
install(TARGETS klines.kss1 RUNTIME DESTINATION ${KDE1_BINDIR})
install(TARGETS kblankscrn.kss1 RUNTIME DESTINATION ${KDE1_BINDIR})
install(TARGETS kbat.kss1 RUNTIME DESTINATION ${KDE1_BINDIR})
install(TARGETS krock.kss1 RUNTIME DESTINATION ${KDE1_BINDIR})
install(TARGETS kflame.kss1 RUNTIME DESTINATION ${KDE1_BINDIR})
install(TARGETS klaser.kss1 RUNTIME DESTINATION ${KDE1_BINDIR})
install(TARGETS kpyro.kss1 RUNTIME DESTINATION ${KDE1_BINDIR})
install(TARGETS khop.kss1 RUNTIME DESTINATION ${KDE1_BINDIR})
install(TARGETS kslip.kss1 RUNTIME DESTINATION ${KDE1_BINDIR})
install(TARGETS kforest.kss1 RUNTIME DESTINATION ${KDE1_BINDIR})
install(TARGETS kblob.kss1 RUNTIME DESTINATION ${KDE1_BINDIR})
install(TARGETS kbouboule.kss1 RUNTIME DESTINATION ${KDE1_BINDIR})
install(TARGETS kscience.kss1 RUNTIME DESTINATION ${KDE1_BINDIR})
install(TARGETS kattraction.kss1 RUNTIME DESTINATION ${KDE1_BINDIR})
install(TARGETS kslidescreen.kss1 RUNTIME DESTINATION ${KDE1_BINDIR})
install(TARGETS kbsod.kss1 RUNTIME DESTINATION ${KDE1_BINDIR})

install(TARGETS klock RUNTIME DESTINATION ${KDE1_BINDIR})

