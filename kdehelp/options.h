//-----------------------------------------------------------------------------
//
// KDE Help Options
//
// (c) Martin R. Jones 1996
//

#ifndef __HELP_OPTIONS_H__
#define __HELP_OPTIONS_H__

#include <qtabdlg.h>
#include <qcombo.h>
#include <qstrlist.h>
#include <Kconfig.h>


//-----------------------------------------------------------------------------

class KFontOptions : public QWidget
{
	Q_OBJECT

public:
	KFontOptions( QWidget *parent = NULL, const char *name = NULL );

public slots:
	void	slotApplyPressed();
	void	slotFontSize( int );
	void	slotStandardFont( const char *n );
	void	slotFixedFont( const char *n );

signals:
	void	fontSize( int );
	void	standardFont( const char * );
	void	fixedFont( const char * );

private:
	void	readOptions();
	void	getFontList( QStrList &list, const char *pattern );
	void	addFont( QStrList &list, const char *xfont );

private:
	int		fSize;
	QString	stdName;
	QString	fixedName;
	QStrList standardFonts;
	QStrList fixedFonts;
};


//-----------------------------------------------------------------------------

class KHelpOptionsDialog : public QTabDialog
{
	Q_OBJECT

public:
	KHelpOptionsDialog( QWidget *parent = NULL, const char *name = NULL );

public:
	KFontOptions *fontOptions;
};


#endif		// __HELP_OPTIONS_H__

