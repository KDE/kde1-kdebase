// File kproxydlg.cpp by Lars Hoss ( Lars.Hoss@munich.netsurf.de )

#include <qlayout.h> //CT

#include <ktablistbox.h>
#include <klocale.h>
#include <kapp.h>
#include <kurl.h>
#include <string.h>
#include "kproxydlg.h"

KProxyDlg::KProxyDlg(QWidget *parent, const char *name, WFlags f)
  : QWidget(parent, name, f)
{

	//CT 12Nov1998 layout management
	QGridLayout *lay = new QGridLayout(this,9,8,10,5);
	lay->addRowSpacing(0,10);
	lay->addRowSpacing(3,20);
	lay->addRowSpacing(6,30);
	lay->addRowSpacing(8,10);
	lay->addColSpacing(0,10);
	lay->addColSpacing(3,20);
	lay->addColSpacing(7,10);

	lay->setRowStretch(0,0);
	lay->setRowStretch(1,1);
	lay->setRowStretch(2,1);
	lay->setRowStretch(3,1);
	lay->setRowStretch(4,0);
	lay->setRowStretch(5,0);
	lay->setRowStretch(6,1);
	lay->setRowStretch(7,0);
	lay->setRowStretch(8,0);

	lay->setColStretch(0,0);
	lay->setColStretch(1,0);
	lay->setColStretch(2,1);
	lay->setColStretch(3,1);
	lay->setColStretch(4,0);
	lay->setColStretch(5,1);
	lay->setColStretch(5,1);
	lay->setColStretch(7,0);
	//CT

  lb_info = new QLabel( klocale->translate("Don't forget to save your settings!"), this);
  //CT  lb_info->setGeometry(20, 20, 360, 30);
  //CT 12Nov1998 layout management
  lb_info->adjustSize();
  lb_info->setMinimumSize(lb_info->size());
  lay->addMultiCellWidget(lb_info,1,1,1,6);
  //CT

  cb_useProxy = new QCheckBox( klocale->translate("Use proxy"), this );
  //CT  cb_useProxy->setGeometry( 20, 70, 140, 30 );
  //CT 12Nov1998 layout management
  cb_useProxy->adjustSize();
  cb_useProxy->setMinimumSize(cb_useProxy->size());
  lay->addMultiCellWidget(cb_useProxy,3,3,1,6);
  //CT

  connect( cb_useProxy, SIGNAL( clicked() ), SLOT( changeProxy() ) );
  
  lb_http_url = new QLabel("HTTP Proxy:", this);
  lb_http_url->setAlignment(/*CT AlignRight | */AlignVCenter);
  //CT  lb_http_url->setGeometry(20, 110, 80, 30);
  //CT 12Nov1998 layout management
  lb_http_url->adjustSize();
  lb_http_url->setMinimumSize(lb_http_url->size());
  lay->addWidget(lb_http_url,4,1);
  //CT

  le_http_url = new QLineEdit(this);
  //CT  le_http_url->setGeometry(110, 110, 105, 30);
  //CT 12Nov1998 layout management
  le_http_url->adjustSize();
  le_http_url->setMinimumSize(le_http_url->size());
  lay->addWidget(le_http_url,4,2);
  //CT

  lb_http_port = new QLabel("Port:", this);
  lb_http_port->setAlignment(/*CT AlignRight | */AlignVCenter);
  //CT  lb_http_port->setGeometry(230, 110, 40, 30);
  //CT 12Nov1998 layout management
  lb_http_port->adjustSize();
  lb_http_port->setMinimumSize(lb_http_port->size());
  lay->addWidget(lb_http_port,4,4);
  //CT

  le_http_port = new QLineEdit(this);
  le_http_port->setGeometry(280, 110, 55, 30);
  //CT 12Nov1998 layout management
  le_http_port->adjustSize();
  le_http_port->setMinimumSize(le_http_port->size());
  lay->addWidget(le_http_port,4,5);
  //CT

  lb_ftp_url = new QLabel("FTP Proxy:", this);
  lb_ftp_url->setAlignment(/*CT AlignRight | */AlignVCenter);
  //CT  lb_ftp_url->setGeometry(20, 150, 80, 30);
  //CT 12Nov1998 layout management
  lb_ftp_url->adjustSize();
  lb_ftp_url->setMinimumSize(lb_ftp_url->size());
  lay->addWidget(lb_ftp_url,5,1);
  //CT

  le_ftp_url = new QLineEdit(this);
  //CT  le_ftp_url->setGeometry(110, 150, 105, 30);
  //CT 12Nov1998 layout management
  le_ftp_url->adjustSize();
  le_ftp_url->setMinimumSize(le_ftp_url->size());
  lay->addWidget(le_ftp_url,5,2);
  //CT

  lb_ftp_port = new QLabel("Port:", this);
  lb_ftp_port->setAlignment(/*CT AlignRight | */AlignVCenter);
  //CT  lb_ftp_port->setGeometry(230, 150, 40, 30);
  //CT 12Nov1998 layout management
  lb_ftp_port->adjustSize();
  lb_ftp_port->setMinimumSize(lb_ftp_port->size());
  lay->addWidget(lb_ftp_port,5,4);
  //CT

  le_ftp_port = new QLineEdit(this);
  //CT  le_ftp_port->setGeometry(280, 150, 55, 30);
  //CT 12Nov1998 layout management
  le_ftp_port->adjustSize();
  le_ftp_port->setMinimumSize(le_ftp_port->size());
  lay->addWidget(le_ftp_port,5,5);
  //CT

  lb_no_prx = new QLabel(klocale->translate("No Proxy for:"), this);
  lb_no_prx->setAlignment(/*CT AlignRight | */AlignVCenter);
  //CT  lb_no_prx->setGeometry(20, 240, 80, 30);
  //CT 12Nov1998 layout management
  lb_no_prx->adjustSize();
  lb_no_prx->setMinimumSize(lb_no_prx->size());
  lay->addWidget(lb_no_prx,7,1);
  //CT

  le_no_prx = new QLineEdit(this);
  //CT  le_no_prx->setGeometry(110, 240, 225, 30);
  //CT 12Nov1998 layout management
  le_no_prx->adjustSize();
  le_no_prx->setMinimumSize(le_no_prx->size());
  lay->addMultiCellWidget(le_no_prx,7,7,2,5);
  //CT


  QString path;
  QPixmap pixmap;
  pixmap.load( KApplication::kde_toolbardir() + "/down.xpm" ); 
  cp_down = new QPushButton( this );
  cp_down->setPixmap( pixmap );
  //CT  cp_down->setGeometry(342, 115, 20, 20);
  cp_down->setFixedSize(20,20);
  cp_down->setMinimumSize(cp_down->size());
  lay->addWidget(cp_down,4,6);

  lay->activate();
  //CT

  connect( cp_down, SIGNAL( clicked() ), SLOT( copyDown() ) );

  // finaly read the options
  readOptions();
}

KProxyDlg::~KProxyDlg()
{
  // now delete everything we allocated before
  delete lb_info;
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

void KProxyDlg::readOptions()
{
  QString port, tmp;
  KURL url;
  
  KConfig *config = KApplication::getKApplication()->getConfig();
  config->setGroup( "Browser Settings/Proxy" );	
  tmp = config->readEntry( "HTTP-Proxy" );
  if( !tmp.isEmpty() ) {
    url = tmp.data();
    port.setNum( url.port() );
    le_http_url->setText( url.host() );
    le_http_port->setText( port.data() ); 
  }
 
  tmp = config->readEntry( "FTP-Proxy" );
  if( !tmp.isEmpty() ) {
    url = tmp.data();
    port.setNum( url.port() );
    le_ftp_url->setText( url.host() );
    le_ftp_port->setText( port.data() ); 
  }
  
  tmp = config->readEntry( "UseProxy" );
  if( tmp == "Yes" ) {
    useProxy = TRUE;
  } else {
    useProxy = FALSE;
  }
  setProxy();
  
  tmp = config->readEntry( "NoProxyFor" );
  le_no_prx->setText( tmp.data() );  
}

void KProxyDlg::copyDown()
{
  le_ftp_url->setText( le_http_url->text() );
  le_ftp_port->setText( le_http_port->text() );
}

void KProxyDlg::setProxy()
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

void KProxyDlg::changeProxy()
{
  useProxy = (! useProxy);
  setProxy();
}

void KProxyDlg::getProxyOpts( struct proxyoptions &proxyopts )
{
  QString url;

  if( useProxy == TRUE ) {
    proxyopts.useProxy = "Yes";
  } else {
    proxyopts.useProxy = "No";
  }
  
  url = le_http_url->text();
  if( url.isEmpty() ) {
    proxyopts.http_proxy = "";
  } else {
    QString httpstr("http://");
    httpstr += le_http_url->text();	// host
    httpstr += ":";
    httpstr += le_http_port->text();	// and port
    proxyopts.http_proxy = httpstr;
  }
  url = le_ftp_url->text();
  if( url.isEmpty() ) {
    proxyopts.ftp_proxy = "";
  } else {
    QString ftpstr("ftp://");
    ftpstr += le_ftp_url->text();	// host
    ftpstr += ":";
    ftpstr += le_ftp_port->text();	// and port
    proxyopts.ftp_proxy = ftpstr;	  
  }
  proxyopts.no_proxy_for = le_no_prx->text();
}

#include "kproxydlg.moc"
