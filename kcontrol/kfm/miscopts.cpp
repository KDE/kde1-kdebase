//
//
// "Misc Options" Tab for KFM configuration
//
// (c) Sven Radej 1998
// (c) David Faure 1998

#include <qlabel.h>
#include <qgroupbox.h>
#include <qlayout.h>//CT - 12Nov1998
#include <kapp.h>

#include "miscopts.h"

#include "../../kfm/config-kfm.h" // include default values directly from kfm

//-----------------------------------------------------------------------------

KMiscOptions::KMiscOptions( QWidget *parent, const char *name )
    : KConfigWidget( parent, name )
{
        QVBoxLayout *lay = new QVBoxLayout(this, 40 /* big border */, 20);
 
        //----------------- sven---------
        urlpropsbox = new QCheckBox(i18n("&Allow per-URL settings"),
                                    this);
        urlpropsbox->adjustSize();
        urlpropsbox->setMinimumSize(urlpropsbox->size());
        lay->addWidget(urlpropsbox);
        //-------------------------------
 
        //----------------- david ---------
        treefollowbox = new QCheckBox(i18n("Tree &view follows navigation"),
                                    this);
        treefollowbox->adjustSize();
        treefollowbox->setMinimumSize(treefollowbox->size());
        lay->addWidget(treefollowbox);
        //-------------------------------

        lay->addStretch(10);
        lay->activate();
 
        loadSettings();
 
        setMinimumSize( 400, 100 );
}

void KMiscOptions::loadSettings()
{
    // *** load ***

    bool bUrlprops = g_pConfig->readBoolEntry( "EnablePerURLProps", false);
    bool bTreeFollow = g_pConfig->readBoolEntry( "TreeFollowsView", false);

    // *** apply to GUI ***

    urlpropsbox->setChecked(bUrlprops);
    treefollowbox->setChecked(bTreeFollow);
}

void KMiscOptions::defaultSettings()
{
    urlpropsbox->setChecked(false);
    treefollowbox->setChecked(false);
}

void KMiscOptions::saveSettings()
{
    g_pConfig->setGroup( "KFM Root Icons" );
    g_pConfig->writeEntry( "EnablePerURLProps", urlpropsbox->isChecked());
    g_pConfig->writeEntry( "TreeFollowsView", treefollowbox->isChecked());
    g_pConfig->sync();
}

void KMiscOptions::applySettings()
{
    saveSettings();
}

#include "miscopts.moc"
