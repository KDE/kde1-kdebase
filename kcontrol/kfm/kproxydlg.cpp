// File kproxydlg.cpp by Lars Hoss <Lars.Hoss@munich.netsurf.de>
// Port to KControl by David Faure <faure@kde.org>

#include <qlayout.h> //CT

#include <ktablistbox.h>
#include <klocale.h>
#include <kapp.h>
#include <kurl.h>
#include <string.h>
#include "kproxydlg.h"

#define ROW_USEPROXY 1
#define ROW_HTTP 2
#define ROW_FTP 3
#define ROW_NOPROXY 5

KProxyOptions::KProxyOptions(QWidget *parent, const char *name)
  : KConfigWidget(parent, name)
{

	//CT 12Nov1998 layout management
	QGridLayout *lay = new QGridLayout(this,7,8,10,5);
	lay->addRowSpacing(4,20);
	lay->addRowSpacing(6,20);
	lay->addColSpacing(0,10);
	lay->addColSpacing(3,20);
	lay->addColSpacing(7,10);

	lay->setRowStretch(0,1);
	lay->setRowStretch(1,0); // USEPROXY
	lay->setRowStretch(2,0); // HTTP
	lay->setRowStretch(3,0); // FTP
	lay->setRowStretch(4,1);
	lay->setRowStretch(5,0); // NOPROXY
	lay->setRowStretch(6,1);

	lay->setColStretch(0,0);
	lay->setColStretch(1,0);
	lay->setColStretch(2,1);
	lay->setColStretch(3,1);
	lay->setColStretch(4,0);
	lay->setColStretch(5,1);
	lay->setColStretch(6,1);
	lay->setColStretch(7,0);
	//CT

// Commented out, since in any KControl module, one has to save ("OK" or "Apply") :)
#if 0
  lb_info = new QLabel( klocale->translate("Don't forget to save your settings!"), this);
  //CT 12Nov1998 layout management
  lb_info->adjustSize();
  lb_info->setMinimumSize(lb_info->size());
  lay->addMultiCellWidget(lb_info,1,1,1,6);
  //CT
#endif

  cb_useProxy = new QCheckBox( klocale->translate("Use proxy"), this );
  //CT 12Nov1998 layout management
  cb_useProxy->adjustSize();
  cb_useProxy->setMinimumSize(cb_useProxy->size());
  lay->addMultiCellWidget(cb_useProxy,ROW_USEPROXY,ROW_USEPROXY,1,6);
  //CT

  connect( cb_useProxy, SIGNAL( clicked() ), SLOT( changeProxy() ) );
  
  lb_http_url = new QLabel("HTTP Proxy:", this);
  lb_http_url->setAlignment(AlignVCenter);
  //CT 12Nov1998 layout management
  lb_http_url->adjustSize();
  lb_http_url->setMinimumSize(lb_http_url->size());
  lay->addWidget(lb_http_url,ROW_HTTP,1);
  //CT

  le_http_url = new QLineEdit(this);
  //CT 12Nov1998 layout management
  le_http_url->adjustSize();
  le_http_url->setMinimumSize(le_http_url->size());
  lay->addWidget(le_http_url,ROW_HTTP,2);
  //CT

  lb_http_port = new QLabel("Port:", this);
  lb_http_port->setAlignment(AlignVCenter);
  //CT 12Nov1998 layout management
  lb_http_port->adjustSize();
  lb_http_port->setMinimumSize(lb_http_port->size());
  lay->addWidget(lb_http_port,ROW_HTTP,4);
  //CT

  le_http_port = new QLineEdit(this);
  le_http_port->setGeometry(280, 110, 55, 30);
  //CT 12Nov1998 layout management
  le_http_port->adjustSize();
  le_http_port->setMinimumSize(le_http_port->size());
  lay->addWidget(le_http_port,ROW_HTTP,5);
  //CT

  lb_ftp_url = new QLabel("FTP Proxy:", this);
  lb_ftp_url->setAlignment(AlignVCenter);
  //CT 12Nov1998 layout management
  lb_ftp_url->adjustSize();
  lb_ftp_url->setMinimumSize(lb_ftp_url->size());
  lay->addWidget(lb_ftp_url,ROW_FTP,1);
  //CT

  le_ftp_url = new QLineEdit(this);
  //CT 12Nov1998 layout management
  le_ftp_url->adjustSize();
  le_ftp_url->setMinimumSize(le_ftp_url->size());
  lay->addWidget(le_ftp_url,ROW_FTP,2);
  //CT

  lb_ftp_port = new QLabel("Port:", this);
  lb_ftp_port->setAlignment(AlignVCenter);
  //CT 12Nov1998 layout management
  lb_ftp_port->adjustSize();
  lb_ftp_port->setMinimumSize(lb_ftp_port->size());
  lay->addWidget(lb_ftp_port,ROW_FTP,4);
  //CT

  le_ftp_port = new QLineEdit(this);
  //CT 12Nov1998 layout management
  le_ftp_port->adjustSize();
  le_ftp_port->setMinimumSize(le_ftp_port->size());
  lay->addWidget(le_ftp_port,ROW_FTP,5);
  //CT

  lb_no_prx = new QLabel(klocale->translate("No Proxy for:"), this);
  lb_no_prx->setAlignment(AlignVCenter);
  //CT 12Nov1998 layout management
  lb_no_prx->adjustSize();
  lb_no_prx->setMinimumSize(lb_no_prx->size());
  lay->addWidget(lb_no_prx,ROW_NOPROXY,1);
  //CT

  le_no_prx = new QLineEdit(this);
  //CT 12Nov1998 layout management
  le_no_prx->adjustSize();
  le_no_prx->setMinimumSize(le_no_prx->size());
  lay->addMultiCellWidget(le_no_prx,ROW_NOPROXY,ROW_NOPROXY,2,5);
  //CT


  QString path;
  QPixmap pixmap;
  pixmap.load( KApplication::kde_toolbardir() + "/down.xpm" ); 
  cp_down = new QPushButton( this );
  cp_down->setPixmap( pixmap );
  //CT 12Nov1998
  cp_down->setFixedSize(20,20);
  cp_down->setMinimumSize(cp_down->size());
  lay->addWidget(cp_down,ROW_HTTP,6);

  lay->activate();
  //CT

  connect( cp_down, SIGNAL( clicked() ), SLOT( copyDown() ) );

  setMinimumSize(480,300);

  // finaly read the options
  loadSettings();
}

KProxyOptions::~KProxyOptions()
{
  // now delete everything we allocated before
  // delete lb_info;
  delete lb_http_url;
  delete le_http_url;
  delete lb_http_port;
  delete le_http_port;
  delete lb_ftp_url;
  delete le_ftp_url;
  delete lb_ftp_port;
  delete le_ftp_port;
  delete cp_down;
  delete cb_useProxy;
  // time to say goodbye ...
}

void KProxyOptions::loadSettings()
{
  g_pConfig->setGroup( "Browser Settings/Proxy" );	
  updateGUI (
      g_pConfig->readEntry( "HTTP-Proxy" ),
      g_pConfig->readEntry( "FTP-Proxy" ),
      g_pConfig->readEntry( "UseProxy" ),
      g_pConfig->readEntry( "NoProxyFor" )
      );
}

void KProxyOptions::defaultSettings() {
  updateGUI ("","","No","");
}

void KProxyOptions::updateGUI(QString httpProxy, QString ftpProxy, QString sUseProxy,
               QString noProxyFor)
{
  QString port;
  KURL url;

  if( !httpProxy.isEmpty() ) {
    url = httpProxy.data();
    port.setNum( url.port() );
    le_http_url->setText( url.host() );
    le_http_port->setText( port.data() ); 
  }

  if( !ftpProxy.isEmpty() ) {
    url = ftpProxy.data();
    port.setNum( url.port() );
    le_ftp_url->setText( url.host() );
    le_ftp_port->setText( port.data() ); 
  }

  if( sUseProxy == "Yes" ) {
    useProxy = TRUE;
  } else {
    useProxy = FALSE;
  }
  setProxy();
  
  le_no_prx->setText( noProxyFor.data() );  

}

void KProxyOptions::saveSettings()
{
    g_pConfig->setGroup( "Browser Settings/Proxy" );	
    QString url;

    url = le_http_url->text();
    if( !url.isEmpty() ) {
        url = "http://";
        url += le_http_url->text();     // host
        url += ":";
        url += le_http_port->text();    // and port
    }
    g_pConfig->writeEntry( "HTTP-Proxy", url );

    url = le_ftp_url->text();
    if( !url.isEmpty() ) {
        url = "ftp://";
        url += le_ftp_url->text();       // host
        url += ":";
        url += le_ftp_port->text();      // and port
    }
    g_pConfig->writeEntry( "FTP-Proxy", url );

    g_pConfig->writeEntry( "UseProxy", useProxy ? "Yes" : "No" );
    g_pConfig->writeEntry( "NoProxyFor", le_no_prx->text() );
    g_pConfig->sync();
}

void KProxyOptions::applySettings()
{
    saveSettings();
}

void KProxyOptions::copyDown()
{
  le_ftp_url->setText( le_http_url->text() );
  le_ftp_port->setText( le_http_port->text() );
}

void KProxyOptions::setProxy()
{
  // now set all input fields
  le_http_url->setEnabled( useProxy );
  le_http_port->setEnabled( useProxy );
  le_ftp_url->setEnabled( useProxy );
  le_ftp_port->setEnabled( useProxy );
  le_no_prx->setEnabled( useProxy );
  cp_down->setEnabled( useProxy );
  cb_useProxy->setChecked( useProxy );
}

void KProxyOptions::changeProxy()
{
  useProxy = (! useProxy);
  setProxy();
}

#include "kproxydlg.moc"
