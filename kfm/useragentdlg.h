/*
 * $Id$
 */

#ifndef _USERAGENTDLG_H
#define _USERAGENTDLG_H

#include <qdialog.h>
#include <qstring.h>

class QLabel;
class QLineEdit;
class QListBox;
class QPushButton;

class UserAgentDialog : public QDialog
{
  Q_OBJECT

public:
  UserAgentDialog ( QWidget * parent=0, const char * name=0, 
							  WFlags f=0 ) ;
  ~UserAgentDialog();

  void setData( QStrList* strlist );
  QStrList data() const;

private slots:
  void helpClicked();
  void textChanged( const char* );
  void returnPressed();
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

/*
 * $Log$
 * Revision 1.1  1997/11/21 08:07:34  kalle
 * Make ourselves known via the UserAgent line.
 *
 */
