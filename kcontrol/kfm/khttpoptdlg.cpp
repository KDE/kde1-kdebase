// File khttpoptdlg.cpp by Jacek Konieczny <jajcus@zeus.posl.gliwice.pl>
// Port to KControl by David Faure <faure@kde.org>

#include <qlayout.h> //CT
#include <qradiobt.h>

#include <klocale.h>
#include <kapp.h>
#include <kbuttonbox.h>
#include <kkeydialog.h>
#include "khttpoptdlg.h"


KHTTPOptions::KHTTPOptions(QWidget *parent, const char *name)
  : KConfigWidget(parent, name)
{

  //CT 12Nov1998 layout management
  QGridLayout *lay = new QGridLayout(this,14,5,10,5);
  lay->addRowSpacing(0,10);
  lay->addRowSpacing(3,10);
  lay->addRowSpacing(6,10);
  lay->addRowSpacing(8,10);
  lay->addRowSpacing(10,10);
  lay->addColSpacing(0,10);
  lay->addColSpacing(2,10);
  lay->addColSpacing(4,10);
  
  lay->setRowStretch(0,0);
  lay->setRowStretch(1,0);
  lay->setRowStretch(2,0);
  lay->setRowStretch(3,1);
  lay->setRowStretch(4,0);
  lay->setRowStretch(5,0);
  lay->setRowStretch(6,1);
  lay->setRowStretch(7,0);
  lay->setRowStretch(8,1);
  lay->setRowStretch(9,0);
  lay->setRowStretch(10,1);
  lay->setRowStretch(11,0);
  lay->setRowStretch(12,1);
  lay->setRowStretch(13,10);
  
  lay->setColStretch(0,0);
  lay->setColStretch(1,1);
  lay->setColStretch(2,0);
  lay->setColStretch(3,1);
  lay->setColStretch(4,0);
  //CT
  
  lb_languages = new QLabel(i18n("Accept languages:"), this);
  //CT 12Nov1998 layout management
  lb_languages->adjustSize();
  lb_languages->setMinimumSize(lb_languages->size());
  lay->addWidget(lb_languages,1,1);
  //CT

  le_languages = new QLineEdit(this);
  //CT 12Nov1998 layout management
  le_languages->adjustSize();
  le_languages->setMinimumSize(le_languages->size());
  lay->addMultiCellWidget(le_languages,2,2,1,3);
  //CT
  
  lb_charsets = new QLabel(i18n("Accept character sets:"), this);
  //CT 12Nov1998 layout management
  lb_charsets->adjustSize();
  lb_charsets->setMinimumSize(lb_charsets->size());
  lay->addWidget(lb_charsets,4,1);
  //CT
  le_charsets = new QLineEdit(this);
  //CT 12Nov1998 layout management
  le_charsets->adjustSize();
  le_charsets->setMinimumSize(le_charsets->size());
  lay->addMultiCellWidget(le_charsets,5,5,1,3);
  //CT

  cb_assumeHTML = new QCheckBox( i18n("Assume HTML"), this );
  //CT 12Nov1998 layout management
  cb_assumeHTML->adjustSize();
  cb_assumeHTML->setMinimumSize(cb_assumeHTML->size());
  lay->addWidget(cb_assumeHTML,7,1);

  cb_disableCookies = new QCheckBox( i18n("Disable Cookies"), this );
  cb_disableCookies->adjustSize();
  cb_disableCookies->setMinimumSize(cb_disableCookies->size());
  connect( cb_disableCookies, SIGNAL( clicked() ), this, SLOT( changeCookiesDisabled() ) );
  lay->addWidget(cb_disableCookies,9,1);

  {
    QButtonGroup *bg = new QButtonGroup( i18n("Default accept policy"), this );
    bg1 = bg;
    bg->setExclusive( TRUE );
    QGridLayout *bgLay = new QGridLayout(bg,4,3,10,5);
    bgLay->addRowSpacing(0,10);
    bgLay->addRowSpacing(2,10);
    bgLay->setRowStretch(0,0);
    bgLay->setRowStretch(1,0);

    rb_gbPolicyAccept = new QRadioButton( i18n("Accept"), bg );
    rb_gbPolicyAccept->adjustSize();
    rb_gbPolicyAccept->setMinimumSize(rb_gbPolicyAccept->size());
    bgLay->addWidget(rb_gbPolicyAccept, 1, 0);

    rb_gbPolicyAsk = new QRadioButton( i18n("Ask"), bg );
    rb_gbPolicyAsk->adjustSize();
    rb_gbPolicyAsk->setMinimumSize(rb_gbPolicyAsk->size());
    bgLay->addWidget(rb_gbPolicyAsk, 1, 1);

    rb_gbPolicyReject = new QRadioButton( i18n("Reject"), bg );
    rb_gbPolicyReject->adjustSize();
    rb_gbPolicyReject->setMinimumSize(rb_gbPolicyReject->size());
    bgLay->addWidget(rb_gbPolicyReject, 1, 2);

    lay->addMultiCellWidget(bg,7,9,3,3);
  }

  // CREATE SPLIT LIST BOX
  wList = new KSplitList( this );
  wList->setMinimumHeight(80);
  lay->addWidget( wList, 12, 1 );
                          
  wListLabel = new QLabel( wList, i18n("Domain specific settings:"), this );
  lay->addWidget( wListLabel, 11, 1 );
  wListLabel->setFixedHeight( wListLabel->sizeHint().height() );

  connect( wList, SIGNAL( highlighted( int ) ), SLOT( updateDomain( int ) ) );
  connect( wList, SIGNAL( selected( int ) ), SLOT( updateDomain( int ) ) );
  {
    QButtonGroup *bg = new QButtonGroup( i18n("Change domain accept policy"), this );
    bg2 = bg;
    bg->setExclusive( TRUE );
    QGridLayout *bgLay = new QGridLayout(bg,6,3,10,5);
    bgLay->addRowSpacing(0,10);
    bgLay->addRowSpacing(2,10);
    bgLay->setRowStretch(0,0);
    bgLay->setRowStretch(1,0);
    bgLay->setRowStretch(2,1);
    bgLay->setRowStretch(3,0);
    bgLay->setRowStretch(2,1);
    bgLay->setRowStretch(3,0);

    le_domain = new QLineEdit(bg);
    le_domain->adjustSize();
    le_domain->setMinimumSize(le_domain->size());
    bgLay->addMultiCellWidget(le_domain,1,1,0,2);
              
    rb_domPolicyAccept = new QRadioButton( i18n("Accept"), bg );
    rb_domPolicyAccept->adjustSize();
    rb_domPolicyAccept->setMinimumSize(rb_domPolicyAccept->size());
    bgLay->addWidget(rb_domPolicyAccept, 3, 0);

    rb_domPolicyAsk = new QRadioButton( i18n("Ask"), bg );
    rb_domPolicyAsk->adjustSize();
    rb_domPolicyAsk->setMinimumSize(rb_domPolicyAsk->size());
    bgLay->addWidget(rb_domPolicyAsk, 3, 1);

    rb_domPolicyReject = new QRadioButton( i18n("Reject"), bg );
    rb_domPolicyReject->adjustSize();
    rb_domPolicyReject->setMinimumSize(rb_domPolicyReject->size());
    rb_domPolicyAsk->setChecked( true );
    bgLay->addWidget(rb_domPolicyReject, 3, 2);

    KButtonBox *bbox = new KButtonBox( bg );
    bbox->addStretch( 20 );
        
    b0 = bbox->addButton( "Change" );
    connect( b0, SIGNAL( clicked() ), this, SLOT( changePressed() ) );
                
    bbox->addStretch( 10 );
                    
    b1 = bbox->addButton( "Delete" );
    connect( b1, SIGNAL( clicked() ), this, SLOT( deletePressed() ) );
                            
    bbox->addStretch( 20 );
                                
    bbox->layout();
    bgLay->addMultiCellWidget( bbox, 5,5,0,2);
                                            
    lay->addWidget(bg,11,3);
  }

  lay->activate();
  //CT

  defaultCharsets = QString("utf-8 ")+klocale->charset()+" iso-8859-1";

  setMinimumSize(480,300);

  // finaly read the options
  loadSettings();
}

KHTTPOptions::~KHTTPOptions()
{
  // now delete everything we allocated before
  delete lb_languages;
  delete le_languages;
  delete lb_charsets;
  delete le_charsets;
  delete cb_assumeHTML;
  // time to say goodbye ...
}

enum KCookieAdvice {
    KCookieDunno=0,
    KCookieAccept,
    KCookieReject,
    KCookieAsk
};

static const char *adviceToStr(KCookieAdvice _advice)
{
    switch( _advice )
    {
    case KCookieAccept: return "Accept";
    case KCookieReject: return "Reject";
    case KCookieAsk: return "Ask";
    default: return "Dunno";
    }
}

static KCookieAdvice strToAdvice(const char *_str)
{
    if (!_str)
        return KCookieDunno;
        
    if (strcasecmp(_str, "Accept") == 0)
	return KCookieAccept;
    else if (strcasecmp(_str, "Reject") == 0)
	return KCookieReject;
    else if (strcasecmp(_str, "Ask") == 0)
	return KCookieAsk;

    return KCookieDunno;	
}

static void splitDomainAdvice(const char *configStr, 
                              QString &domain,
                              KCookieAdvice &advice)
{
    QString tmp(configStr);
    int splitIndex = tmp.find(':');
    if ( splitIndex == -1)
    {
        domain = configStr;
        advice = KCookieDunno;
    }
    else
    {
        domain = tmp.left(splitIndex);
        advice = strToAdvice( tmp.mid( splitIndex+1, tmp.length()));
    }
}                            

void KHTTPOptions::removeDomain(const char *domain)
{
    const char *configStr = 0L;
    QString searchFor(domain);
    searchFor += ":";
    
    for( configStr = domainConfig.first();
         configStr != 0;
         configStr = domainConfig.next())
    {
       if (strncasecmp(configStr, searchFor.data(), searchFor.length()) == 0)
       {
           domainConfig.removeRef(configStr);
           return;
       }
    }
}

void KHTTPOptions::changePressed()
{
    const char *domain = le_domain->text();
    const char *advice;

    if (strlen(domain) == 0)
    {
    	// Warning box
        return;
    }

    if (rb_domPolicyAccept->isChecked())
        advice = adviceToStr(KCookieAccept);
    else if (rb_domPolicyReject->isChecked())
        advice = adviceToStr(KCookieReject);
    else
        advice = adviceToStr(KCookieAsk);
    
    QString configStr(domain);
    
    if (configStr[0] != '.')
        configStr = "." + configStr;    

    removeDomain(configStr.data());
    
    configStr += ":";
    
    configStr += advice;

    domainConfig.inSort(configStr.data());
    
    int index = domainConfig.find(configStr.data());

    updateDomainList();
    
    wList->setCurrentItem(index);
}

void KHTTPOptions::deletePressed()
{
    QString domain(le_domain->text());
    
    if (domain[0] != '.')
        domain = "." + domain;

    removeDomain(domain.data());
    updateDomainList();

    le_domain->setText("");
    rb_domPolicyAccept->setChecked( false );
    rb_domPolicyReject->setChecked( false );
    rb_domPolicyAsk->setChecked( true );

    if (wList->count() > 0)
        wList->setCurrentItem(0);
}

void KHTTPOptions::updateDomain(int index)
{
  const char *configStr;
  QString domain;
  KCookieAdvice advice;

  if ((index < 0) || (index >= (int) domainConfig.count()))
      return;
  
  int id = 0;

  for( configStr = domainConfig.first();
       configStr != 0;
       configStr = domainConfig.next())
  {
       if (id == index)
           break; 
       id++;                                                                                                                                 
  }
  if (!configStr)
      return;

  splitDomainAdvice(configStr, domain, advice);

  le_domain->setText(domain);  
  rb_domPolicyAccept->setChecked( advice == KCookieAccept );
  rb_domPolicyReject->setChecked( advice == KCookieReject );
  rb_domPolicyAsk->setChecked( (advice != KCookieAccept) &&
                              (advice != KCookieReject) );
}

void KHTTPOptions::changeCookiesDisabled()
{
  bool enabled = !cb_disableCookies->isChecked();
    
  rb_gbPolicyAccept->setEnabled( enabled);
  rb_gbPolicyReject->setEnabled( enabled);
  rb_gbPolicyAsk->setEnabled( enabled );

  rb_domPolicyAccept->setEnabled( enabled);
  rb_domPolicyReject->setEnabled( enabled);
  rb_domPolicyAsk->setEnabled( enabled );
  bg1->setEnabled( enabled );
  bg2->setEnabled( enabled );

  b0->setEnabled( enabled );
  b1->setEnabled( enabled );

  wListLabel->setEnabled( enabled );    
  wList->setEnabled( enabled );    
  le_domain->setEnabled( enabled );
}

void KHTTPOptions::updateDomainList()
{
  wList->setAutoUpdate(false);
  wList->clear();

  int id = 0;

  for( const char *domain = domainConfig.first();
       domain != 0;
       domain = domainConfig.next())
  {
       KSplitListItem *sli = new KSplitListItem( domain, id);
       connect( wList, SIGNAL( newWidth( int ) ),
	       sli, SLOT( setWidth( int ) ) );
       sli->setWidth(wList->width());
       wList->inSort( sli );
       id++;                                                                                                                                 
  }
  wList->setAutoUpdate(true);
  wList->update();
}

void KHTTPOptions::loadSettings()
{
  QString tmp;
  g_pConfig->setGroup( "Browser Settings/HTTP" );	
  tmp = g_pConfig->readEntry( "AcceptLanguages",klocale->languages());
  le_languages->setText( tmp );
  tmp = g_pConfig->readEntry( "AcceptCharsets",defaultCharsets);
  le_charsets->setText( tmp );

  cb_assumeHTML->setChecked(g_pConfig->readBoolEntry( "AssumeHTML", false ));

  tmp = g_pConfig->readEntry("CookieGlobalAdvice", "Ask");
  KCookieAdvice globalAdvice = strToAdvice(tmp.data());

  cb_disableCookies->setChecked( !g_pConfig->readBoolEntry( "Cookies", true ));

  rb_gbPolicyAccept->setChecked( globalAdvice == KCookieAccept );
  rb_gbPolicyReject->setChecked( globalAdvice == KCookieReject );
  rb_gbPolicyAsk->setChecked( (globalAdvice != KCookieAccept) &&
                              (globalAdvice != KCookieReject) );

  (void) g_pConfig->readListEntry("CookieDomainAdvice", domainConfig);

  updateDomainList();
}

void KHTTPOptions::saveSettings()
{
  const char *advice;
  g_pConfig->setGroup( "Browser Settings/HTTP" );	
  g_pConfig->writeEntry( "AcceptLanguages", le_languages->text());
  g_pConfig->writeEntry( "AcceptCharsets", le_charsets->text());
  g_pConfig->writeEntry( "AssumeHTML",cb_assumeHTML->isChecked());
 
  g_pConfig->writeEntry( "Cookies", !cb_disableCookies->isChecked() );

  if (rb_gbPolicyAccept->isChecked())
      advice = adviceToStr(KCookieAccept);
  else if (rb_gbPolicyReject->isChecked())
      advice = adviceToStr(KCookieReject);
  else
      advice = adviceToStr(KCookieAsk);
  g_pConfig->writeEntry("CookieGlobalAdvice", advice);
  g_pConfig->writeEntry("CookieDomainAdvice", domainConfig);

  g_pConfig->sync();
}

void KHTTPOptions::applySettings()
{
  saveSettings();
}

void KHTTPOptions::defaultSettings()
{
  le_languages->setText( klocale->languages() );
  le_charsets->setText( defaultCharsets );
  cb_assumeHTML->setChecked( false );

  cb_disableCookies->setChecked( false );

  rb_gbPolicyAccept->setChecked( false );
  rb_gbPolicyReject->setChecked( false );
  rb_gbPolicyAsk->setChecked( true );
  domainConfig.clear();
  updateDomainList();
  le_domain->setText("");  
  rb_domPolicyAccept->setChecked( false );
  rb_domPolicyReject->setChecked( false );
  rb_domPolicyAsk->setChecked( true );
}

#include "khttpoptdlg.moc"
