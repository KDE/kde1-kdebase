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

// lets be modern .. -- Bernd
#define DEFAULT_VIEW_FONT "helvetica"
#define DEFAULT_VIEW_FIXED_FONT "courier"

// the default size of the kfm browswer windows
// these are optimized sizes displaying a maximum number
// of icons. -- Bernd
#define KFMGUI_HEIGHT 360
#define KFMGUI_WIDTH 540

