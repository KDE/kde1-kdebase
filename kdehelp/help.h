//-----------------------------------------------------------------------------
//
// KDE Help Viewer
//

#ifndef __HELP_H__
#define __HELP_H__

#include <unistd.h>

#include <qwidget.h>
#include <qdialog.h>
#include <qmenubar.h>
#include <qbttngrp.h>
#include <qstrlist.h>
#include <qlined.h>
#include <qscrbar.h>
#include <qlabel.h>

#include <kfm.h>

#include "cgi.h"
#include "html.h"
#include "bookmark.h"
#include "misc.h"
#include "fmthtml.h"
#include "info.h"
#include "man.h"
#include "options.h"

// accelerator IDs
#define NEW				100
#define CLOSE			101
#define QUIT			102
#define COPY			200

#define KDEHELP_VERSION		"0.4.9"

#define STATUSBAR_HEIGHT	20
#define BUTTON_HEIGHT		26
#define BUTTON_WIDTH		26
#define BUTTON_SEPARATION	6
#define BOOKMARK_ID_BASE	200

//-----------------------------------------------------------------------------

class KOpenURLDialog : public QDialog
{
	Q_OBJECT
public:
	KOpenURLDialog( QWidget *parent = NULL, const char *name = NULL );

signals:
	void openURL( const char *URL, int );

public slots:
	void openPressed();

private:
	QLineEdit *lineEdit;
};

//-----------------------------------------------------------------------------

class KHelpView : public KHTMLWidget
{
	Q_OBJECT
public:
	KHelpView(QWidget *parent, const char *name, const char *_pix_path );
};

//-----------------------------------------------------------------------------

class KHelpWindow : public QWidget
{
	Q_OBJECT
public:
	KHelpWindow(QWidget *parent=NULL, const char *name=NULL);
	virtual ~KHelpWindow();

	int 	openURL( const char *URL, int withHistory = TRUE );
	void	openNewWindow( const char *url );

public slots:
	void	slotNewWindow();
	void	slotOpenFile();
	void	slotOpenURL();
	void	slotSearch();
	void	slotClose();
	void	slotCopy();
	void	slotBack();
	void	slotForward();
	void	slotDir();
	void	slotTop();
	void	slotUp();
	void	slotPrev();
	void	slotNext();
	void	slotAddBookmark();
	void	slotBookmarkSelected( int id );
	void	slotBookmarkHighlighted( int id );
	void	slotBookmarkChanged();
	void	slotStopProcessing();
	void	slotOptionsGeneral();
	void	slotUsingHelp();
	void	slotAbout();
	void	slotSetTitle( const char * );
	void	slotURLSelected( const char *, int );
	void	slotOnURL( const char * );
	void	slotFormSubmitted( const char *, const char * );
	void	slotPopupMenu( const char *, const QPoint & );
	void	slotDropEvent( KDNDDropZone * );
//	void	slotImageRequest( const char * );
	void	slotRemoteDone();
	void	slotCGIDone();
	void	slotScrollVert( int _y );
	void	slotScrollHorz( int _y );
	void	slotScrollUp();
	void	slotScrollDown();
	void	slotScrollLeft();
	void	slotScrollRight();
	void	slotScrollPageUp();
	void	slotScrollPageDown();
	void	slotBackgroundColor( const QColor &col );
	void	slotFontSize( int );
	void	slotStandardFont( const char * );
	void	slotFixedFont( const char * );
	void	slotPopupOpenURL();
	void	slotPopupAddBookmark();
	void	slotPopupOpenNew();
	void	slotDocumentChanged();
	void	slotDocumentDone();

protected:
	void	resizeEvent(QResizeEvent *);
	void	closeEvent( QCloseEvent * );

private:
	enum FileType { UnknownFile, HTMLFile, InfoFile, ManFile };

	void	readOptions();
	int		openFile( const QString & );
	int 	formatInfo( int bodyOnly = FALSE );
	int		formatMan( int bodyOnly = FALSE );
	int 	openHTML( const char *location );
	int		openRemote( const char *_url );
	int		runCGI( const char *_url );
	FileType detectFileType( const QString &filename );
	void	convertSpecial( const char *buffer, QString &converted );
	void	enableMenuItems();
	void	enableToolbarButton( int id, bool enable );
	void	createToolbar();
	void	fillBookmarkMenu( KBookmark *parent, QPopupMenu *menu, int &id );
	QString	getPrefix();
	QString	getLocation();
	void	addBookmark( const char *_title, const char *url );

private:
	QMenuBar *menu;
	QButtonGroup *toolbar;
	QPopupMenu *fileMenu;
	QPopupMenu *gotoMenu;
	QPopupMenu *bookmarkMenu;
	QScrollBar *vert;
	QScrollBar *horz;
	QLabel *statusBar;
	QAccel *accel;
	KHelpView *view;
	KDNDDropZone *dropZone;
	KOpenURLDialog *openURLDialog;

	KFM *remotePage;
//	QList<KFM> remoteImage;
	QString remoteFile;
	QString localFile;

	KCGI *CGIServer;

	static KHelpOptionsDialog *optionsDialog;
	static KBookmarkManager bookmarkManager;

	static int  fontBase;
	static QString standardFont;
	static QString fixedFont;

	KPixmap toolbarPixmaps[16];
	QString fullURL;
	QString currentURL;
	QString currentInfo;
	QString title;
	QString ref;

	int topOffset;
	int bottomOffset;

	int idClose;
	int idCopy;
	int idBack;
	int idForward;
	int idDir;
	int idTop;
	int idUp;
	int idPrev;
	int idNext;

	bool busy;

	cHistory<QString> history;
	cHTMLFormat html;
	cInfo *info;
	cMan *man;
	cHelpFormatBase *format;

	static QString newURL;
};

#endif	// __HELP_H__

