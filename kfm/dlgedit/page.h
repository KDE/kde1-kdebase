#ifndef DevicePropsPage_h
#define DevicePropsPage_h

#include <qchkbox.h>
#include <qlined.h>

class DevicePropsPage : public PropsPage
{
    Q_OBJECT
public:
    DevicePropsPage( Properties *_props );
    ~DevicePropsPage() { }
    
protected:
    QLineEdit* device;
    QLineEdit* mountpoint;
    QCheckBox* readonly;
    QLineEdit* fstype;
    QComboBox* mounted;
    QComboBox* unmounted;
};

#endif
