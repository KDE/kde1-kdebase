add_subdirectory(icons)

include_directories(
    ${QT_INCLUDE_DIR}
    ${KDE1_INCLUDE_DIR}
    )

set(KMENUEDIT_SRCS
kmenuedit.cpp main.cpp pmenu.cpp
	cpopmenu.cpp confmenu.cpp button.cpp IconPathDialog.cpp
	IconPathDialogData.cpp DesktopPathDialog.cpp
	DesktopPathDialogData.cpp entrydialog.cpp
	MenuNameDialog.cpp MenuNameDialogData.cpp )

set(KMENUEDIT_MOC
kmenuedit.moc pmenu.moc cpopmenu.moc confmenu.moc
	button.moc IconPathDialog.moc IconPathDialogData.moc
	DesktopPathDialog.moc DesktopPathDialogData.moc
	entrydialog.moc	MenuNameDialog.moc MenuNameDialogData.moc)

QT1_WRAP_MOC(MOC_FILES ${KMENUEDIT_MOC})

add_executable(kmenuedit ${KMENUEDIT_SRCS} ${MOC_FILES})
target_link_libraries(kmenuedit
    ${QT_LIBRARIES}
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    ${KDE1_KFM}
    ${KDE1_KFILE}
    )

install(TARGETS kmenuedit RUNTIME DESTINATION ${KDE1_BINDIR})

install_icon(kmenuedit.xpm "")
