//-----------------------------------------------------------------------------
//
// KDE Help Viewer
//

#ifndef __HELPWIN_H__
#define __HELPWIN_H__

#include <unistd.h>

#include <qwidget.h>
#include <qdialog.h>
#include <qmenubar.h>
#include <qbttngrp.h>
#include <qstrlist.h>
#include <qlined.h>
#include <qscrbar.h>
#include <qlabel.h>
#include <qcursor.h>

#include <kfm.h>

#include "cgi.h"
#include "html.h"
#include "bookmark.h"
#include "misc.h"
#include "fmthtml.h"
#include "info.h"
#include "man.h"
#include "options.h"
#include "history.h"
#include "finddlg.h"

// accelerator IDs
#define NEW			100
#define CLOSE			101
#define QUIT			102
#define COPY			200

#define KDEHELP_VERSION		"0.7"

#define STATUSBAR_HEIGHT	20
#define SCROLLBAR_WIDTH		16
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

class KPageInfo
{
public:
	KPageInfo( const char *u, int y )
		{	url = u; yOffset = y; }

    KPageInfo( const KPageInfo &i )
        {   url = i.url.copy(); yOffset = i.yOffset; }

	const QString getUrl() const
		{	return url; }
	int getOffset() const
		{	return yOffset; }

	void setOffset( int y )
		{	yOffset = y; }

private:
	QString url;
	int yOffset;
};

//-----------------------------------------------------------------------------

class KHelpView : public KHTMLWidget
{
public:
    KHelpView( QWidget *parent = 0L, const char *name = 0L );
    virtual ~KHelpView();

    static KHistory *urlHistory;

protected:
    virtual bool URLVisited( const char *_url );
};

//-----------------------------------------------------------------------------

class KHelpWindow : public QWidget
{
	Q_OBJECT
public:
	// List of all for the HelpWindow that can currently be 
	// triggered off externally
	enum AllowedActions { Copy, GoBack, GoForward, GoPrevious,  GoNext,
			      GoUp, GoTop, Stop };

	KHelpWindow(QWidget *parent=NULL, const char *name=NULL);
	virtual ~KHelpWindow();

	int openURL( const char *URL, bool withHistory = true );

	bool canCurrentlyDo(AllowedActions action);
	const char *getCurrentURL();

    const cHistory<KPageInfo> &getHistory() const { return history; }
	void setHistory( const cHistory<KPageInfo> &hist )
        {   history = hist; }

signals:
	void enableMenuItems();
	void openNewWindow(const char *newURL);

	void setURL( const char *url);
	// the name of the url being displayed has changed to "url"
	
	void setLocation( const char *url);
	// This signal gets emitted when the URL in the LocationBar should be changed
     
	void bookmarkChanged(KBookmark *);

	void setTitle(const char *_title);

public slots:
	void	slotOpenFile();
	void	slotOpenURL();
	void	slotSearch();
	void	slotReload();
	void	slotPrint();

	void	slotCopy();
	void	slotFind();
	void	slotFindNext();
	void	slotFindNext( const QRegExp & );
	void	slotBack();
	void	slotForward();
	void	slotDir();
	void	slotTop();
	void	slotUp();
	void	slotPrev();
	void	slotNext();
	void	slotTextSelected( bool sel );
	void	slotAddBookmark();
	void	slotBookmarkSelected( int id );
	void	slotBookmarkHighlighted( int id );
	void	slotBookmarkChanged();
	void	slotStopProcessing();
	void	slotSetTitle( const char * );
	void	slotURLSelected( const char *, int );
	void	slotOnURL( const char * );
	void	slotFormSubmitted( const char *, const char *, const char * );
	void	slotPopupMenu( const char *, const QPoint & );
	void	slotDropEvent( KDNDDropZone * );
	void	slotCGIDone();
	void	slotScrollVert( int _y );
	void	slotScrollHorz( int _y );
	void	slotBackgroundColor( const QColor &col );
	void	slotFontSize( int );
	void	slotStandardFont( const char * );
	void	slotFixedFont( const char * );
	void	slotColorsChanged( const QColor&, const QColor&, const QColor&,
		    const QColor&, const bool, const bool );
	void	slotPopupOpenURL();
	void	slotPopupAddBookmark();
	void	slotPopupOpenNew();
	void	slotViewResized( const QSize & );
	void	slotDocumentChanged();
	void	slotDocumentDone();

protected:
	virtual void resizeEvent( QResizeEvent * );
	virtual bool eventFilter( QObject *, QEvent * );
	virtual bool x11Event( XEvent * );

private:
	enum FileType { UnknownFile, HTMLFile, InfoFile, ManFile, CannotOpenFile };

	void	readOptions();
	int	openFile( const QString & );
	int 	formatInfo( int bodyOnly = FALSE );
	int	formatMan( int bodyOnly = FALSE );
	int 	openHTML( const char *location );
	int	runCGI( const char *_url );
	FileType detectFileType( const QString &filename );
	bool	tryHtmlDefault( QString &filename ) const;
	void	convertSpecial( const char *buffer, QString &converted );
	void	enableToolbarButton( int id, bool enable );
	void	createMenu();
	QString	getPrefix();
	QString	getLocation();
	void	addBookmark( const char *_title, const char *url );
	void	layout();

private:
	QScrollBar *vert;
	QScrollBar *horz;
	QLabel *statusBar;
	QPopupMenu *rmbPopup;
//	QAccel *accel;
	KHelpView *view;
	KDNDDropZone *dropZone;
	KOpenURLDialog *openURLDialog;
	KFindTextDialog *findDialog;

	QString localFile;

	KCGI *CGIServer;

	static KHelpOptionsDialog *optionsDialog;
	static KBookmarkManager bookmarkManager;

	// html view preferences
	static int  fontBase;
	static QString standardFont;
	static QString fixedFont;

	static QColor bgColor;
	static QColor textColor;
	static QColor linkColor;
	static QColor vLinkColor;
	static bool   underlineLinks;
	static bool   forceDefaults;

	QString fullURL;
	QString currentURL;
	QString currentInfo;
	QString title;
	QString ref;

	// current width of the html view
	int viewWidth;

	// scroll to here when parsed
	int scrollTo;

	// busy parsing
	bool busy;

	QCursor oldCursor;

	cHistory<KPageInfo> history;
	cHTMLFormat html;
	cInfo *info;
	cMan *man;
	cHelpFormatBase *format;

	static QString newURL;
};

#endif	// __HELP_H__

