#include "krenamewin.moc"
#include <stdio.h>
#include <config-kfm.h>

#include <klocale.h>
#include <kapp.h>
#include <qmsgbox.h>

KRenameWin::KRenameWin(QWidget *parent, const char *_src, const char *_dest, bool _modal ) :
    QDialog ( parent, "" , _modal )
{
    modal = _modal;
    
    src = _src;
    dest = _dest;
    
    setCaption(klocale->translate("KFM Warning"));

    b4 = new QPushButton( klocale->translate("Cancel"), this, "Cancel");
    b4->setGeometry( 10, 170, 90, 30 );
    connect(b4, SIGNAL(clicked()), this, SLOT(b4Pressed()));

    b3 = new QPushButton( klocale->translate("Rename"), 
			  this, "Rename");
    b3->setGeometry( 110, 170, 90, 30 );
    connect(b3, SIGNAL(clicked()), this, SLOT(b3Pressed()));
    
    b2 = new QPushButton( klocale->translate("Skip"), 
			  this, "Skip");
    b2->setGeometry( 210, 170, 90, 30 );
    connect(b2, SIGNAL(clicked()), this, SLOT(b2Pressed()));

    b0 = new QPushButton( klocale->translate("Overwrite"),
			  this, "Overwrite");
    b0->setGeometry( 310, 170, 90, 30 );
    connect(b0, SIGNAL(clicked()), this, SLOT(b0Pressed()));

    b1 = new QPushButton( klocale->translate("Overwrite All"), 
			  this, "Overwrite All");
    b1->setGeometry( 410, 170, 90, 30 );
    connect(b1, SIGNAL(clicked()), this, SLOT(b1Pressed()));
        
    QLabel *lb = new QLabel( src.data(), this );
    lb->setGeometry( 10, 10, 350, 20 );

    lb = new QLabel( klocale->translate("already exists. Overwrite with"), 
		     this );
    lb->setGeometry( 10, 40, 350, 20 );

    QString str;
    str.sprintf( "%s ?", dest.data() );
    lb = new QLabel( str.data(), this );
    lb->setGeometry( 10, 70, 350, 20 );
    
    lb = new QLabel( klocale->translate("Or rename to"), this );
    lb->setGeometry( 10, 110, 350, 20 );

    lineEdit = new QLineEdit( this );
    lineEdit->setText( dest.data() );
    lineEdit->setGeometry( 10, 130, 300, 30 );
    
    b4->setDefault( true );
    
    setGeometry( x(), y(), 510, 210 );
}

KRenameWin::~KRenameWin()
{
}

void KRenameWin::b0Pressed()
{
    if ( modal )
	done( 0 );
    else
	emit result( this, 0, src.data(), dest.data() );
}

void KRenameWin::b1Pressed()
{
    if ( modal )
	done( 1 );
    else
	emit result( this, 1, src.data(), dest.data() );
}

void KRenameWin::b2Pressed()
{
    if ( modal )
	done( 2 );
    else
	emit result( this, 2, src.data(), dest.data() );
}

// Rename
void KRenameWin::b3Pressed()
{
    if ( strcmp( "", lineEdit->text() ) == 0 )
	return;

    if ( dest == lineEdit->text() )
    {
	QMessageBox::warning( this, klocale->translate( "KFM Error" ),
			      klocale->translate( "You did not change the name!\n" ) );
	return;
    }
    
    if ( modal )
	done( 3 );
    else
	emit result( this, 3, src.data(), lineEdit->text() );
}

void KRenameWin::b4Pressed()
{
    if ( modal )
	done( 4 );
    else
	emit result( this, 4, src.data(), dest.data() );
}


