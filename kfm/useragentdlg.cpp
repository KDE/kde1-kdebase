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

UserAgentDialog::UserAgentDialog( QWidget * parent, 
								  const char * name, 
								  WFlags f ) :
  QWidget( parent, name, f )
{
  onserverLA = new QLabel( klocale->translate( "On server:" ), this );
  onserverLA->setGeometry( 10, 20, 60, 30 );
  onserverLA->setAlignment( AlignRight|AlignVCenter );

  onserverED = new QLineEdit( this );
  onserverED->setGeometry( 80, 20, 140, 30 );
  connect( onserverED, SIGNAL( textChanged( const char* ) ),
		   SLOT( textChanged( const char* ) ) );
  connect( onserverED, SIGNAL( returnPressed() ),
		   SLOT( returnPressed() ) );

  loginasLA = new QLabel( klocale->translate( "login as:" ), this );
  loginasLA->setGeometry( 10, 60, 60, 30 );
  loginasLA->setAlignment( AlignRight|AlignVCenter );

  loginasED = new QLineEdit( this );
  loginasED->setGeometry( 80, 60, 140, 30 );
  connect( loginasED, SIGNAL( textChanged( const char* ) ),
		   SLOT( textChanged( const char* ) ) );
  connect( loginasED, SIGNAL( returnPressed() ),
		   SLOT( returnPressed() ) );

  addPB = new QPushButton( klocale->translate( "Add" ), this );
  addPB->setGeometry( 230, 20, 100, 30 );
  addPB->setEnabled( false );
  connect( addPB, SIGNAL( clicked() ), SLOT( addClicked() ) );
  
  deletePB = new QPushButton( klocale->translate( "Delete" ), this );
  deletePB->setGeometry( 230, 60, 100, 30 );
  deletePB->setEnabled( false );
  connect( deletePB, SIGNAL( clicked() ), SLOT( deleteClicked() ) );

  bindingsLA = new QLabel( klocale->translate( "Known bindings:" ), this );
  bindingsLA->setGeometry( 60, 110, 100, 30 );

  bindingsLB = new QListBox( this );
  bindingsLB->setGeometry( 60, 140, 210, 150 );
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
  delete onserverLA;
  delete onserverED;
  delete loginasLA;
  delete loginasED;
  delete addPB;
  delete deletePB;
  delete bindingsLB;
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
 * Revision 1.3  1997/12/20 01:46:09  hoss
 * *** empty log message ***
 *
 * Revision 1.2  1997/11/24 21:31:59  kulow
 * I would like to remind everyone, not to use default arguments in the implementation ;)
 *
 * Revision 1.1  1997/11/21 08:07:31  kalle
 * Make ourselves known via the UserAgent line.
 *
 */
