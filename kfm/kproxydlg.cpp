// File kproxydlg.cpp by Lars Hoss ( Lars.Hoss@munich.netsurf.de )

#include <ktablistbox.h>
#include <klocale.h>
#include <kapp.h>
#include <kurl.h>
#include <string.h>
#include "kproxydlg.h"

KProxyDlg::KProxyDlg(QWidget *parent, const char *name, WFlags f)
  : QWidget(parent, name, f)
{
//  lb_info = new QLabel( klocale->translate("You may configure a proxy and port number for each of\nthe internet protocols that KFM supports."), this);
  lb_info = new QLabel( klocale->translate("Don't forget to save your settings!"), this);
  lb_info->setGeometry(20, 20, 360, 30);
  
  cb_useProxy = new QCheckBox( "Use proxy", this );
  cb_useProxy->setGeometry( 20, 70, 80, 30 );
  connect( cb_useProxy, SIGNAL( clicked() ), SLOT( changeProxy() ) );
  
  lb_http_url = new QLabel("HTTP Proxy:", this);
  lb_http_url->setAlignment(AlignRight | AlignVCenter);
  lb_http_url->setGeometry(20, 110, 80, 30);
  le_http_url = new QLineEdit(this);
  le_http_url->setGeometry(110, 110, 105, 30);
  lb_http_port = new QLabel("Port:", this);
  lb_http_port->setAlignment(AlignRight | AlignVCenter);
  lb_http_port->setGeometry(230, 110, 40, 30);
  le_http_port = new QLineEdit(this);
  le_http_port->setGeometry(280, 110, 55, 30);

  lb_ftp_url = new QLabel("FTP Proxy:", this);
  lb_ftp_url->setAlignment(AlignRight | AlignVCenter);
  lb_ftp_url->setGeometry(20, 150, 80, 30);
  le_ftp_url = new QLineEdit(this);
  le_ftp_url->setGeometry(110, 150, 105, 30);
  lb_ftp_port = new QLabel("Port:", this);
  lb_ftp_port->setAlignment(AlignRight | AlignVCenter);
  lb_ftp_port->setGeometry(230, 150, 40, 30);
  le_ftp_port = new QLineEdit(this);
  le_ftp_port->setGeometry(280, 150, 55, 30);

  lb_no_prx = new QLabel(klocale->translate("No Proxy for:"), this);
  lb_no_prx->setAlignment(AlignRight | AlignVCenter);
  lb_no_prx->setGeometry(20, 240, 80, 30);
  le_no_prx = new QLineEdit(this);
  le_no_prx->setGeometry(110, 240, 225, 30);

  QString path;
  QPixmap pixmap;
  pixmap.load( KApplication::kde_toolbardir() + "/down.xpm" ); 
  cp_down = new QPushButton( this );
  cp_down->setPixmap( pixmap );
  cp_down->setGeometry(342, 115, 20, 20);
  connect( cp_down, SIGNAL( clicked() ), SLOT( copyDown() ) );
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

void KProxyDlg::setData(QStrList *strList)
{
  QString port;
  KURL url;

  QString tmp( strList->first() );
  if( tmp == "Yes" ) {
    useProxy = TRUE;
  } else {
    useProxy = FALSE;
  }
  setProxy();
  
  tmp = ( strList->next() );
  if ( ! tmp.isEmpty() ) {
    printf("HTTP Proxy found in kfmrc!\n");
    url = tmp.data();
    le_http_url->setText( url.host() );
    port.setNum ( url.port() );    
    le_http_port->setText( port.data() );
  }
  
  tmp = strList->next();
  if ( ! tmp.isEmpty() ) {
    printf("FTP Proxy found in kfmrc!\n");
    url = tmp.data();
    le_ftp_url->setText( url.host() );
    port.setNum ( url.port() );
    le_ftp_port->setText( port.data() );
  }
  
  le_no_prx->setText(strList->next());
}

static QStrList strList(true);

QStrList KProxyDlg::data() const
{
  strList.clear();
  QString url;
  
  if( useProxy == TRUE ) {
    strList.append( "Yes" );
  } else {
    strList.append( "No" );
  }
  
  url = le_http_url->text();
  if( url.isEmpty() ) {
    printf("No http proxy selected!\n");
    strList.append("");
  } else {
    QString httpstr("http://");
    httpstr += le_http_url->text();	// host
    httpstr += ":";
    httpstr += le_http_port->text();	// and port
    strList.append( httpstr.data() );
  }
  url = le_ftp_url->text();
  if( url.isEmpty() ) {
    printf("No ftp proxy selected!\n");
    strList.append("");
  } else {
    QString ftpstr("ftp://");
    ftpstr += le_ftp_url->text();	// host
    ftpstr += ":";
    ftpstr += le_ftp_port->text();	// and port
    strList.append( ftpstr.data() );	  
  }
  strList.append(le_no_prx->text());
  return strList;
}

#include "kproxydlg.moc"
