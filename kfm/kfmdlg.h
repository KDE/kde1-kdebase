#ifndef KFMDLG_H
#define KFMDLG_H

#include <qdialog.h>
#include <qlined.h>
#include <qpushbt.h>
#include <qlabel.h>

/// Asking for a single line of text
/**
  This class can be used to ask for a new filename or for
  an URL.
  */
class DlgLineEntry : public QDialog
{
    Q_OBJECT
public:
    /// Constructor
    /**
      Create a dialog that asks for a single line of text. _value is the initial
      value of the line. _text appears as label on top of the entry box.
      */
    DlgLineEntry( const char *_text, const char *_value, QWidget *parent );

    /// Return the value the user entered
    const char * getText() { return edit->text(); }
    
protected:
    /// The line edit widget
    QLineEdit *edit;
};

#endif
