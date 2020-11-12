install(FILES .directory DESTINATION ${KDE1_DATADIR}/kappfinder/apps)

install(FILES
    Editors/.directory
    Editors/emacs.kdelnk
    Editors/lemacs.kdelnk
    Editors/xedit.kdelnk
    Editors/xemacs.kdelnk
    Editors/nedit.kdelnk
    DESTINATION ${KDE1_DATADIR}/kappfinder/apps/Editors
)

install(FILES
    Internet/.directory
    Internet/arena.kdelnk
    Internet/netscape.kdelnk
    Internet/xftp.kdelnk
    Internet/coolmail.kdelnk
    Internet/pine.kdelnk
    Internet/xfmail.kdelnk
    Internet/xbiff.kdelnk
    Internet/chromium.kdelnk

    DESTINATION ${KDE1_DATADIR}/kappfinder/apps/Internet
)

install(FILES
    Toys/.directory
    Toys/xpinguin.kdelnk
    Toys/xeyes.kdelnk
    DESTINATION ${KDE1_DATADIR}/kappfinder/apps/Toys
)

install(FILES
    WordProcessing/.directory
    WordProcessing/acroread.kdelnk
    WordProcessing/ghostview.kdelnk
    WordProcessing/gv.kdelnk
    WordProcessing/lyx.kdelnk
    WordProcessing/xdvi.kdelnk
    WordProcessing/xpdf.kdelnk
    WordProcessing/WordPerfect.kdelnk
    DESTINATION ${KDE1_DATADIR}/kappfinder/apps/WordProcessing
)

install(FILES
    Development/.directory
    Development/dlgedit.kdelnk
    Development/ddd.kdelnk
    DESTINATION ${KDE1_DATADIR}/kappfinder/apps/Development
)

install(FILES
    Graphics/.directory
    Graphics/xfig.kdelnk
    Graphics/xpaint.kdelnk
    Graphics/xv.kdelnk
    Graphics/acroread.kdelnk
    DESTINATION ${KDE1_DATADIR}/kappfinder/apps/Graphics
)

install(FILES
    System/.directory
    System/xosview.kdelnk
    DESTINATION ${KDE1_DATADIR}/kappfinder/apps/System
)

install(FILES
    Utilities/.directory
    Utilities/xcalc.kdelnk
    Utilities/xclipboard.kdelnk
    Utilities/xclock.kdelnk
    Utilities/xconsole.kdelnk
    Utilities/xterm.kdelnk
    Utilities/xload.kdelnk
    Utilities/passwd.kdelnk
    DESTINATION ${KDE1_DATADIR}/kappfinder/apps/Utilities
)

install(FILES
    Office/.directory
    Office/Applix.kdelnk
    Office/XAcc.kdelnk
    Office/XsLite.kdelnk
    Office/plan.kdelnk
    DESTINATION ${KDE1_DATADIR}/kappfinder/apps/Office
)
