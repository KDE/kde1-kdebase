//
//
// "Desktop Icons Options" Tab for KFM configuration
//
// (c) Martin R. Jones 1996
//
// Port to KControl, split from "Misc" Tab.
// (c) David Faure 1998

#ifndef __KFM_ROOT_OPTIONS_H
#define __KFM_ROOT_OPTIONS_H

#include <qtabdlg.h>
#include <qstrlist.h>
#include <qchkbox.h>
#include <kspinbox.h>
#include <kcolorbtn.h>

#include <kconfig.h>
#include <kcontrol.h>

extern KConfigBase *g_pConfig;

//-----------------------------------------------------------------------------
// The "Desktop Icons Options" Tab contains :

// Horizontal Grid Spacing, Vertical Grid Spacing
// Transparent text
// bg & fg color for root icons

class KRootOptions : public KConfigWidget
{
        Q_OBJECT
public:
        KRootOptions( QWidget *parent = 0L, const char *name = 0L );
        virtual void loadSettings();
        virtual void saveSettings();
        virtual void applySettings();
        virtual void defaultSettings();
 
 private slots:
        void slotIconFgColorChanged(const QColor &);
        void slotIconBgColorChanged(const QColor &);
        void makeBgActive( bool );
 
private:
        QLabel * bgLabel;

        KNumericSpinBox *hspin;
        KNumericSpinBox *vspin;
        QCheckBox *iconstylebox;
        KColorButton *fgColorBtn;
        KColorButton *bgColorBtn;

        QColor icon_fg;
        QColor icon_bg;
};

#endif // __KFM_ROOT_OPTIONS_H
