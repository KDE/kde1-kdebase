#ifndef DevicePropsPage_h
#define DevicePropsPage_h

#include <qchkbox.h>
#include <qlined.h>

class DevicePropsPage : public PropsPage
{
    Q_OBJECT
public:
    DevicePropsPageData( QWidget* parent  );

protected:
    QLineEdit* device;
    QLineEdit* mountpoint;
    QCheckBox* readonly;
    QLineEdit* fstype;
};

#endif
