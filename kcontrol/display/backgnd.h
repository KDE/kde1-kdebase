//-----------------------------------------------------------------------------
//
// KDE Display background setup module
//
// Copyright (c)  Martin R. Jones 1996
//

#ifndef __BACKGND_H__
#define __BACKGND_H__

#include <qwidget.h>
#include <qpushbt.h>
#include <qcombo.h>
#include <qlistbox.h>
#include <qradiobt.h>
#include <qlined.h>
#include <qbttngrp.h>

#include <kintegerline.h>
#include <kcolordlg.h>
#include <kcolorbtn.h>
#include <kcontrol.h>
#include <kslider.h>
#include <drag.h>


#include "display.h"

class KBackground;
class PatternEntry;
class QLCDNumber;

class KItem : public QObject
{
  
public:

  KItem() {}

  KItem ( const KItem &itm )	// deep copy
    {
      color1 = itm.color1;
      color2 = itm.color2;
      wpMode = itm.wpMode;
      ncMode = itm.ncMode;
      stMode = itm.stMode;
      orMode = itm.orMode;
      bUseWallpaper = itm.bUseWallpaper;
      wallpaper = itm.wallpaper.data();

      for ( int i = 0; i < 8; i++ ){
	pattern[i] = itm.pattern[i];
      }
    }

  KItem &operator=( const KItem &itm )	// deep copy
    {
      color1 = itm.color1;
      color2 = itm.color2;
      wpMode = itm.wpMode;
      ncMode = itm.ncMode;
      stMode = itm.stMode;
      orMode = itm.orMode;
      bUseWallpaper = itm.bUseWallpaper;
      wallpaper = itm.wallpaper.data();
      
      for ( int i = 0; i < 8; i++ ){
	pattern[i] = itm.pattern[i];
      }

      return *this;
    }

  QColor color1;
  QColor color2;
  int wpMode;
  int ncMode;
  int stMode; 
  int orMode;
  uint pattern[8];

  bool bUseWallpaper;
  QString wallpaper;
};


class KBGMonitor : public QWidget
{
  Q_OBJECT
public:
  KBGMonitor( QWidget *parent ) : QWidget( parent ) {};

  // we don't want no steenking palette change
  virtual void setPalette( const QPalette & ) {};
};

class KRenameDeskDlg : public QDialog
{
  Q_OBJECT
public:
  KRenameDeskDlg( const char *t, QWidget *parent );
  virtual ~KRenameDeskDlg() {}

  const char *title()
    {  return edit->text(); }

private:
  QLineEdit *edit;
};

class PatternEntry 
{
public: 
  PatternEntry() {}
  PatternEntry( QString n, uint p[]) { 
    name = n;
    for (uint i = 0; i < 8; i++)
      pattern[i] = p[i];
  }

  QString name;
  uint pattern[8];

  bool operator==( uint p[] ) {
    for (uint i = 0; i < 8; i++)
      if (pattern[i] != p[i]) 
	return false;
    return true;
  }

  bool operator==( const char *item) {
    return name == item;
  }
	
};

class KBPatternDlg : public QDialog
{
  Q_OBJECT

public:
  KBPatternDlg( QColor color1, QColor color2, uint p[], int *orient,
		int *type, QWidget *parent = 0, char *name = 0 );
    
protected:
  int savePatterns();
   
protected slots:
  void selected(const char *item);
  virtual void done ( int r );
  void slotMode( int );
    
private:
  QLabel *preview;
  QLabel *lPreview;
  QLabel *lName;
  QListBox *listBox;
  QList<PatternEntry> list;
  QCheckBox *orientCB;
  QRadioButton *rbVert;
  QRadioButton *rbHoriz;
  QRadioButton *rbPattern;
  QButtonGroup *suGroup;
	
  enum { Portrait = 1, Landscape, Pattern };
	

  bool changed;
  PatternEntry *current;
  uint *pattern;
  int *orMode;
  int *tpMode;
  int mode;
    
  QColor color1, color2;
};


class KRandomDlg : public QDialog
{
  Q_OBJECT

public:
  KRandomDlg(int _desktop, KBackground *_kb, char *name = 0 );
  ~KRandomDlg() {}

  friend class KBackground;

protected:
  void addToPicList( QString pic );
  void readSettings();
  void copyCurrent();

protected slots:
  void selected( int index );
  void slotDelete();
  void slotAdd();
  void picDropped(KDNDDropZone *zone);
  void changeDir();
  void slotBrowse ();

  virtual void done ( int r );

private:
  QLabel *desktopLabel;
  QLabel *timerLabel;
  QListBox *listBox;
  QList<QString> list;
  QCheckBox *orderButton;

  QCheckBox *dirCheckBox;
  QLineEdit *dirLined;
  QPushButton *dirPushButton;

  KIntegerLine *timerLined;
  KDNDDropZone *picdrop;

  KBackground *kb;
  QList<KItem> ItemList;

  bool useDir;
  QString picDir;

  int count;
  int delay;
  bool inorder;

  bool changed;
  int desktop;
  int item;
};


class KBackground : public KDisplayModule
{
  Q_OBJECT
public:
	
  enum { Portrait = 1, Landscape };
  enum { Flat = 1, Gradient, Pattern };

  KBackground( QWidget *parent, int mode, int desktop = 0 );

  virtual void readSettings( int deskNum = 0 );
  virtual void apply( bool force = FALSE );
  virtual void loadSettings();
  virtual void applySettings();
  virtual void defaultSettings();

  friend class KItem;
  friend class KRandomDlg;

protected slots:

  void slotApply();
  void slotSelectColor1( const QColor &col );
  void slotSelectColor2( const QColor &col );
  void slotBrowse();
  void slotWallpaper( const char * );
  void slotWallpaperMode( int );
  void slotColorMode( int );
  void slotStyleMode( int );
  void slotSwitchDesk( int );
  void slotRenameDesk();
  void slotHelp();
  void slotSetup2Color();
  void slotSetupRandom();
  void slotToggleRandom();
  void slotToggleOneDesktop();
  void slotToggleDock();
  void slotDropped(KDNDDropZone *zone);

  void resizeEvent( QResizeEvent * );

protected:
  void getDeskNameList();
  void setDesktop( int );
  void showSettings();
  void writeSettings( int deskNum );
  void setMonitor();
  bool loadWallpaper( const char *filename, bool useContext = TRUE );
  void retainResources();
  void setDefaults();

  bool setNew( QString pic, int item );

protected:
  enum { Tiled = 1,
	 Mirrored,
	 CenterTiled,
	 Centred,
	 CentredBrick,
	 CentredWarp,
	 CentredMaxpect,
	 SymmetricalTiled,
	 SymmetricalMirrored,
	 Scaled };

  enum { OneColor = 1, TwoColor };

  QListBox     *deskListBox;

  QCheckBox *oneDesktopButton;
  QCheckBox *dockButton;
  QCheckBox *randomButton;

  QRadioButton *rbPattern;
  QRadioButton *rbGradient;

  KColorButton *colButton1;
  KColorButton *colButton2;

  QButtonGroup *ncGroup;

  QPushButton  *renameButton;
  QPushButton  *changeButton;
  QPushButton  *randomSetupButton;
  QPushButton  *browseButton;

  QComboBox *wpModeCombo;
  QComboBox *wpCombo;

  QSlider *cacheSlider;
  QLCDNumber *cacheLCD;

  QLabel *monitorLabel;
  KBGMonitor* monitor;
  KDNDDropZone *monitorDrop;

  QPixmap wpPixmap;
  QString deskName;
  QStrList deskNames;

  int deskNum;
  int random;
  int maxDesks;

  KItem currentItem;
  KRandomDlg *rnddlg;

  bool changed;

  bool randomMode;
  bool oneDesktopMode;
  bool interactive;
  bool docking;

};


#endif
