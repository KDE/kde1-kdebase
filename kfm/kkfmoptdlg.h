#ifndef __KOPTTAB_H
#define __KOPTTAB_H

#include <qdialog.h>
#include <qtabdlg.h>
#include <ktabctl.h>
#include "kproxydlg.h"
#include "useragentdlg.h"
#include "htmlopts.h"
#include "khttpoptdlg.h"

class KKFMOptDlg : public QDialog
{
  Q_OBJECT
  public:
    KKFMOptDlg(QWidget *parent=0, const char *name=0, WFlags f=0);
    ~KKFMOptDlg();

    // methods to access data
    void setUsrAgentData(QStrList *strList);
    QStrList dataUsrAgent() const;
    void proxyData( struct proxyoptions &proxyopts );
    void fontData(struct fontoptions& fontopts);
    void colorData(struct coloroptions& coloropts);
    void miscData(struct rootoptions& miscopts);
    void httpData( struct httpoptions &httpopts );

  private:
    QPushButton *help;
    QPushButton *ok;
    QPushButton *cancel;
    KTabCtl     *tabDlg;
    KProxyDlg   *prxDlg;
    KFontOptions   *fontDlg;
    KColorOptions   *colorDlg;
    KMiscOptions   *miscDlg;
    UserAgentDialog *usrDlg;
    KHTTPOptionsDlg *httpDlg;
    
  public slots:
    void helpShow();

};

#endif // __KOPTTAB_H
