//
//
// "Root Options" Tab for KFM configuration
//
// (c) Martin R. Jones 1996
// (c) Bernd Wuebben 1998
//
// Layouts
// (c) Christian Tibirna 1998
// Port to KControl, split from Misc Tab
// (c) David Faure 1998

#include <qlabel.h>
#include <qgroupbox.h>
#include <qlayout.h>//CT - 12Nov1998
#include <kapp.h>

#include "rootopts.h"

#include "../../kfm/config-kfm.h" // include default values directly from kfm

//-----------------------------------------------------------------------------

// Define some constants to make additions easier (David)
#define ROW_HGRID 1
#define ROW_VGRID 3
#define ROW_TRANSPARENT 4
#define ROW_ICON_FG 5
#define ROW_ICON_BG 7

KRootOptions::KRootOptions( QWidget *parent, const char *name )
    : KConfigWidget( parent, name )
{
        //CT 12Nov1998 
        QGridLayout *lay = new QGridLayout(this,9 /*rows*/,5 /*cols*/,
                                           10 /*border*/,5 /*autoborder*/);
        lay->addRowSpacing(0,15);
        lay->addRowSpacing(1,30); // HGRID
        lay->addRowSpacing(2, 5);
        lay->addRowSpacing(3,30); // VGRID
        lay->addRowSpacing(4,30); // TRANSPARENT
        lay->addRowSpacing(5,30); // ICON_FG
        lay->addRowSpacing(6, 5);
        lay->addRowSpacing(7,30); // ICON_BG
        lay->addRowSpacing(8,10);

        lay->addColSpacing(0,10);
        lay->addColSpacing(2,10);
        lay->addColSpacing(3,80);
        lay->addColSpacing(4,35); //sven: colorbuttons are big
 
        lay->setRowStretch(0,0);
        lay->setRowStretch(1,0); // HGRID
        lay->setRowStretch(2,1);
        lay->setRowStretch(3,0); // VGRID
        lay->setRowStretch(4,1); // TRANSPARENT
        lay->setRowStretch(5,0); // ICON_FG
        lay->setRowStretch(6,1);
        lay->setRowStretch(7,0); // ICON_BG
        lay->setRowStretch(8,0);
 
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
        lay->addWidget(label,ROW_HGRID,1);
 
        hspin  = new KNumericSpinBox(this);
        hspin->adjustSize();
        hspin->setMinimumSize(hspin->size());
        lay->addWidget(hspin,ROW_HGRID,3);
        hspin->setRange(0,DEFAULT_GRID_MAX - DEFAULT_GRID_MIN);
 
        label = new QLabel( klocale->translate(
                            "Vertical Root Grid Spacing:"), this );
        label->adjustSize();
        label->setMinimumSize(label->size());
        lay->addWidget(label,ROW_VGRID,1);
 
        vspin  = new KNumericSpinBox(this);
        vspin->adjustSize();
        vspin->setMinimumSize(vspin->size());
        lay->addWidget(vspin,ROW_VGRID,3);
        vspin->setRange(0,DEFAULT_GRID_MAX - DEFAULT_GRID_MIN);
 
        iconstylebox = new QCheckBox(klocale->translate("&Transparent Text for Root Icons."),
                                  this);
        //CT 12Nov1998
        iconstylebox->adjustSize();
        iconstylebox->setMinimumSize(iconstylebox->size());
        lay->addMultiCellWidget(iconstylebox,ROW_TRANSPARENT,ROW_TRANSPARENT,1,3);
        //CT 
 
        connect(iconstylebox,SIGNAL(toggled(bool)),this,SLOT(makeBgActive(bool)));
 
        //CT 12Nov1998 color buttons
        label = new QLabel(klocale->translate("Icon foreground color:"),this);
        label->adjustSize();
        label->setMinimumSize(label->size());
        lay->addWidget(label,ROW_ICON_FG,1);
 
        fgColorBtn = new KColorButton(icon_fg,this);
        fgColorBtn->adjustSize();
        fgColorBtn->setMinimumSize(fgColorBtn->size());
        lay->addWidget(fgColorBtn,ROW_ICON_FG,3);
        connect( fgColorBtn, SIGNAL( changed( const QColor & ) ),
                SLOT( slotIconFgColorChanged( const QColor & ) ) );
 
        bgLabel = new QLabel(klocale->translate("Icon background color:"),this);
        bgLabel->adjustSize();
        bgLabel->setMinimumSize(bgLabel->size());
        lay->addWidget(bgLabel,ROW_ICON_BG,1);
 
        bgColorBtn = new KColorButton(icon_bg,this);
        bgColorBtn->adjustSize();
        bgColorBtn->setMinimumSize(bgColorBtn->size());
        lay->addWidget(bgColorBtn,ROW_ICON_BG,3);
        connect( bgColorBtn, SIGNAL( changed( const QColor & ) ),
                SLOT( slotIconBgColorChanged( const QColor & ) ) );
 
        lay->activate();
        //CT
 
        loadSettings();
 
        setMinimumSize( 480, 180 );
}

void KRootOptions::loadSettings()
{
    // *** load ***

    // Root Icons settings
    g_pConfig->setGroup( "KFM Root Icons" );
    bool bTransparent = (bool)g_pConfig->readNumEntry("Style", DEFAULT_ROOT_ICONS_STYLE);
    //CT 12Nov1998
    icon_fg = g_pConfig->readColorEntry("Foreground",&DEFAULT_ICON_FG);
    icon_bg = g_pConfig->readColorEntry("Background",&DEFAULT_ICON_BG);
    //CT
    g_pConfig->setGroup( "KFM Root Defaults" );
    int gridwidth = g_pConfig->readNumEntry( "GridWidth", DEFAULT_GRID_WIDTH );
    int gridheight = g_pConfig->readNumEntry( "GridHeight", DEFAULT_GRID_HEIGHT );
    
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
}

void KRootOptions::defaultSettings()
{
    // Root Icons Settings
    iconstylebox->setChecked((bool)DEFAULT_ROOT_ICONS_STYLE);
    makeBgActive((bool)DEFAULT_ROOT_ICONS_STYLE);
    fgColorBtn->setColor(DEFAULT_ICON_FG);
    bgColorBtn->setColor(DEFAULT_ICON_BG);
    hspin->setValue(DEFAULT_GRID_WIDTH - DEFAULT_GRID_MIN);
    vspin->setValue(DEFAULT_GRID_HEIGHT - DEFAULT_GRID_MIN);
}

void KRootOptions::saveSettings()
{
    // Root Icons Settings
    g_pConfig->setGroup( "KFM Root Icons" );
    g_pConfig->writeEntry( "Style", iconstylebox->isChecked() ? 1 : 0);
    //CT 12Nov1998
    g_pConfig->writeEntry( "Foreground", icon_fg);
    g_pConfig->writeEntry( "Background", icon_bg);
    //CT
    g_pConfig->setGroup( "KFM Root Defaults" );
    g_pConfig->writeEntry( "GridWidth", hspin->getValue()+DEFAULT_GRID_MIN);
    g_pConfig->writeEntry( "GridHeight", vspin->getValue()+DEFAULT_GRID_MIN);

    g_pConfig->sync();
}

void KRootOptions::applySettings()
{
    saveSettings();
}

//CT 12Nov1998
void KRootOptions::slotIconFgColorChanged(const QColor &col) {
    if ( icon_fg != col )
        icon_fg = col;
}
 
void KRootOptions::slotIconBgColorChanged(const QColor &col) {
    if ( icon_bg != col )
        icon_bg = col;
}
 
void KRootOptions::makeBgActive(bool a) {
  bgColorBtn->setEnabled(!a);
  bgLabel->setEnabled(!a);
}
//CT

#include "rootopts.moc"
