//
//
// "Misc Options" Tab for KFM configuration
//
// (c) Martin R. Jones 1996
// (c) Bernd Wuebben 1998
//
// Layouts
// (c) Christian Tibirna 1998
// Port to KControl
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
        //CT 12Nov1998
        //Sven: Inserted my checkbox (urlpropsbox) in CT's layout
        //David: Inserted my checkbox (treefollowbox) in CT's layout
 
        QGridLayout *lay = new QGridLayout(this,10,5,10,5);
        lay->addRowSpacing(0,15);
        lay->addRowSpacing(1,30);
        lay->addRowSpacing(2, 5);
        lay->addRowSpacing(3,30);
        lay->addRowSpacing(7,30);
        lay->addRowSpacing(8, 5);
        lay->addRowSpacing(9,30);
        lay->addRowSpacing(10,10);
        lay->addColSpacing(0,10);
        lay->addColSpacing(2,10);
        lay->addColSpacing(3,80);
        lay->addColSpacing(4,35); //sven: colorbuttons are big
 
        lay->setRowStretch(0,0);
        lay->setRowStretch(1,0);
        lay->setRowStretch(2,1);
        lay->setRowStretch(3,0);
        lay->setRowStretch(4,1); // per-url
        lay->setRowStretch(5,1); // tree follows view
        lay->setRowStretch(6,1); // transparent text
        lay->setRowStretch(7,0);
        lay->setRowStretch(8,1);
        lay->setRowStretch(9,0);
        lay->setRowStretch(10,0);
 
        lay->setColStretch(0,0);
        lay->setColStretch(1,0);
        lay->setColStretch(2,1);
        lay->setColStretch(3,0);
        lay->setColStretch(4,1);
        //CT
 
        QLabel *label;
        label = new QLabel( klocale->translate(
                            "Horizontal Root Grid Spacing:"), this );
        label->adjustSize();
        label->setMinimumSize(label->size());
        lay->addWidget(label,1,1);
 
        hspin  = new KNumericSpinBox(this);
        hspin->adjustSize();
        hspin->setMinimumSize(hspin->size());
        lay->addWidget(hspin,1,3);
        hspin->setRange(0,DEFAULT_GRID_MAX - DEFAULT_GRID_MIN);
 
        label = new QLabel( klocale->translate(
                            "Vertical Root Grid Spacing:"), this );
        label->adjustSize();
        label->setMinimumSize(label->size());
        lay->addWidget(label,3,1);
 
        vspin  = new KNumericSpinBox(this);
        vspin->adjustSize();
        vspin->setMinimumSize(vspin->size());
        lay->addWidget(vspin,3,3);
        vspin->setRange(0,DEFAULT_GRID_MAX - DEFAULT_GRID_MIN);
 
        //----------------- sven---------
        urlpropsbox = new QCheckBox(klocale->translate("&Allow per-URL settings"),
                                    this);
        urlpropsbox->adjustSize();
        urlpropsbox->setMinimumSize(urlpropsbox->size());
        lay->addMultiCellWidget(urlpropsbox,4,4,1,3);
        //-------------------------------
 
        //----------------- david ---------
        treefollowbox = new QCheckBox(klocale->translate("Tree &view follows navigation"),
                                    this);
        treefollowbox->adjustSize();
        treefollowbox->setMinimumSize(treefollowbox->size());
        lay->addMultiCellWidget(treefollowbox,5,5,1,3);
        //-------------------------------
 
        iconstylebox = new QCheckBox(klocale->translate("&Transparent Text for Root Icons."),
                                  this);
        //CT 12Nov1998
        iconstylebox->adjustSize();
        iconstylebox->setMinimumSize(iconstylebox->size());
        lay->addMultiCellWidget(iconstylebox,6,6,1,3);
        //CT 
 
        connect(iconstylebox,SIGNAL(toggled(bool)),this,SLOT(makeBgActive(bool)));
 
        //CT 12Nov1998 color buttons
        label = new QLabel(klocale->translate("Icon foreground color:"),this);
        label->adjustSize();
        label->setMinimumSize(label->size());
        lay->addWidget(label,7,1);
 
        fgColorBtn = new KColorButton(icon_fg,this);
        fgColorBtn->adjustSize();
        fgColorBtn->setMinimumSize(fgColorBtn->size());
        lay->addWidget(fgColorBtn,7,3);
        connect( fgColorBtn, SIGNAL( changed( const QColor & ) ),
                SLOT( slotIconFgColorChanged( const QColor & ) ) );
 
        bgLabel = new QLabel(klocale->translate("Icon background color:"),this);
        bgLabel->adjustSize();
        bgLabel->setMinimumSize(bgLabel->size());
        lay->addWidget(bgLabel,9,1);
 
        bgColorBtn = new KColorButton(icon_bg,this);
        bgColorBtn->adjustSize();
        bgColorBtn->setMinimumSize(bgColorBtn->size());
        lay->addWidget(bgColorBtn,9,3);
        connect( bgColorBtn, SIGNAL( changed( const QColor & ) ),
                SLOT( slotIconBgColorChanged( const QColor & ) ) );
 
        lay->activate();
        //CT
 
        loadSettings();
 
        setMinimumSize( 480, 180 );
}

void KMiscOptions::loadSettings()
{
    // *** load ***

    // Root Icons settings
    g_pConfig->setGroup( "KFM Root Icons" );
    bool bTransparent = (bool)g_pConfig->readNumEntry("Style", DEFAULT_ROOT_ICONS_STYLE);
    //CT 12Nov1998
    icon_fg = g_pConfig->readColorEntry("Foreground",&DEFAULT_ICON_FG);
    icon_bg = g_pConfig->readColorEntry("Background",&DEFAULT_ICON_BG);
    //CT
    g_pConfig->setGroup( "KFM Misc Defaults" );
    int gridwidth = g_pConfig->readNumEntry( "GridWidth", DEFAULT_GRID_WIDTH );
    int gridheight = g_pConfig->readNumEntry( "GridHeight", DEFAULT_GRID_HEIGHT );
    
    // Misc Settings
    bool bUrlprops = g_pConfig->readBoolEntry( "EnablePerURLProps", false);
    bool bTreeFollow = g_pConfig->readBoolEntry( "TreeFollowsView", false);

    // *** apply to GUI ***

    // Root Icon Settings
    iconstylebox->setChecked(bTransparent);
    makeBgActive(bTransparent);

    fgColorBtn->setColor(icon_fg);
    bgColorBtn->setColor(icon_bg);

    if(gridwidth - DEFAULT_GRID_MIN < 0 )
        gridwidth = DEFAULT_GRID_MIN;
    hspin->setValue(gridwidth - DEFAULT_GRID_MIN);

    if(gridheight - DEFAULT_GRID_MIN < 0 )
        gridheight = DEFAULT_GRID_MIN;
    vspin->setValue(gridheight - DEFAULT_GRID_MIN);

    // Misc Settings
    urlpropsbox->setChecked(bUrlprops);
    treefollowbox->setChecked(bTreeFollow);
}

void KMiscOptions::defaultSettings()
{
    // Root Icons Settings
    iconstylebox->setChecked((bool)DEFAULT_ROOT_ICONS_STYLE);
    makeBgActive((bool)DEFAULT_ROOT_ICONS_STYLE);
    fgColorBtn->setColor(DEFAULT_ICON_FG);
    bgColorBtn->setColor(DEFAULT_ICON_BG);
    hspin->setValue(DEFAULT_GRID_WIDTH - DEFAULT_GRID_MIN);
    vspin->setValue(DEFAULT_GRID_HEIGHT - DEFAULT_GRID_MIN);

    // Misc Settings
    urlpropsbox->setChecked(false);
    treefollowbox->setChecked(false);
}

void KMiscOptions::saveSettings()
{
    // Root Icons Settings
    g_pConfig->setGroup( "KFM Root Icons" );
    g_pConfig->writeEntry( "Style", iconstylebox->isChecked() ? 1 : 0);
    //CT 12Nov1998
    g_pConfig->writeEntry( "Foreground", icon_fg);
    g_pConfig->writeEntry( "Background", icon_bg);
    //CT
    g_pConfig->setGroup( "KFM Misc Defaults" );
    g_pConfig->writeEntry( "GridWidth", hspin->getValue()+DEFAULT_GRID_MIN);
    g_pConfig->writeEntry( "GridHeight", vspin->getValue()+DEFAULT_GRID_MIN);

    // Misc Settings
    g_pConfig->writeEntry( "EnablePerURLProps", urlpropsbox->isChecked());
    g_pConfig->writeEntry( "TreeFollowsView", treefollowbox->isChecked());

    g_pConfig->sync();
}

void KMiscOptions::applySettings()
{
    saveSettings();
}

//CT 12Nov1998
void KMiscOptions::slotIconFgColorChanged(const QColor &col) {
    if ( icon_fg != col )
        icon_fg = col;
}
 
void KMiscOptions::slotIconBgColorChanged(const QColor &col) {
    if ( icon_bg != col )
        icon_bg = col;
}
 
void KMiscOptions::makeBgActive(bool a) {
  bgColorBtn->setEnabled(!a);
  bgLabel->setEnabled(!a);
}
//CT

#include "miscopts.moc"
