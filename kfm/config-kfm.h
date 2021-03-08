#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// #define TORBENSDEBUG
// #define COOLOSDEBUG
void debugT( const char *msg , ...);
void debugC( const char *msg , ...);


#define ID_STRING_OPEN 1 /* Open */
#define ID_STRING_CD   2 /* Cd */
#define ID_STRING_NEW_VIEW 3 /* New View */
#define ID_STRING_COPY  4 /* Copy */
#define ID_STRING_DELETE 5 /* Delete */
#define ID_STRING_MOVE_TO_TRASH 6 /* Move to Trash */
#define ID_STRING_PASTE 7 /* Paste */
#define ID_STRING_OPEN_WITH 8 /* Open With */
#define ID_STRING_CUT 9 /* Cut */
#define ID_STRING_MOVE 10 /* Move */
#define ID_STRING_PROP 11 /* Properties */
#define ID_STRING_LINK 12 /* Link */
#define ID_STRING_TRASH 13 /* Empty Trash Bin*/
#define ID_STRING_ADD_TO_BOOMARKS 14
#define ID_STRING_SAVE_URL_PROPS 15 /* sven */
#define ID_STRING_SHOW_MENUBAR 16 /* sven */
#define ID_STRING_UP 17 /* sven */
#define ID_STRING_BACK 18 /* sven */
#define ID_STRING_FORWARD 19 /* sven */

// browser/tree window color defaults -- Bernd
#define HTML_DEFAULT_BG_COLOR white
#define HTML_DEFAULT_LNK_COLOR red
#define HTML_DEFAULT_TXT_COLOR black
#define HTML_DEFAULT_VLNK_COLOR magenta

// root window grid spacing defaults -- Bernd
#define DEFAULT_GRID_WIDTH 70
#define DEFAULT_GRID_HEIGHT 70
#define DEFAULT_GRID_MAX 150
#define DEFAULT_GRID_MIN 50

// root window icon text transparency default -- stefan@space.twc.de
#define DEFAULT_ROOT_ICONS_STYLE 0
// show hidden files on desktop default
#define DEFAULT_SHOW_HIDDEN_ROOT_ICONS false
//CT root icons foreground/background defaults
#define DEFAULT_ICON_FG white
#define DEFAULT_ICON_BG black
//CT

// lets be modern .. -- Bernd
#define DEFAULT_VIEW_FONT "helvetica"
#define DEFAULT_VIEW_FIXED_FONT "courier"

// the default size of the kfm browswer windows
// these are optimized sizes displaying a maximum number
// of icons. -- Bernd
#define KFMGUI_HEIGHT 360
#define KFMGUI_WIDTH 540

// Default terminal for Open Terminal and for 'run in terminal'
#define DEFAULT_TERMINAL "konsole1"

// Default editor for "View Document/Frame Source"
#define DEFAULT_EDITOR "kedit"

// Default UserAgent string (e.g. Konqueror/1.1)
#define DEFAULT_USERAGENT_STRING QString("Konqueror/")+KDE_VERSION_STRING

