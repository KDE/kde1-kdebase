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
#include <kconfig.h>


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

class KColorOptions : public QWidget
{
	Q_OBJECT
public:
	KColorOptions( QWidget *parent = NULL, const char *name = NULL );

signals:
	void	colorsChanged( const QColor &bg, const QColor &text,
			    const QColor &link, const QColor &vlink );

protected slots:
	void	slotApplyPressed();
	void	slotBgColorChanged( const QColor &col );
	void	slotTextColorChanged( const QColor &col );
	void	slotLinkColorChanged( const QColor &col );
	void	slotVLinkColorChanged( const QColor &col );

private:
	void	readOptions();

private:
	QColor bgColor;
	QColor textColor;
	QColor linkColor;
	QColor vLinkColor;
	bool   changed;
};

//-----------------------------------------------------------------------------

class KHelpOptionsDialog : public QTabDialog
{
	Q_OBJECT
public:
	KHelpOptionsDialog( QWidget *parent = NULL, const char *name = NULL );

public:
	KFontOptions *fontOptions;
	KColorOptions *colorOptions;
};


#endif		// __HELP_OPTIONS_H__

