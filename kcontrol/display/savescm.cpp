#include <stdlib.h>
#include <stdio.h>

#include "savescm.h"
#include "savescm.moc"

#include <qaccel.h>
#include <kapp.h>
#include <qframe.h>
#include <qlabel.h>

SaveScm::SaveScm( QWidget *parent, const char *name )
	: QDialog( parent, name, TRUE )
{
	setFocusPolicy(QWidget::StrongFocus);
	
	QFrame* tmpQFrame;
	tmpQFrame = new QFrame( this );
	tmpQFrame->setGeometry( 5, 5, 250, 65 );
	tmpQFrame->setFrameStyle( 35 );
	tmpQFrame->setLineWidth( 2 );

	QLabel* tmpQLabel;
	tmpQLabel = new QLabel( this );
	tmpQLabel->setGeometry( 15, 25, 35, 25 );
	tmpQLabel->setText( klocale->translate("Name") );
	tmpQLabel->setAlignment( 290 );

	nameLine = new QLineEdit( this );
	nameLine->setGeometry( 60, 25, 170, 25 );
	nameLine->setMaxLength(18);
	
	ok = new QPushButton( this );
	ok->setGeometry( 115, 80, 65, 30 );
	ok->setText( klocale->translate("OK") );
	ok->setAutoDefault(TRUE);
	connect( ok, SIGNAL(clicked()), SLOT(accept()) );

	cancel = new QPushButton( this );
	cancel->setGeometry( 190, 80, 60, 30 );
	cancel->setText( klocale->translate("Cancel") );
	cancel->setAutoDefault(TRUE);
	connect( cancel, SIGNAL(clicked()), SLOT(reject()) );

	resize( 260, 115 );
	setMaximumSize( 260, 115 );
	setMinimumSize( 260, 115 );
}
