//                              -*- Mode: C++ -*- 
// Title            : kgreeter.cpp
// 
// Description      : Greeter module for xdm
// Author           : Steffen Hansen
// Created On       : Mon Apr 28 21:48:52 1997
// Last Modified By : Steffen Hansen
// Last Modified On : Tue Sep  9 18:53:27 1997
// Update Count     : 57
// Status           : Unknown, Use with caution!
// 

#include "kgreeter.h"

extern "C" {
#include "dm.h"
#include "greet.h"

// Make the C++ compiler shut the f... up:
int Verify( struct display*, struct greet_info*, struct verify_info*);
char** parseArgs( char**, const char*);
void DeleteXloginResources( struct display*, Display*);
void SetupDisplay( struct display* d);
void SecureDisplay( struct display* d, Display *);
void RegisterCloseOnFork( int );
int source(void*,void*);
void SessionExit(void*,void*,void*);
}

#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

#ifdef TEST_KDM

int Verify( struct display*, struct greet_info*, struct verify_info*) {}
char** parseArgs( char**, const char*) {}
void DeleteXloginResources( struct display*, Display*) {}
void SetupDisplay( struct display* d) {}
void SecureDisplay( struct display* d, Display *dpy) {}
void RegisterCloseOnFork( int i) {}
int source(void*,void*) {}
void SessionExit(void*,void*,void*) {}
#endif

// Global vars
KGreeter* kgreeter;

KDMConfig               *kdmcfg;
struct display          *d;
Display                 ** dpy;
struct verify_info      *verify;
struct greet_info       *greet;


class MyApp:public KApplication {
public:
  MyApp( int &argc, char **argv );
  virtual bool x11EventFilter( XEvent * );
};

MyApp::MyApp(int &argc, char **argv ):KApplication(argc, argv){
}

bool 
MyApp::x11EventFilter( XEvent * ev){
     //printf("Qt: got one event %d for window %ld\n", ev->type, ev->xany.window);
     if( ev->type == KeyPress){
	  // This should go away
	  if (XLookupKeysym(&(ev->xkey),0) == XK_Return)
	       kgreeter->ReturnPressed();
     }
     // Hack to tell FDialogs to take focus
     if( ev->type == ConfigureNotify) {
	  QEvent e( Event_Show);
	  QWidgetList *list = topLevelWidgets();
	  QWidgetListIt it( *list);
	  while( it.current()) {
	       if( it.current()->winId() == 
		   (( XConfigureEvent *) ev)->window) {
		    MyApp::sendEvent( it.current(), &e);
		    break;
	       }
	       ++it;
	  }

	  delete list;
     }
     return FALSE;
}

static void
set_min( QWidget* w)
{
     w->adjustSize();
     w->setMinimumSize( w->size());
}

static void
set_fixed( QWidget* w)
{
     w->adjustSize();
     w->setFixedSize( w->size());
}

KGreeter::KGreeter(QWidget *parent = 0, const char *t = 0) 
  : FDialog( parent, t, false, 
	     WStyle_Customize | WStyle_NoBorder | WStyle_Tool)
{
     QFrame* winFrame = new QFrame( this);
     winFrame->setFrameStyle(QFrame::WinPanel| QFrame::Raised);
     QBoxLayout* vbox = new QBoxLayout(  winFrame, 
					 QBoxLayout::TopToBottom, 
					 10, 10);
     QBoxLayout* hbox1 = new QBoxLayout( QBoxLayout::LeftToRight, 10);
     QBoxLayout* hbox2 = new QBoxLayout( QBoxLayout::LeftToRight, 10);

     QGridLayout* grid = new QGridLayout( 3, 2, 5);

     QLabel* welcomeLabel = new QLabel( kdmcfg->greetString()->data(), this);
     welcomeLabel->setAlignment(AlignCenter);
     welcomeLabel->setFont( *kdmcfg->greetFont());
     set_min( welcomeLabel);
     vbox->addWidget( welcomeLabel);
     if( kdmcfg->users()) {
	  user_view = new KDMView( this);
	  user_view->insertItemList( kdmcfg->users());
	  vbox->addWidget( user_view, 3);
     } else {
	  user_view = NULL;
     }

     pixLabel = new QLabel( this);
     QPixmap pixmap;
     if( QFile::exists( kdmcfg->logo()->data()))
	  pixmap.load( kdmcfg->logo()->data());
     else
	  pixmap.resize( 100,100);
     pixLabel->setPixmap( pixmap);
     pixLabel->setFixedSize( pixmap.width(), pixmap.height());
     //pixLabel->setFrameStyle( QFrame::Panel| QFrame::Sunken);
     
     loginLabel = new QLabel( klocale->translate("Login:"), this);
     set_min( loginLabel);
     loginEdit = new QLineEdit( this);
     loginEdit->setFocus();

     passwdLabel = new QLabel( klocale->translate("Password:"), this);
     set_min( passwdLabel);
     passwdEdit = new QLineEdit( this);
     //passwdEdit->setEchoMode( QLineEdit::Password);
     QColorGroup   passwdColGroup( 
	  QApplication::palette()->normal().foreground(), 
	  QApplication::palette()->normal().background(),
	  QApplication::palette()->normal().light(), 
	  QApplication::palette()->normal().dark(), 
	  QApplication::palette()->normal().mid(),
	  QApplication::palette()->normal().base(),
	  QApplication::palette()->normal().base());
     QPalette passwdPalette( passwdColGroup, passwdColGroup, passwdColGroup);
     passwdEdit->setPalette( passwdPalette);
     vbox->addLayout( hbox1);
     vbox->addLayout( hbox2);
     hbox1->addWidget( pixLabel, 0, AlignTop);
     hbox1->addLayout( grid, 3);
     
     QFrame* sepFrame = new QFrame( this);
     sepFrame->setFrameStyle( QFrame::HLine| QFrame::Sunken);

     grid->addWidget( loginLabel , 0, 0);
     grid->addWidget( loginEdit  , 0, 1);
     grid->addWidget( passwdLabel, 1, 0);
     grid->addWidget( passwdEdit , 1, 1);
     grid->addMultiCellWidget( sepFrame, 2, 2, 0, 1);
     grid->setColStretch( 1, 4);

     failedLabel = new QLabel( klocale->translate("Login failed!"), this);
     failedLabel->setFont( *kdmcfg->failFont());
     set_min( failedLabel);
     failedLabel->hide();
     hbox2->addWidget( failedLabel);

     QLabel* sessionargLabel = new QLabel(klocale->translate("Session Type:"),
					  this);
     set_min( sessionargLabel);
     sessionargLabel->setAlignment( AlignRight|AlignVCenter);
     hbox2->addWidget( sessionargLabel);
     sessionargBox = new QComboBox( false, this);

     QStrListIterator it( *kdmcfg->sessionTypes());

     for( ; it.current(); ++it) {
	  sessionargBox->insertItem( it.current());
     }
     
     set_fixed( sessionargBox);
     hbox2->addWidget( sessionargBox);

     goButton = new QPushButton( klocale->translate("Go!"), this);
     connect( goButton, SIGNAL( clicked()), SLOT(go_button_clicked()));
     //goButton->setDefault( true);
     set_fixed( goButton);
     hbox2->addWidget( goButton, AlignBottom);

     cancelButton = new QPushButton( klocale->translate("Cancel"), this);
     connect( cancelButton, SIGNAL(clicked()), SLOT(cancel_button_clicked()));
     set_fixed( cancelButton);
     hbox2->addWidget( cancelButton, AlignBottom);

     int sbw;
#ifndef TEST_KDM
     if( kdmcfg->shutdownButton() != KDMConfig::KNone 
	 && ( kdmcfg->shutdownButton() != KDMConfig::ConsoleOnly 
	 || d->displayType.location == Local)) {
	  shutdownButton = new QPushButton( klocale->translate("Shutdown..."),
					    this);
	  connect( shutdownButton, SIGNAL(clicked()), 
		   SLOT(shutdown_button_clicked()));
	  set_fixed( shutdownButton);
	  hbox2->addWidget( shutdownButton, AlignBottom);
	  sbw = shutdownButton->width() + 100;
     } else
#endif
	  sbw = 80;
     int w = QMAX( 
	  QMAX( 
	       QMAX( loginLabel->width(), passwdLabel->width()) + 
	       2*QMAX( loginEdit->width(), passwdEdit->width()) + 
	       pixLabel->width() + 60, welcomeLabel->width() + 40), 
	  failedLabel->width() + sessionargLabel->width() 
	  + sessionargBox->width() + goButton->width() + 
	  cancelButton->width() + sbw);
     vbox->activate();
     if( user_view) {
	  int tmp = user_view->sizeHintHeight( w);
	  tmp = QMIN( tmp, winFrame->minimumSize().height());
	  //tmp = QMAX( tmp, user_view->absMinHeight());
	  user_view->setMinimumSize( w, tmp);
     }
     vbox->activate();
     setMinimumSize( winFrame->minimumSize());
     startTimer(500);
     timer = new QTimer( this );
     // Signals/Slots
     connect( timer, SIGNAL(timeout()),
           this , SLOT(timerDone()) );
     if( user_view)
	  connect( user_view, SIGNAL(selected(int)), 
		   this, SLOT(slot_user_name( int)));
     adjustSize();
}

void 
KGreeter::slot_user_name( int i)
{
     loginEdit->setText( kdmcfg->users()->at( i)->text());
     passwdEdit->setFocus();
}

void 
KGreeter::SetTimer(){
#if 0
     if (!isVisible()){
	  show();
	  qApp->desktop()->releaseKeyboard();
     }
#endif     

     if (!failedLabel->isVisible())
	  timer->start( 40000, TRUE );
}

void 
KGreeter::timerDone()
{
     if (failedLabel->isVisible()){
	  failedLabel->hide();
	  goButton->setEnabled( true);
     } else {
	  //cancel_button_clicked();
	  //mydesktop->grabKeyboard();
	  //hide();
     }
}

void 
KGreeter::cancel_button_clicked(){
     loginEdit->setText("");
     passwdEdit->setText("");
     loginEdit->setFocus();
}

void 
KGreeter::shutdown_button_clicked(){
     timer->stop();
     KDMShutdown k( kdmcfg->shutdownButton(),
		    this, "Shutdown",
		    kdmcfg->shutdown()->data(), 
		    kdmcfg->restart()->data());
     k.exec();
     SetTimer();
}

void 
KGreeter::go_button_clicked(){
     greet->name = qstrdup(loginEdit->text());
     greet->password = qstrdup(passwdEdit->text());
     
     if (!Verify (d, greet, verify)){
	  //passwdEdit->setEnabled( false);
	  //loginEdit->setEnabled( false);
	  //cancelButton->hide();
	  //goButton->hide();
	  failedLabel->show();
	  goButton->setEnabled( false);
	  cancel_button_clicked();
	  timer->start( 2000, true );
	  //cancelButton->setFocus();
	  return;
     }
     // Set session argument:
     verify->argv = parseArgs( verify->argv, sessionargBox->text( sessionargBox->currentItem()));
     qApp->desktop()->setCursor( waitCursor);
     qApp->desktop()->grabMouse();
     hide();
     DeleteXloginResources( d, dpy);
     qApp->desktop()->setCursor( arrowCursor);
     qApp->exit();
}

void
KGreeter::ReturnPressed()
{
     /*if (!isVisible()) {
	  printf("KGreeter: Not visible\n");
	  return;
     }*/
     if( !goButton->isEnabled())
	  return;
     if( loginEdit->hasFocus()) {
	  passwdEdit->setFocus();
     }
     else if (passwdEdit->hasFocus()
	      || goButton->hasFocus() 
	      || cancelButton->hasFocus()) {
	  go_button_clicked();
     }
}

static void 
DoIt()
{
     // First initialize display:
     SetupDisplay( d);

     kgreeter = new KGreeter;		   
     kgreeter->show();
     QApplication::restoreOverrideCursor();
     qApp->exec();

     delete kgreeter;
}

#include "kgreeter.moc"

#ifdef TEST_KDM

int main(int argc, char **argv)
{
     MyApp app(argc, argv);
     
     kdmcfg = new KDMConfig( KApplication::kdedir());

     app.setStyle( WindowsStyle);
     app.setFont( *kdmcfg->normalFont());

     kgreeter = new KGreeter;
     app.setMainWidget( kgreeter);
     kgreeter->show();
     return app.exec();
}

#endif

static int
IOErrorHandler (Display*)
{
  exit (RESERVER_DISPLAY);
  /* Not reached */
  return 0;
}

greet_user_rtn 
GreetUser(
     struct display          *d2,
     Display                 ** dpy2,
     struct verify_info      *verify2,
     struct greet_info       *greet2,
     struct dlfuncs       */*dlfuncs*/
     )
{
     d = d2;
     dpy = dpy2;
     verify = verify2;
     greet = greet2;
     
     int argc = 4;
     char* argv[5] = {"kdm", "-display", NULL, NULL, NULL};
     argv[2] = d->name;
     MyApp myapp( argc, argv );
     printf("LANG=%s, Domain=%s, appName=%s\n", getenv("LANG"), 
	    klocale->language().data(), kapp->appName().data());
     QApplication::setOverrideCursor( waitCursor );
     // Test new option "-kdedir"
     extern char* optionKdedir;
     if( optionKdedir[0])
	  kdmcfg = new KDMConfig( optionKdedir);
     else
	  kdmcfg = new KDMConfig( KApplication::kdedir());
     
     myapp.setFont( *kdmcfg->normalFont());
     myapp.setStyle( kdmcfg->style());

     *dpy = qt_xdisplay();
     
     RegisterCloseOnFork (ConnectionNumber (*dpy));
     SecureDisplay (d, *dpy);
     // this is for a real BackSpace-key
     KeySym mysym = XK_BackSpace;
     XChangeKeyboardMapping(*dpy, 22, 1, &mysym, 1);
     
     // this is necessary, since Qt-1.1 just overwrites the
     // IOErrorHandler that was set by xdm!!!
     // we have to return RESERVER_DISPLAY to restart the server
     XSetIOErrorHandler(IOErrorHandler);
     
     DoIt();
     
     /*
      * Run system-wide initialization file
      */
     if (source (verify->systemEnviron, d->startup) != 0)
     {
	  printf("Startup program %s exited with non-zero status\n",
		 d->startup);
	  SessionExit (d, OBEYSESS_DISPLAY, FALSE);
     }                                                            
     // Clean up and log user in:
     delete kdmcfg;
     QColor::destroyAllocContext( -1); // free all colors
     XKillClient( qt_xdisplay(), AllTemporary);
     return Greet_Success;
}
