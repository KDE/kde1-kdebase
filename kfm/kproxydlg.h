#ifndef __KPROXYDLG_H
#define __KPROXYDLG_H

#include <qdialog.h>
#include <qpushbt.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qlined.h>
#include <qpushbt.h>

class KProxyDlg : public QWidget
{
Q_OBJECT
  public:
    KProxyDlg(QWidget *parent=0, const char *name=0, WFlags f=0);
    ~KProxyDlg();

    void setData(QStrList *strList);
    QStrList data() const;
     
  private:
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
};

#endif // __KPROXYDLG_H
