#include <stdlib.h>
#include <stdio.h>

#include "savescm.h"
#include "savescm.moc"

#include <qaccel.h>
#include <kapp.h>
#include <kbuttonbox.h>
#include <qlayout.h>
#include <qframe.h>
#include <qlabel.h>

SaveScm::SaveScm( QWidget *parent, const char *name )
	: QDialog( parent, name, TRUE )
{
	setFocusPolicy(QWidget::StrongFocus);
	
	QBoxLayout *topLayout = new QVBoxLayout( this, 10 );

	QBoxLayout *stackLayout = new QVBoxLayout( 3 );
	topLayout->addLayout( stackLayout );

	nameLine = new QLineEdit( this );
	nameLine->setMaxLength(18);
	nameLine->setFixedHeight( nameLine->sizeHint().height() );
	
	QLabel* tmpQLabel;
	tmpQLabel = new QLabel( nameLine, 
			i18n( "&Enter the name of a new\n"\
					"color scheme to be added to the list." ), this );
	tmpQLabel->setAlignment( AlignLeft | AlignBottom | ShowPrefix );
	tmpQLabel->setFixedHeight( tmpQLabel->sizeHint().height() );
	tmpQLabel->setMinimumWidth( tmpQLabel->sizeHint().width() );
	
	stackLayout->addStretch( 10 );
	stackLayout->addWidget( tmpQLabel );
	stackLayout->addWidget( nameLine );
	
	QFrame* tmpQFrame;
	tmpQFrame = new QFrame( this );
	tmpQFrame->setFrameStyle( QFrame::HLine | QFrame::Sunken );
	tmpQFrame->setMinimumHeight( tmpQFrame->sizeHint().height() );
	
	topLayout->addWidget( tmpQFrame );
	
	KButtonBox *bbox = new KButtonBox( this );
	bbox->addStretch( 10 );
	
	QPushButton *ok = bbox->addButton( i18n( "&OK" ) );
	connect( ok, SIGNAL( clicked() ), SLOT( accept() ) );
	
	QPushButton *cancel = bbox->addButton( i18n( "&Cancel" ) );
	connect( cancel, SIGNAL( clicked() ), SLOT( reject() ) );
	
	bbox->layout();
	topLayout->addWidget( bbox );

    topLayout->activate();
	
	resize( 250, 0 );
}
