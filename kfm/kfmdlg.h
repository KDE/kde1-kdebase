#ifndef KFMDLG_H
#define KFMDLG_H

#include <qdialog.h>
#include <qlined.h>
#include <qpushbt.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include "kURLcompletion.h"

class KApplicationTree;
class KMimeBind;

/**
 * @sort Asking for a single line of text
 * This class can be used to ask for a new filename or for
 * an URL.
 */
class DlgLineEntry : public QDialog
{
    Q_OBJECT
public:
    /**
     * Create a dialog that asks for a single line of text. _value is the initial
     * value of the line. _text appears as label on top of the entry box.
     * 
     * @param _file_mode if set to TRUE, the editor widget will provide command
     *                   completion ( Ctrl-S and Ctrl-D )
     */
    DlgLineEntry( const char *_text, const char *_value, QWidget *parent, bool _file_mode = FALSE );
    ~DlgLineEntry();

    /**
     * @return the value the user entered
     */
    const char * getText() { return edit->text(); }
    
public slots:
    /**
     * The slot for clearing the edit widget
     */
    void slotClear();

protected:
    /**
     * The line edit widget
     */
    QLineEdit *edit;

    /**
      * Completion helper ..
      */
    KURLCompletion * completion;
};

/**
 */
class OpenWithDlg : public QDialog
{
  Q_OBJECT
public:
  /**
   * Create a dialog that asks for a single line of text. _value is the initial
   * value of the line. _text appears as label on top of the entry box.
   * 
   * @param _file_mode if set to TRUE, the editor widget will provide command
   *                   completion ( Ctrl-S and Ctrl-D )
   */
  OpenWithDlg( const char *_text, const char *_value, QWidget *parent, bool _file_mode = FALSE );
  ~OpenWithDlg();
  
  /**
   * @return the value the user entered
   */
  const char * getText() { return edit->text(); }
  KMimeBind* mimeBind() { return m_pBind; }
  
public slots:
  /**
   * The slot for clearing the edit widget
   */
  void slotClear();
  void slotBrowse();
  void slotSelected( const char* _name, const char* _exec );
  void slotHighlighted( const char* _name, const char* _exec );
  void slotOK();

protected:

  void resizeEvent(QResizeEvent *);   
  
protected:
  /**
   * The line edit widget
   */
  QLineEdit *edit;
  
  /**
   * Completion helper ..
   */
  KURLCompletion * completion;

  KApplicationTree* m_pTree;
  QLabel *label;

  QString qExec;
  QString qName;
  bool  haveApp;
  QPushButton *ok;
  QPushButton *clear;
  QPushButton *cancel;
  QPushButton* browse;
  QCheckBox* terminal;

  KMimeBind *m_pBind;
};

#endif
