#ifndef _KMSGWIN_H_
#define _KMSGWIN_H_

#include <qwidget.h>
#include <qpushbt.h>
#include <qlabel.h>
#include <qframe.h>
#include <qpixmap.h>


/// KMsgWin
/** A message box API for the KDE project. KMsgWin provides a Windows - lookalike message- or
 error box with icons and up to 4 configurable buttons.
 This one uses QWidget instead of QDialog due to a bug in Qt.
*/

class KMsgWin : public QWidget
{
    Q_OBJECT;
public:

    enum {INFORMATION = 1, EXCLAMATION = 2, STOP = 4, QUESTION = 8};
    enum {DB_FIRST = 16, DB_SECOND = 32, DB_THIRD = 64, DB_FOURTH = 128};
    
    /// Constructor
    /** The generic constructor for a KMsgWin Widget. All parameters have a default value of 0.
     @Doc:
     \begin{itemize}
     \item { \tt The parent widget }
     \item { \tt A caption (title) for the message box. }
     \item { \tt a message string. The default alignment is centered. The string may contain
     newline characters.}
     \item { \tt Flags: This parameter is responsible for several behaviour options of the
     message box. See below for valid constans for this parameter.}
     \item { \tt Up to 4 button strings (b1text to b4text). You have to specify at least b1text.
     Unspecified buttons will not appear in the message box, therefore you can control the
     number of buttons in the box.}
     \end{itemize}
     */
    KMsgWin(QWidget *parent = 0, const char *caption = 0, const char *message = 0, int flags = INFORMATION,
            const char *b1text = 0, const char *b2text = 0, const char *b3text = 0, const char *b4text = 0);

protected:
    enum {B_SPACING = 10, B_WIDTH = 80};
    QLabel      *msg, *picture;
    QPushButton *b1, *b2, *b3, *b4;
    QFrame      *f1;
    int nr_buttons;
    int         w, h, h1, text_offset;
    void calcOptimalSize();

    virtual void resizeEvent( QResizeEvent * );

    void        initMe(const char *caption, const char *message, const char *b1text,
                       const char *b2text, const char *b3text, const char *b4text,
                       const QPixmap & icon = 0);

public slots:
    void b1Pressed();
    void b2Pressed();
    void b3Pressed();
    void b4Pressed();

signals:
    void result( QWidget*, int );
};

#endif

