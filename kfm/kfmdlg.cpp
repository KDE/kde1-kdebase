#include "kfmdlg.h"
#include "fileentry.h"

DlgLineEntry::DlgLineEntry( const char *_text, const char* _value, QWidget *parent, bool _file_mode )
        : QDialog( parent, 0L, true )
{
    setGeometry( x(), y(), 350, 110 );

    QLabel *label = new QLabel( _text , this );
    label->setGeometry( 10, 10, 330, 15 );
    
    if ( _file_mode )
        edit = new KFileEntry( this, 0L );
    else
        edit = new QLineEdit( this, 0L );
    edit->setGeometry( 10, 40, 330, 25 );
    connect( edit, SIGNAL(returnPressed()), SLOT(accept()) );

    QPushButton *ok;
    QPushButton *clear;
    QPushButton *cancel;
    ok = new QPushButton( "Ok", this );
    ok->setGeometry( 10,70, 60,30 );
    connect( ok, SIGNAL(clicked()), SLOT(accept()) );

    clear = new QPushButton( "Clear", this );
    clear->setGeometry( 145, 70, 60, 30 );
    connect( clear, SIGNAL(clicked()), SLOT(slotClear()) );

    cancel = new QPushButton( "Cancel", this );
    cancel->setGeometry( 280, 70, 60, 30 );
    connect( cancel, SIGNAL(clicked()), SLOT(reject()) );

    edit->setText( _value );
    edit->setFocus();
}

void DlgLineEntry::slotClear()
{
    edit->setText("");
}

#include "kfmdlg.moc"
