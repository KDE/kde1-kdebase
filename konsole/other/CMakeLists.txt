install(FILES
BlackOnLightYellow.schema BlackOnWhite.schema
Example.Schema GreenOnBlack.schema WhiteOnBlack.schema
default.Schema linux.kdelnk mc.kdelnk shell.kdelnk
syscolor.schema Linux.schema XTerm.schema vim.schema
DarkPicture.schema LightPicture.schema su.kdelnk
DESTINATION ${KDE1_DATADIR}/konsole )

install_icon(konsole.xpm "")
install_icon(colourness.xpm "")
install_icon(brightness.xpm "")
install_icon(contrast.xpm "")

install(FILES
    linux8x16.pcf.gz  linux8x8.pcf.gz  vga.pcf.gz
    DESTINATION share/fonts/misc
)
