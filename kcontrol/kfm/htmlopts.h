//-----------------------------------------------------------------------------
//
// KDE Help Options
//
// (c) Martin R. Jones 1996
//
// Port to KControl
// (c) Torben Weis 1998

#ifndef __HTML_OPTIONS_H__
#define __HTML_OPTIONS_H__

#include <qtabdlg.h>
#include <qcombo.h>
#include <qstrlist.h>
#include <qchkbox.h>
#include <kspinbox.h>
#include <kconfig.h>
#include <kcolorbtn.h>
#include <qradiobt.h>
#include <qcombo.h>

#include <kcontrol.h>
#include <kwm.h>

extern KConfigBase *g_pConfig;;

//-----------------------------------------------------------------------------

class KFontOptions : public KConfigWidget
{
  Q_OBJECT
public:
  KFontOptions( QWidget *parent = 0L, const char *name = 0L );

  virtual void loadSettings();
  virtual void saveSettings();
  virtual void applySettings();
  virtual void defaultSettings();

public slots:
  void slotFontSize( int );
  void slotStandardFont( const char *n );
  void slotFixedFont( const char *n );

private:
  void getFontList( QStrList &list, const char *pattern );
  void addFont( QStrList &list, const char *xfont );

private:
  QComboBox* m_pFixed;
  QComboBox* m_pStandard;
  QRadioButton* m_pSmall;
  QRadioButton* m_pMedium;
  QRadioButton* m_pLarge;
  
  int fSize;
  QString stdName;
  QString fixedName;
  QStrList standardFonts;
  QStrList fixedFonts;
};

//-----------------------------------------------------------------------------

class KColorOptions : public KConfigWidget
{
  Q_OBJECT
public:
  KColorOptions( QWidget *parent = NULL, const char *name = NULL );

  virtual void loadSettings();
  virtual void saveSettings();
  virtual void applySettings();
  virtual void defaultSettings();
  
protected slots:
  void slotBgColorChanged( const QColor &col );
  void slotTextColorChanged( const QColor &col );
  void slotLinkColorChanged( const QColor &col );
  void slotVLinkColorChanged( const QColor &col );

private:
  KColorButton* m_pBg;
  KColorButton* m_pText;
  KColorButton* m_pLink;
  KColorButton* m_pVLink;
  QCheckBox *cursorbox;
  QColor bgColor;
  QColor textColor;
  QColor linkColor;
  QColor vLinkColor;
  bool changeCursor;
  bool changed;
};

#endif		// __HTML_OPTIONS_H__

