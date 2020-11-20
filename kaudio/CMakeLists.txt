include_directories(
    ${KDE1_INCLUDE_DIR}
    )

set(KAUDIO_SRCS kaudioserver.cpp)
set(MAUDIO_SRCS io_oss.cpp maudio.cpp sample.cpp)
set(KAUDIO_MOC democlient.moc kplayaudio.moc)
set(KPLAYAUDIO_SRCS kplayaudio.cpp)

QT1_WRAP_MOC(MOC_FILES kplayaudio.moc)
QT1_WRAP_MOC(DEMO_MOC_FILES democlient.moc)

add_executable(maudio ${MAUDIO_SRCS})
target_link_libraries(maudio
    ${QT_LIBRARIES}
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_MEDIATOOL}
    )

add_executable(kaudioserver ${KAUDIO_SRCS})
target_link_libraries(kaudioserver
    ${QT_LIBRARIES}
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_MEDIATOOL}
    )

if(ENABLE_COMPAT AND PULSEAUDIO_FOUND)
    target_compile_definitions(maudio PRIVATE ENABLE_PULSE)
    target_include_directories(maudio PRIVATE ${PULSEAUDIO_INCLUDE_DIR})
    target_link_libraries(maudio pulse-simple ${PULSEAUDIO_LIBRARY})
endif()

add_executable(kplayaudio ${KPLAYAUDIO_SRCS} ${MOC_FILES})
target_link_libraries(kplayaudio
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    ${KDE1_KFILE}
    ${KDE1_KFM}
    ${KDE1_MEDIATOOL}
    Qt::Qt1
    ${X11_LIBRARIES}
    )

install(TARGETS maudio kaudioserver kplayaudio RUNTIME DESTINATION ${KDE1_BINDIR})
install(FILES  maudio.html DESTINATION ${KDE1_HTMLDIR}/en/kmedia)
