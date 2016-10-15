add_subdirectory(kcmbell)
add_subdirectory(kcmdisplay)
add_subdirectory(kcminfo)
add_subdirectory(kcminput)
add_subdirectory(kcmkeys)
add_subdirectory(kcmkfm)
add_subdirectory(kcmkpanel)
add_subdirectory(kcmkwm)
add_subdirectory(kcmlocale)
add_subdirectory(kcmsamba)
add_subdirectory(kcmsyssound)
add_subdirectory(kthememgr)

install(FILES
    index-1.html  index-2.html  index.html logotp3.gif
    DESTINATION ${KDE1_HTMLDIR}/en/kcontrol )
