#ifndef DevicePropsPage_included
#define DevicePropsPage_included

#include <qwidget.h>

#include "DevicePropsPageData.h"

class DevicePropsPage : public QWidget,
			private DevicePropsPageData
{
    Q_OBJECT

public:

    DevicePropsPage
    (
        QWidget* parent = NULL,
        const char* name = NULL
    );

    virtual ~DevicePropsPage();

};
#endif // DevicePropsPage_included
