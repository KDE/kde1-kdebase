//-----------------------------------------------------------------------------
//
// UserAgent Options
// (c) Kalle Dalheimer 1997
//
// Port to KControl
// (c) David Faure <faure@kde.org> 1998

#ifndef _USERAGENTDLG_H
#define _USERAGENTDLG_H

#include <qdialog.h>
#include <qstring.h>
#include <qwidget.h>
#include <qlined.h>
#include <qlabel.h>
#include <qpushbt.h>
#include <qlistbox.h>

#include <kcontrol.h>

extern KConfigBase *g_pConfig;

class UserAgentOptions : public KConfigWidget
{
  Q_OBJECT

public:
  UserAgentOptions ( QWidget * parent = 0L, const char * name = 0L) ;
  ~UserAgentOptions();

  virtual void loadSettings();
  virtual void saveSettings();
  virtual void applySettings();
  virtual void defaultSettings();
  
private slots:
  void textChanged( const char* );
//  void returnPressed();
  void addClicked();
  void deleteClicked();
  void listboxHighlighted( const char* );
  
private:
  QLabel* onserverLA;
  QLineEdit* onserverED;
  QLabel* loginasLA;
  QLineEdit* loginasED;

  QPushButton* addPB;
  QPushButton* deletePB;
  
  QLabel* bindingsLA;
  QListBox* bindingsLB;

  QPushButton* okPB;
  QPushButton* cancelPB;
  QPushButton* helpPB;

  int highlighted_item;
};


#endif
