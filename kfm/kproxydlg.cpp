#include <ktablistbox.h>
#include <klocale.h>
#include <kapp.h>
#include "kproxydlg.h"

KProxyDlg::KProxyDlg(QWidget *parent=0, const char *name=0, WFlags f)
  : QWidget(parent, name, f)
{
//  lb_info = new QLabel( klocale->translate("You may configure a proxy and port number for each of\nthe internet protocols that KFM supports."), this);
  lb_info = new QLabel( klocale->translate("This feature is not implemented yet!"), this);
  lb_info->setGeometry(20, 20, 360, 30);
  
  lb_http_url = new QLabel("HTTP Proxy:", this);
  lb_http_url->setAlignment(AlignRight | AlignVCenter);
  lb_http_url->setGeometry(20, 70, 80, 30);
  le_http_url = new QLineEdit(this);
  le_http_url->setGeometry(110, 70, 105, 30);
  lb_http_port = new QLabel("Port:", this);
  lb_http_port->setAlignment(AlignRight | AlignVCenter);
  lb_http_port->setGeometry(230, 70, 40, 30);
  le_http_port = new QLineEdit(this);
  le_http_port->setGeometry(280, 70, 55, 30);

  lb_ftp_url = new QLabel("FTP Proxy:", this);
  lb_ftp_url->setAlignment(AlignRight | AlignVCenter);
  lb_ftp_url->setGeometry(20, 110, 80, 30);
  le_ftp_url = new QLineEdit(this);
  le_ftp_url->setGeometry(110, 110, 105, 30);
  lb_ftp_port = new QLabel("Port:", this);
  lb_ftp_port->setAlignment(AlignRight | AlignVCenter);
  lb_ftp_port->setGeometry(230, 110, 40, 30);
  le_ftp_port = new QLineEdit(this);
  le_ftp_port->setGeometry(280, 110, 55, 30);
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
  // time to say goodbye ...
}

void KProxyDlg::setData(QStrList *strList)
{
  // printf("KProxyDlg: strList has %d entries\n", strList->count());
  le_http_url->setText(strList->first());
  le_http_port->setText(strList->next());
  le_ftp_url->setText(strList->next());
  le_ftp_port->setText(strList->next());
}

static QStrList strList(true);

QStrList KProxyDlg::data() const
{
  strList.clear();
  strList.append(le_http_url->text());
  // printf("KProxyDlg: appending %s\n", le_http_url->text());
  strList.append(le_http_port->text());
  strList.append(le_ftp_url->text());
  strList.append(le_ftp_port->text());
  return strList;
}

#include "kproxydlg.moc"
