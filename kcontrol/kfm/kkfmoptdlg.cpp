#include <qlayout.h> //CT
#include <kapp.h>
#include <klocale.h>
#include "kkfmoptdlg.h"


KKFMOptDlg::KKFMOptDlg(QWidget *parent, const char *name, WFlags f)
  : QDialog(parent, name, true, f)
{
  //CT 12Nov1998 layout management
  QGridLayout *lay = new QGridLayout(this, 4,6,5);
  lay->addRowSpacing(0,10);
  lay->addRowSpacing(3,10);
  lay->addColSpacing(0,10);
  lay->addColSpacing(1,72);
  lay->addColSpacing(3,72);
  lay->addColSpacing(4,72);
  lay->addColSpacing(5,10);

  lay->setRowStretch(0,0);
  lay->setRowStretch(1,1);
  lay->setRowStretch(2,0);
  lay->setRowStretch(3,0);
  lay->setColStretch(0,0);
  lay->setColStretch(1,0);
  lay->setColStretch(2,1);
  lay->setColStretch(3,0);
  lay->setColStretch(4,0);
  lay->setColStretch(5,0);

  // KTabCtl
  tabDlg = new KTabCtl(this);
  
  prxDlg = new KProxyDlg(tabDlg);
  tabDlg->addTab(prxDlg, klocale->translate("&Proxy"));
  
  httpDlg = new KHTTPOptionsDlg(tabDlg);
  tabDlg->addTab(httpDlg, klocale->translate("&HTTP"));

  usrDlg = new UserAgentDialog(tabDlg);
  tabDlg->addTab(usrDlg, klocale->translate("User &Agent"));

  fontDlg = new KFontOptions(tabDlg);
  tabDlg->addTab(fontDlg, klocale->translate("&Fonts"));

  colorDlg = new KColorOptions(tabDlg);
  tabDlg->addTab(colorDlg, klocale->translate("&Colors"));

  miscDlg = new KMiscOptions(tabDlg);
  tabDlg->addTab(miscDlg, klocale->translate("&Other"));

  //CT 12Nov1998 layout management
  prxDlg->adjustSize();
  prxDlg->setMinimumSize(prxDlg->size());
  httpDlg->adjustSize();
  httpDlg->setMinimumSize(httpDlg->size());
  usrDlg->adjustSize();
  usrDlg->setMinimumSize(usrDlg->size());
  fontDlg->adjustSize();
  fontDlg->setMinimumSize(fontDlg->size());
  colorDlg->adjustSize();
  colorDlg->setMinimumSize(colorDlg->size());
  miscDlg->adjustSize();
  miscDlg->setMinimumSize(miscDlg->size());

  tabDlg->adjustSize();
  tabDlg->setMinimumSize(tabDlg->size());

  lay->addMultiCellWidget(tabDlg,1,1,1,4);

  setMinimumWidth( tabDlg->width()+20 );
  //CT

  // help button
  help = new QPushButton(klocale->translate("Help"), this);
  help->adjustSize();
  help->setMinimumSize(help->size());
  lay->addWidget(help,3,1);
  connect(help, SIGNAL(clicked()), SLOT(helpShow()));

  // ok button
  ok = new QPushButton(klocale->translate("OK"), this);
  ok->adjustSize();
  ok->setMinimumSize(ok->size());
  lay->addWidget(ok,3,3);
  ok->setDefault(TRUE);
  connect(ok, SIGNAL(clicked()), SLOT(accept()));
  
  // cancel button
  cancel = new QPushButton(klocale->translate("Cancel"), this);
  cancel->adjustSize();
  cancel->setMinimumSize(cancel->size());

  lay->addWidget(cancel,3,4);

  lay->activate();
  connect(cancel, SIGNAL(clicked()), SLOT(reject()));

  // my name is
  setCaption(klocale->translate("KFM Preferences Settings"));

}

KKFMOptDlg::~KKFMOptDlg()
{
  // now delete everything we allocated before
  delete help;
  delete ok;
  delete cancel;
  delete usrDlg;
  delete prxDlg;
  delete httpDlg;
  delete tabDlg;
}

void KKFMOptDlg::helpShow()
{
  kapp->invokeHTMLHelp(0, 0);
}

void KKFMOptDlg::setUsrAgentData(QStrList *strList)
// forward data to UserAgentDialog
{
  usrDlg->setData(strList);
}

QStrList KKFMOptDlg::dataUsrAgent() const
{
  return(usrDlg->data()); 
}

void KKFMOptDlg::proxyData( struct proxyoptions &proxyopts )
{
  prxDlg->getProxyOpts( proxyopts );
}

void KKFMOptDlg::fontData(struct fontoptions& fntopts){

  fontDlg->getFontOpts(fntopts);

}

void KKFMOptDlg::colorData(struct coloroptions& coloropts){

  colorDlg->getColorOpts(coloropts);

}

void KKFMOptDlg::miscData(struct rootoptions& miscopts){

  miscDlg->getMiscOpts(miscopts);

}

void KKFMOptDlg::httpData( struct httpoptions &httpopts )
{
  httpDlg->getHTTPOpts( httpopts );
}

#include "kkfmoptdlg.moc"
