
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <klocale.h>
#include <kapp.h>
#include <kbind.h>
#include <klined.h>

#include "kfmdlg.h"
#include "kURLcompletion.h"
#include "open-with.h"
#include "config-kfm.h"

DlgLineEntry::DlgLineEntry( const char *_text, const char* _value, QWidget *parent, bool _file_mode )
        : QDialog( parent, 0L, true )
{
    setGeometry( x(), y(), 350, 100 );
    setFocusPolicy(StrongFocus);

    QLabel *label = new QLabel( _text , this );
    label->setGeometry( 10, 10, 330, 15 );

    edit = new KLined( this, 0L );
    
    if ( _file_mode ) {
        completion = new KURLCompletion();
	connect ( edit, SIGNAL (completion()),
		  completion, SLOT (make_completion()));
	connect ( edit, SIGNAL (rotation()),
		  completion, SLOT (make_rotation()));
	connect ( edit, SIGNAL (textChanged(const char *)),
		  completion, SLOT (edited(const char *)));
	connect ( completion, SIGNAL (setText (const char *)),
		  edit, SLOT (setText (const char *)));
    }
    else
	    completion = 0L;

    edit->setGeometry( 10, 40, 330, 25 );
    connect( edit, SIGNAL(returnPressed()), SLOT(accept()) );

    QPushButton *ok;
    QPushButton *clear;
    QPushButton *cancel;
    ok = new QPushButton( klocale->translate("OK"), this );
    ok->setGeometry( 10,70, 80,25 );
    ok->setDefault(TRUE);
    connect( ok, SIGNAL(clicked()), SLOT(accept()) );

    clear = new QPushButton( klocale->translate("Clear"), this );
    clear->setGeometry( 135, 70, 80, 25 );
    connect( clear, SIGNAL(clicked()), SLOT(slotClear()) );

    cancel = new QPushButton( klocale->translate("Cancel"), this );
    cancel->setGeometry( 260, 70, 80, 25 );
    connect( cancel, SIGNAL(clicked()), SLOT(reject()) );

    edit->setText( _value );
    edit->setFocus();
}

DlgLineEntry::~DlgLineEntry()
{
	delete completion;
}

void DlgLineEntry::slotClear()
{
    edit->setText("");
}

/***************************************************************
 *
 * OpenWithDlg
 *
 ***************************************************************/

OpenWithDlg::OpenWithDlg( const char *_text, const char* _value, QWidget *parent, bool _file_mode )
        : QDialog( parent, 0L, true )
{
  m_pTree = 0L;
  m_pBind = 0L;
  haveApp = false;
  
  setGeometry( x(), y(), 370, 100 );
  setFocusPolicy(StrongFocus);

  label = new QLabel( _text , this );
  label->setGeometry( 10, 10, 350, 15 );
  
  edit = new KLined( this, 0L );
    
  if ( _file_mode )
  {
    completion = new KURLCompletion();
    connect ( edit, SIGNAL (completion()),
	      completion, SLOT (make_completion()));
    connect ( edit, SIGNAL (rotation()),
	      completion, SLOT (make_rotation()));
    connect ( edit, SIGNAL (textChanged(const char *)),
	      completion, SLOT (edited(const char *)));
    connect ( completion, SIGNAL (setText (const char *)),
	      edit, SLOT (setText (const char *)));
  }
  else
    completion = 0L;

  edit->setGeometry( 10, 35, 350, 25 );
  connect( edit, SIGNAL(returnPressed()), SLOT(accept()) );
  
  ok = new QPushButton( klocale->translate("OK"), this );
  ok->setGeometry( 10,70, 80,25 );
  ok->setDefault(TRUE);
  connect( ok, SIGNAL(clicked()), SLOT(slotOK()) );
  
  browse = new QPushButton( klocale->translate("&Browser"), this );
  browse->setGeometry( 100, 70, 80, 25 );
  connect( browse, SIGNAL(clicked()), SLOT(slotBrowse()) );
  
  clear = new QPushButton( klocale->translate("Clear"), this );
  clear->setGeometry( 190, 70, 80, 25 );
  connect( clear, SIGNAL(clicked()), SLOT(slotClear()) );
  
  cancel = new QPushButton( klocale->translate("Cancel"), this );
  cancel->setGeometry( 280, 70, 80, 25 );
  connect( cancel, SIGNAL(clicked()), SLOT(reject()) );

  terminal = new QCheckBox( klocale->translate("Run in terminal"), this );
  terminal->adjustSize();
  terminal->setGeometry( 360-terminal->width(), 10, terminal->width(), 15 );
  
  edit->setText( _value );
  edit->setFocus();
  haveApp = false;
}

OpenWithDlg::~OpenWithDlg()
{
  delete completion;
}

void OpenWithDlg::slotClear()
{
  edit->setText("");
}

void OpenWithDlg::slotBrowse()
{
  if ( m_pTree )
    return;
  
  browse->setEnabled( false );

  ok->setGeometry( 10,280, 80,25 );
  browse->setGeometry( 100, 280, 80, 25 );
  clear->setGeometry( 190, 280, 80, 25 );
  cancel->setGeometry( 280, 280, 80, 25 );

  m_pTree = new KApplicationTree( this );
  connect( m_pTree, SIGNAL( selected( const char*, const char* ) ), this, SLOT( slotSelected( const char*, const char* ) ) );

  connect( m_pTree, SIGNAL( highlighted( const char*, const char* ) ), this, SLOT( slotHighlighted( const char*, const char* ) ) );

  m_pTree->setGeometry( 10, 70, 350, 200 );
  m_pTree->show();
  m_pTree->setFocus();
  
  resize( width(), height() + 210 );
}

void OpenWithDlg::resizeEvent(QResizeEvent *){

  // someone will have to write proper geometry management 
  // but for now this will have to do ....

  if(m_pTree){

    label->setGeometry( 10, 10, width() - terminal->width()-20, 15 );
    edit->setGeometry( 10, 35, width() - 20, 25 );
    ok->setGeometry( 10,height() - 30, (width()-20-30)/4,25 );
    browse->setGeometry( 10 + (width()-20-30)/4 + 10
			 ,height() - 30, (width()-20-30)/4, 25 );
    clear->setGeometry(10 + 2*((width()-20-30)/4) + 2*10 
		       ,height() - 30, (width()-20-30)/4, 25 );
    cancel->setGeometry( 10 + 3*((width()-20-30)/4) + 3*10 ,
			 height() - 30, (width()-20-30)/4, 25 );
    m_pTree->setGeometry( 10, 70, width() - 20, height() - 110 );
    terminal->setGeometry( 350-terminal->width(), 10, terminal->width(), 15);

  }
  else{

    label->setGeometry( 10, 10, width() - 20, 15 );
    edit->setGeometry( 10, 35, width() - 20, 25 );
    ok->setGeometry( 10,height() - 30, (width()-20-30)/4,25);
    browse->setGeometry( 10 + (width()-20-30)/4 + 10,
			 height() - 30, (width()-20-30)/4, 25 );
    clear->setGeometry( 10 + 2*((width()-20-30)/4) + 2*10,
			height() - 30, (width()-20-30)/4, 25 );
    cancel->setGeometry( 10 + 3*((width()-20-30)/4) + 3*10,
			 height() - 30, (width()-20-30)/4, 25 );

  }
}

void OpenWithDlg::slotSelected( const char* _name, const char* _exec )
{
  m_pBind = KMimeBind::findByName( _name );
  if ( !m_pBind )
    edit->setText( _exec );
  else
    edit->setText( "" );

  accept();
}

void OpenWithDlg::slotHighlighted( const char* _name, const char* _exec )
{
  qName = _name;
  qName.detach();
  qExec = _exec;
  qExec.detach();
  haveApp = true;

}


void OpenWithDlg::slotOK(){

  if(haveApp){
    m_pBind = KMimeBind::findByName( qName.data() );
  }

  if( terminal->isChecked() ) {
    KConfig *config = kapp->getConfig();
    config->setGroup( "KFM Misc Defaults" );
    QString t(config->readEntry( "Terminal", DEFAULT_TERMINAL ));

    t += " -e ";
    t += edit->text();
    edit->setText( t );
  }

  haveApp = false;
  accept();
}

#include "kfmdlg.moc"

