add_subdirectory(kcs)
if(JPEG_FOUND)
    add_subdirectory(ad)
endif()
add_subdirectory(pics)

include_directories(
    ${QT_INCLUDE_DIR}
    ${KDE1_INCLUDE_DIR}
    )

set(DISPLAY_SRCS
backgnd.cpp display.cpp
		colorscm.cpp savescm.cpp scrnsave.cpp
		general.cpp main.cpp widgetcanvas.cpp
		fontchooser.cpp kresourceman.cpp
)

set(DISPLAY_MOC
backgnd.moc display.moc colorscm.moc savescm.moc
		scrnsave.moc general.moc widgetcanvas.moc fontchooser.moc
		kresourceman.moc
)
QT1_WRAP_MOC(MOC_FILES ${DISPLAY_MOC})

add_executable(kcmdisplay ${DISPLAY_SRCS} ${MOC_FILES})
target_link_libraries(kcmdisplay
    ${QT_LIBRARIES}
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    ${KDE1_KFILE}
    ${KDE1_KFM}
    ${KDE1_KIMGIO}
    )

install(TARGETS kcmdisplay RUNTIME DESTINATION ${KDE1_BINDIR})
