#include "kfmdlg.h"

DlgLineEntry::DlgLineEntry( const char *_text, const char* _value, QWidget *parent )
        : QDialog( parent, 0L, TRUE )
{
    setGeometry( x(), y(), 200, 110 );

    QLabel *label = new QLabel( _text , this );
    label->setGeometry( 10, 10, 180, 15 );
    
    edit = new QLineEdit( this, 0L );
    edit->setGeometry( 10, 40, 180, 20 );
    connect( edit, SIGNAL(returnPressed()), SLOT(accept()) );

    QPushButton *ok;
    QPushButton *cancel;
    ok = new QPushButton( "Ok", this );
    ok->setGeometry( 10,70, 50,30 );
    connect( ok, SIGNAL(clicked()), SLOT(accept()) );

    cancel = new QPushButton( "Cancel", this );
    cancel->setGeometry( 140, 70, 50, 30 );
    connect( cancel, SIGNAL(clicked()), SLOT(reject()) );

    edit->setText( _value );
    edit->setFocus();
}

#include "kfmdlg.moc"
