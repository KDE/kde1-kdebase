
#include "display.h"

#include "display.moc"

KDisplayModule::KDisplayModule( QWidget *parent, int mode, int )
	: KConfigWidget( parent )
{
	_runMode = mode;
}

