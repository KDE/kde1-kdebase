// File khttpoptdlg.cpp by Jacek Konieczny ( jajcus@zeus.posl.gliwice.pl )

#include <ktablistbox.h>
#include <klocale.h>
#include <kapp.h>
#include <string.h>
#include "khttpoptdlg.h"

KHTTPOptionsDlg::KHTTPOptionsDlg(QWidget *parent, const char *name, WFlags f)
  : QWidget(parent, name, f)
{
  
  lb_languages = new QLabel(i18n("Accept languages:"), this);
  lb_languages->setGeometry(20, 10, 200, 30);
  le_languages = new QLineEdit(this);
  le_languages->setGeometry(20, 40, 200, 30);
  
  lb_charsets = new QLabel(i18n("Accept character sets:"), this);
  lb_charsets->setGeometry(20, 80, 200, 30);
  le_charsets = new QLineEdit(this);
  le_charsets->setGeometry(20, 110, 200, 30);
  
  cb_assumeHTML = new QCheckBox( i18n("Assume HTML"), this );
  cb_assumeHTML->setGeometry( 20, 150, 200, 30 );
  
  // finaly read the options
  readOptions();
}

KHTTPOptionsDlg::~KHTTPOptionsDlg()
{
  // now delete everything we allocated before
  delete lb_languages;
  delete le_languages;
  delete lb_charsets;
  delete le_charsets;
  delete cb_assumeHTML;
  // time to say goodbye ...
}

void KHTTPOptionsDlg::readOptions()
{
  QString tmp;
  KConfig *config = kapp->getConfig();
  config->setGroup( "Browser Settings/HTTP" );	
  tmp = config->readEntry( "AcceptLanguages",klocale->languages());
  le_languages->setText( tmp );
  tmp = config->readEntry( "AcceptCharsets",QString("utf-8 ")
                           +klocale->charset()+"iso-8859-1");
  le_charsets->setText( tmp );
   
  cb_assumeHTML->setChecked(config->readBoolEntry( "AssumeHTML",false ));
}

void KHTTPOptionsDlg::getHTTPOpts( struct httpoptions &httpopts )
{

  httpopts.assumeHTML = cb_assumeHTML->isChecked();
  httpopts.languages = le_languages->text();
  httpopts.charsets = le_charsets->text();
}

#include "khttpoptdlg.moc"
