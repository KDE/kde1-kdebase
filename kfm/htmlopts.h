//-----------------------------------------------------------------------------
//
// KDE Help Options
//
// (c) Martin R. Jones 1996
//

#ifndef __HTML_OPTIONS_H__
#define __HTML_OPTIONS_H__

#include <qtabdlg.h>
#include <qcombo.h>
#include <qstrlist.h>
#include <qchkbox.h>
#include <kspinbox.h>
#include <kconfig.h>


struct fontoptions{

  int     fontsize;
  QString standardfont;
  QString fixedfont;
  bool    changed;

};

struct rootoptions{

  int     gridwidth;
  int     gridheight;
  int     iconstyle;
  bool    changed;

};


struct coloroptions{

  QColor bg;
  QColor text;
  QColor link;
  QColor vlink;
  bool   changed;
  bool   changeCursoroverLink;
};

//-----------------------------------------------------------------------------

class KFontOptions : public QWidget
{
	Q_OBJECT

public:

	KFontOptions( QWidget *parent = NULL, const char *name = NULL );
	void getFontOpts(struct fontoptions& fntopts);

public slots:
	void	slotFontSize( int );
	void	slotStandardFont( const char *n );
	void	slotFixedFont( const char *n );

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


	struct fontoptions  fontopts;
};

//-----------------------------------------------------------------------------

class KColorOptions : public QWidget
{
	Q_OBJECT
public:
	KColorOptions( QWidget *parent = NULL, const char *name = NULL );
	void getColorOpts(struct coloroptions& coloropts);


protected slots:
	void	slotBgColorChanged( const QColor &col );
	void	slotTextColorChanged( const QColor &col );
	void	slotLinkColorChanged( const QColor &col );
	void	slotVLinkColorChanged( const QColor &col );

private:
	void	readOptions();

private:
	QCheckBox *cursorbox;
	QColor bgColor;
	QColor textColor;
	QColor linkColor;
	QColor vLinkColor;
	bool   changeCursor;
	bool   changed;
	struct coloroptions coloropts;
};

//-----------------------------------------------------------------------------

class KMiscOptions : public QWidget
{
	Q_OBJECT
public:
	KMiscOptions( QWidget *parent = NULL, const char *name = NULL );
	void getMiscOpts(struct rootoptions& rootopts);

private:
	void	readOptions();

private:
	QCheckBox *cursorbox,*iconstylebox;
	KNumericSpinBox *hspin;
	KNumericSpinBox *vspin;
	int    gridwidth;
    int    gridheight;
    bool   transparent;
	bool   changeCursor;
	bool   changed;
	struct rootoptions rootopts;
};




#endif		// __HTML_OPTIONS_H__

