/*
 * $Id$
 */

#include "useragentdlg.h"

#include <kapp.h>
#include <klocale.h>

#include <qlabel.h>
#include <qlined.h>
#include <qlistbox.h>
#include <qpushbt.h>
#include <qlayout.h> //CT
UserAgentDialog::UserAgentDialog( QWidget * parent, 
								  const char * name, 
								  WFlags f ) :
  QWidget( parent, name, f )
{

  //CT 12Nov1998 layout management
  QGridLayout *lay = new QGridLayout(this,7,5,10,5);
  lay->addRowSpacing(0,10);
  lay->addRowSpacing(3,25);
  lay->addRowSpacing(6,10);
  lay->addColSpacing(0,10);
  lay->addColSpacing(4,10);

  lay->setRowStretch(0,0);
  lay->setRowStretch(1,0);
  lay->setRowStretch(2,0);
  lay->setRowStretch(3,0);
  lay->setRowStretch(4,0);
  lay->setRowStretch(5,1);
  lay->setRowStretch(6,0);

  lay->setColStretch(0,0);
  lay->setColStretch(1,0);
  lay->setColStretch(2,1);
  lay->setColStretch(3,1);
  lay->setColStretch(4,0);
  //CT

  onserverLA = new QLabel( klocale->translate( "On server:" ), this );
  //CT  onserverLA->setGeometry( 10, 20, 60, 30 );
  onserverLA->setAlignment( AlignRight|AlignVCenter );
  //CT 12Nov1998 layout management
  onserverLA->adjustSize();
  onserverLA->setMinimumSize(onserverLA->size());
  lay->addWidget(onserverLA,1,1);
  //CT

  onserverED = new QLineEdit( this );
  //CT  onserverED->setGeometry( 80, 20, 140, 30 );
  //CT 12Nov1998 layout management
  onserverED->adjustSize();
  onserverED->setMinimumSize(onserverED->size());
  lay->addWidget(onserverED,1,2);
  //CT

  connect( onserverED, SIGNAL( textChanged( const char* ) ),
		   SLOT( textChanged( const char* ) ) );
  connect( onserverED, SIGNAL( returnPressed() ),
		   SLOT( returnPressed() ) );

  loginasLA = new QLabel( klocale->translate( "login as:" ), this );
  //CT  loginasLA->setGeometry( 10, 60, 60, 30 );
  loginasLA->setAlignment( AlignRight|AlignVCenter );
  //CT 12Nov1998 layout management
  loginasLA->adjustSize();
  loginasLA->setMinimumSize(loginasLA->size());
  lay->addWidget(loginasLA,2,1);
  //CT

  loginasED = new QLineEdit( this );
  //CT  loginasED->setGeometry( 80, 60, 140, 30 );
  //CT 12Nov1998 layout management
  loginasED->adjustSize();
  loginasED->setMinimumSize(loginasED->size());
  lay->addWidget(loginasED,2,2);
  //CT

  connect( loginasED, SIGNAL( textChanged( const char* ) ),
		   SLOT( textChanged( const char* ) ) );
  connect( loginasED, SIGNAL( returnPressed() ),
		   SLOT( returnPressed() ) );

  addPB = new QPushButton( klocale->translate( "Add" ), this );
  //CT  addPB->setGeometry( 230, 20, 100, 30 );
  //CT 12Nov1998 layout management
  addPB->adjustSize();
  addPB->setMinimumSize(addPB->size());
  lay->addWidget(addPB,1,3);
  //CT

  addPB->setEnabled( false );
  connect( addPB, SIGNAL( clicked() ), SLOT( addClicked() ) );
  
  deletePB = new QPushButton( klocale->translate( "Delete" ), this );
  //CT  deletePB->setGeometry( 230, 60, 100, 30 );
  //CT 12Nov1998 layout management
  deletePB->adjustSize();
  deletePB->setMinimumSize(deletePB->size());
  lay->addWidget(deletePB,2,3);
  //CT

  deletePB->setEnabled( false );
  connect( deletePB, SIGNAL( clicked() ), SLOT( deleteClicked() ) );

  bindingsLA = new QLabel( klocale->translate( "Known bindings:" ), this );
  //CT  bindingsLA->setGeometry( 60, 110, 200, 30 );
  //CT 12Nov1998 layout management
  bindingsLA->adjustSize();
  bindingsLA->setMinimumSize(bindingsLA->size());
  lay->addMultiCellWidget(bindingsLA,4,4,2,3);
  //CT

  bindingsLB = new QListBox( this );
  //CT  bindingsLB->setGeometry( 60, 140, 210, 150 );
  //CT 12Nov1998 layout management
  bindingsLB->adjustSize();
  bindingsLB->setMinimumSize(bindingsLB->size());
  lay->addMultiCellWidget(bindingsLB,5,5,2,3);

  lay->activate();
  //CT

  bindingsLB->setMultiSelection( false );
  bindingsLB->setScrollBar( true );
  connect( bindingsLB, SIGNAL( highlighted( const char* ) ),
		   SLOT( listboxHighlighted( const char* ) ) );

/*-> removed, because no longer used
  okPB = new QPushButton( klocale->translate( "OK" ), this );
  okPB->setGeometry( 10, 310, 100, 30 );
  okPB->setDefault( true );
  connect( okPB, SIGNAL( clicked() ), SLOT( accept() ) );

  cancelPB = new QPushButton( klocale->translate( "Cancel" ), this );
  cancelPB->setGeometry( 120, 310, 100, 30 );
  connect( cancelPB, SIGNAL( clicked() ), SLOT( reject() ) );

  helpPB = new QPushButton( klocale->translate( "Help" ), this );
  helpPB->setGeometry( 230, 310, 100, 30 );
  connect( helpPB, SIGNAL( clicked() ), SLOT( helpClicked() ) );
*/

//-> was: setCaption( klocale->translate( "Set UserAgent" ) ); 
}


UserAgentDialog::~UserAgentDialog()
{
  /*  delete onserverLA;
  delete onserverED;
  delete loginasLA;
  delete loginasED;
  delete addPB;
  delete deletePB;
  delete bindingsLB;*/
//->  was: delete okPB; <- It crashes kfm with a nice seg fault !!
//->  was: delete cancelPB;
//->  was: delete helpPB;
}


void UserAgentDialog::setData( QStrList* strlist )
{
  bindingsLB->insertStrList( strlist );
}

static QStrList strlist( true );

QStrList UserAgentDialog::data() const
{
  strlist.clear();
  for( uint i = 0; i < bindingsLB->count(); i++ )
	strlist.append( bindingsLB->text( i ) );

  return strlist;
}


void UserAgentDialog::helpClicked()
{
}


void UserAgentDialog::textChanged( const char* )
{
  const char *login = loginasED->text();
  const char *server = onserverED->text();

  if( login && login[0] && server && server[0] )
	addPB->setEnabled( true );
  else
	addPB->setEnabled( false );

  deletePB->setEnabled( false );
}


void UserAgentDialog::returnPressed()
{
  const char *login = loginasED->text();
  const char *server = onserverED->text();

  if( login && login[0] && server && server[0] )
	{
	  QString text = server;
	  text += ':';
	  text += login;
	  bindingsLB->insertItem( new QListBoxText( text.data() ) );
	}
}


void UserAgentDialog::addClicked()
{
  // no need to check if the fields contain text, the add button is only
  // enabled if they do

  QString text = onserverED->text();
  text += ':';
  text += loginasED->text();
  bindingsLB->insertItem( new QListBoxText( text.data() ) );
  onserverED->setText( "" );
  loginasED->setText( "" );
}


void UserAgentDialog::deleteClicked()
{
  if( bindingsLB->count() )
	bindingsLB->removeItem( highlighted_item );
}


void UserAgentDialog::listboxHighlighted( const char* _itemtext )
{
  QString itemtext( _itemtext );
  int colonpos = itemtext.find( ':' );
  onserverED->setText( itemtext.left( colonpos ) );
  loginasED->setText( itemtext.right( itemtext.length() - colonpos - 1) );
  deletePB->setEnabled( true );
  addPB->setEnabled( false );

  highlighted_item = bindingsLB->currentItem();
}



#include "useragentdlg.moc"

/*
 * $Log$
 * Revision 1.6  1998/06/27 09:20:08  kalle
 * Fixing four hundred Makefiles for KDE 1.0, vol. 3
 * Bumped package version number to 1.0pre
 * Improved RPM spec file
 *
 * Revision 1.5  1998/03/04 05:08:23  wuebben
 * Bernd: Made kfm quite a bit more configurable
 * 1.) bg,link,vlink,text colors can be configured
 * 2.) Fonts can be configured
 * 3.) Whether the cursor changes over a link can be configured
 *
 * TODO: Background pixmaps, linedit colors,perhaps link cursor configurable
 *
 * Note: Most of the code is form kdehelp, so things are hopefully OK. However
 * kfm has become a somewhat complex beast. Please test as much as possible;-)
 *
 * Revision 1.4  1998/01/10 01:38:31  torben
 * Torben: Bug fixes ( see CHANGES )
 *
 * Revision 1.3  1997/12/20 01:46:09  hoss
 * *** empty log message ***
 *
 * Revision 1.2  1997/11/24 21:31:59  kulow
 * I would like to remind everyone, not to use default arguments in the implementation ;)
 *
 */
