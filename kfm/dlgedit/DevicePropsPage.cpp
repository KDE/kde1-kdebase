#include "DevicePropsPage.h"

#define Inherited QWidget

DevicePropsPage::DevicePropsPage
(
	QWidget* parent,
	const char* name
)
	:
	Inherited( parent, name ),
	DevicePropsPageData( this )
{
}


DevicePropsPage::~DevicePropsPage()
{
}
