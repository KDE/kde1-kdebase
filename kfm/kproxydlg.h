#ifndef __KPROXYDLG_H
#define __KPROXYDLG_H

#include <qdialog.h>
#include <qpushbt.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qlined.h>
#include <qpushbt.h>
#include <qchkbox.h>

struct proxyoptions {
  QString useProxy; 		// shall we use proxy?
  QString ftp_proxy;		// url of the ftp proxy
  QString http_proxy;		// url of the http proxy
  QString no_proxy_for;		// do not use proxy for these urls
  QString proxy_login;		// proxy login
  QString proxy_pass;		//   and password
};

class KProxyDlg : public QWidget
{
Q_OBJECT
  public:
    KProxyDlg(QWidget *parent=0, const char *name=0, WFlags f=0);
    ~KProxyDlg();

    void getProxyOpts( struct proxyoptions &proxyopts );
     
  private:
    bool useProxy;
    
    // a little information for the user
    QLabel *lb_info;
    
    // ftp proxy fields
    QLabel *lb_ftp_url;		// label ftp url
    QLineEdit *le_ftp_url;	// lineedit ftp url
    QLabel *lb_ftp_port;	// and so on :)
    QLineEdit *le_ftp_port;

    // http proxy fields
    QLabel *lb_http_url;
    QLineEdit *le_http_url;
    QLabel *lb_http_port;
    QLineEdit *le_http_port;  

    // "no proxy for" fields
    QLabel *lb_no_prx;
    QLineEdit *le_no_prx;

    // copy down butto
    QPushButton *cp_down;
    // use proxy checker
    QCheckBox *cb_useProxy;

    void setProxy();
    void readOptions();
    
  public slots:
    void copyDown();		// use the http setting for all services
    void changeProxy();
};

#endif // __KPROXYDLG_H
