#ifndef KFMDLG_H
#define KFMDLG_H

#include <qdialog.h>
#include <qlined.h>
#include <qpushbt.h>
#include <qlabel.h>

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

    /**
     * @return the value the user entered
     */
    const char * getText() { return edit->text(); }
    
protected:
    /**
     * The line edit widget
     */
    QLineEdit *edit;
};

#endif
