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
#include <qbttngrp.h>
 
#include <kcontrol.h>
#include <kkeydialog.h>
 
extern KConfigBase *g_pConfig;

/**
*  Dialog for configuring HTTP Options like charset and language negotiation
*  and assuming that file got from HTTP is HTML if no Content-Type is given
*/
class KHTTPOptions : public KConfigWidget
{
Q_OBJECT
  public:
    KHTTPOptions(QWidget *parent = 0L, const char *name = 0L);
    ~KHTTPOptions();

    virtual void loadSettings();
    virtual void saveSettings();
    virtual void applySettings();
    virtual void defaultSettings();
    
  private:
    void removeDomain(const char *domain);
    void updateDomainList();
    
    QLabel     *wListLabel;
    KSplitList *wList;

    // Acceptable languages "LANG" - locale selected languages
    QLabel *lb_languages;	
    QLineEdit *le_languages;	

    // Acceptable charsets "CHARSET" - locale selected charset
    QLabel *lb_charsets;	
    QLineEdit *le_charsets;	

    // Assume HTML if mime type is not known
    QCheckBox *cb_assumeHTML;

    // Cookies disabled
    QCheckBox    *cb_disableCookies;


    QButtonGroup *bg1;
    // Global cookie policies
    QRadioButton *rb_gbPolicyAccept;
    QRadioButton *rb_gbPolicyAsk;
    QRadioButton *rb_gbPolicyReject;

    QButtonGroup *bg2;
    // Domain cookie policies
    QLineEdit *le_domain;	
    QRadioButton *rb_domPolicyAccept;
    QRadioButton *rb_domPolicyAsk;
    QRadioButton *rb_domPolicyReject;
    QPushButton  *b0;
    QPushButton  *b1;

    QString defaultCharsets;
    
    QStrList domainConfig;
public slots:
    void changeCookiesDisabled();
    void changePressed();
    void deletePressed();
    void updateDomain(int id);
};

#endif // __KPROXYDLG_H
