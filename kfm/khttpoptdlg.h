// khttpoptdlg.h - extra HTTP configuration by Jacek Konieczy <jajcus@zeus.polsl.gliwice.pl>
#ifndef __KHTTPOPTDLG_H
#define __KHTTPOPTDLG_H

#include <qdialog.h>
#include <qpushbt.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qlined.h>
#include <qpushbt.h>
#include <qchkbox.h>

struct httpoptions {
  QString languages; 		// Acceptable languages "LANG" - locale selected languages
  QString charsets;		// Acceptable charsets "CHARSET" - locale selected charset
  bool assumeHTML;		// Assume HTML if mime type is not known
};

/**
*  Dialog for configuring HTTP Options like charset and language negotiation  
*  and assuming that file got from HTTP is HTML if no Content-Type is given
*/
class KHTTPOptionsDlg : public QWidget
{
Q_OBJECT
  public:
    KHTTPOptionsDlg(QWidget *parent=0, const char *name=0, WFlags f=0);
    ~KHTTPOptionsDlg();

    void getHTTPOpts( struct httpoptions &httpopts );
     
  private:
    
    // Languages
    QLabel *lb_languages;	
    QLineEdit *le_languages;	

    // charsets
    QLabel *lb_charsets;	
    QLineEdit *le_charsets;	

    // Assume HTML checker
    QCheckBox *cb_assumeHTML;

    void readOptions();
};

#endif // __KPROXYDLG_H
