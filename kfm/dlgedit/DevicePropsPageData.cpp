
#include "DevicePropsPageData.h"


#include <qcombo.h>
#include <qlabel.h>
DevicePropsPageData::DevicePropsPageData
(
	QWidget* parent
)
{
	QLabel* tmpQLabel;
	tmpQLabel = new QLabel( parent, "Label_1" );
	tmpQLabel->setGeometry( 10, 10, 140, 30 );
	tmpQLabel->setText( "Device ( /dev/fd0 )" );

	device = new QLineEdit( parent, "LineEdit_1" );
	device->setGeometry( 10, 40, 180, 30 );
	device->setText( "" );

	tmpQLabel = new QLabel( parent, "Label_2" );
	tmpQLabel->setGeometry( 10, 80, 170, 30 );
	tmpQLabel->setText( "Mount Point ( /floppy )" );

	mountpoint = new QLineEdit( parent, "LineEdit_2" );
	mountpoint->setGeometry( 10, 110, 180, 30 );
	mountpoint->setText( "" );

	readonly = new QCheckBox( parent, "CheckBox_1" );
	readonly->setGeometry( 220, 40, 100, 30 );
	readonly->setText( "Readonly" );

	tmpQLabel = new QLabel( parent, "Label_4" );
	tmpQLabel->setGeometry( 10, 150, 300, 30 );
	tmpQLabel->setText( "Filesystems ( iso9660,msdos,minix,default )" );

	fstype = new QLineEdit( parent, "LineEdit_3" );
	fstype->setGeometry( 10, 180, 280, 30 );
	fstype->setText( "" );

	tmpQLabel = new QLabel( parent, "Label_5" );
	tmpQLabel->setGeometry( 10, 220, 100, 30 );
	tmpQLabel->setText( "Mounted Icon" );

	tmpQLabel = new QLabel( parent, "Label_6" );
	tmpQLabel->setGeometry( 170, 220, 100, 30 );
	tmpQLabel->setText( "Unmounted Icon" );

	QComboBox* tmpQComboBox;
	tmpQComboBox = new QComboBox( false, parent, "ComboBox_1" );
	tmpQComboBox->setGeometry( 10, 250, 120, 30 );
	tmpQComboBox->setSizeLimit( 10 );

	tmpQComboBox = new QComboBox( false, parent, "ComboBox_2" );
	tmpQComboBox->setGeometry( 170, 250, 120, 30 );
	tmpQComboBox->setSizeLimit( 10 );

	parent->resize( 400, 300 );
}
