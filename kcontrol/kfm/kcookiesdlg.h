// kcookiesdlg.h - Cookies configuration
//
// First version of cookies configuration by Waldo Bastian <bastian@kde.org>
// This dialog box created by David Faure <faure@kde.org>

#ifndef __KCOOKIESDLG_H
#define __KCOOKIESDLG_H

#include <qdialog.h>
#include <qpushbt.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qlined.h>
#include <qpushbt.h>
#include <qchkbox.h>
#include <qbttngrp.h>

#include <kkeydialog.h> // for ksplitlist
#include <kcontrol.h>
 
extern KConfigBase *g_pConfig;

class KCookiesOptions : public KConfigWidget
{
Q_OBJECT
  public:
    KCookiesOptions(QWidget *parent = 0L, const char *name = 0L);
    ~KCookiesOptions();

    virtual void loadSettings();
    virtual void saveSettings();
    virtual void applySettings();
    virtual void defaultSettings();
    
  private:
    void removeDomain(const char *domain);
    void updateDomainList();
    
    QLabel     *wListLabel;
    KSplitList *wList;

    // Cookies enabled
    QCheckBox    *cb_enableCookies;

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

    QStrList domainConfig;
public slots:
    void changeCookiesEnabled();
    void changePressed();
    void deletePressed();
    void updateDomain(int id);
};

#endif // __KCOOKIESDLG_H
