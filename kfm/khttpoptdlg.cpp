// File khttpoptdlg.cpp by Jacek Konieczny ( jajcus@zeus.posl.gliwice.pl )

#include <qlayout.h> //CT

#include <ktablistbox.h>
#include <klocale.h>
#include <kapp.h>
#include <string.h>
#include "khttpoptdlg.h"

KHTTPOptionsDlg::KHTTPOptionsDlg(QWidget *parent, const char *name, WFlags f)
  : QWidget(parent, name, f)
{

  //CT 12Nov1998 layout management
  QGridLayout *lay = new QGridLayout(this,9,3,10,5);
  lay->addRowSpacing(0,10);
  lay->addRowSpacing(3,10);
  lay->addRowSpacing(6,10);
  lay->addRowSpacing(8,10);
  lay->addColSpacing(0,10);
  lay->addColSpacing(2,10);
  
  lay->setRowStretch(0,0);
  lay->setRowStretch(1,0);
  lay->setRowStretch(2,0);
  lay->setRowStretch(3,1);
  lay->setRowStretch(4,0);
  lay->setRowStretch(5,0);
  lay->setRowStretch(6,1);
  lay->setRowStretch(7,1);
  lay->setRowStretch(8,0);
  
  lay->setColStretch(0,0);
  lay->setColStretch(1,1);
  lay->setColStretch(2,0);
  //CT
  
  lb_languages = new QLabel(i18n("Accept languages:"), this);
  //CT  lb_languages->setGeometry(20, 10, 200, 30);
  //CT 12Nov1998 layout management
  lb_languages->adjustSize();
  lb_languages->setMinimumSize(lb_languages->size());
  lay->addWidget(lb_languages,1,1);
  //CT

  le_languages = new QLineEdit(this);
  //CT  le_languages->setGeometry(20, 40, 200, 30);
  //CT 12Nov1998 layout management
  le_languages->adjustSize();
  le_languages->setMinimumSize(le_languages->size());
  lay->addWidget(le_languages,2,1);
  //CT
  
  lb_charsets = new QLabel(i18n("Accept character sets:"), this);
  //CT  lb_charsets->setGeometry(20, 80, 200, 30);
  //CT 12Nov1998 layout management
  lb_charsets->adjustSize();
  lb_charsets->setMinimumSize(lb_charsets->size());
  lay->addWidget(lb_charsets,4,1);
  //CT
  le_charsets = new QLineEdit(this);
  //CT  le_charsets->setGeometry(20, 110, 200, 30);
  //CT 12Nov1998 layout management
  le_charsets->adjustSize();
  le_charsets->setMinimumSize(le_charsets->size());
  lay->addWidget(le_charsets,5,1);
  //CT

  cb_assumeHTML = new QCheckBox( i18n("Assume HTML"), this );
  //CT  cb_assumeHTML->setGeometry( 20, 150, 200, 30 );
  //CT 12Nov1998 layout management
  cb_assumeHTML->adjustSize();
  cb_assumeHTML->setMinimumSize(cb_assumeHTML->size());
  lay->addWidget(cb_assumeHTML,7,1);

  lay->activate();
  //CT

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
