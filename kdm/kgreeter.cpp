    /*

    Greeter module for xdm
    $Id$

    Copyright (C) 1997, 1998 Steffen Hansen
                             stefh@mip.ou.dk


    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    */
 
#include "kgreeter.h"

#include <stdio.h>
#include <signal.h>
#include <pwd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <math.h>
#include <unistd.h>

#include <qmsgbox.h>
#include <kstring.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>

#include "dm.h"
#include "greet.h"

// Make the C++ compiler shut the f... up:
extern "C" {
int Verify( struct display*, struct greet_info*, struct verify_info*);
char** parseArgs( char**, const char*);
void DeleteXloginResources( struct display*, Display*);
void SetupDisplay( struct display* d);
void SecureDisplay( struct display* d, Display *);
void RegisterCloseOnFork( int );
int source(void*,void*);
void SessionExit(void*,void*,void*);
}

#ifdef USESHADOW
#include<shadow.h>
#endif

#ifdef HAVE_LOGIN_CAP_H
#include <login_cap.h>		/* BSDI-like login classes */
#endif

static struct passwd *pwd = NULL;

#ifdef HAVE_LOGIN_CAP_H
static login_cap_t *lc = NULL;
#endif

#if USESHADOW
static struct spwd *swd = NULL;
#endif

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
KGreeter* kgreeter = 0;

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

MyApp::MyApp(int &argc, char **argv ) : KApplication(argc, argv){
}

bool 
MyApp::x11EventFilter( XEvent * ev){
     if( ev->type == KeyPress && kgreeter){
	  // This should go away
	  if (XLookupKeysym(&(ev->xkey),0) == XK_Return)
	       kgreeter->ReturnPressed();
     }
     // Hack to tell dialogs to take focus
     if( ev->type == ConfigureNotify) {
	  QWidget* target = QWidget::find( (( XConfigureEvent *) ev)->window);
	  target = target->topLevelWidget();
	  if( target->isVisible() && !target->isPopup())
	    XSetInputFocus( qt_xdisplay(), target->winId(), 
			    RevertToParent, CurrentTime);
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
  : QWidget( parent, t, WStyle_Customize | WStyle_NoBorder | WStyle_Tool)
{
     QFrame* winFrame = new QFrame( this);
     winFrame->setFrameStyle(QFrame::WinPanel| QFrame::Raised);
     QBoxLayout* vbox = new QBoxLayout(  winFrame, 
					 QBoxLayout::TopToBottom, 
					 10, 10);
     QBoxLayout* hbox1 = new QBoxLayout( QBoxLayout::LeftToRight, 10);
     QBoxLayout* hbox2 = new QBoxLayout( QBoxLayout::LeftToRight, 10);

     QGridLayout* grid = new QGridLayout( 4, 2, 5);

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
     pixLabel->setFrameStyle( QFrame::Panel| QFrame::Sunken);
     pixLabel->setAutoResize( true);
     pixLabel->setMargin( 0);
     QPixmap pixmap;
     if( QFile::exists( kdmcfg->logo()->data()))
	  pixmap.load( kdmcfg->logo()->data());
     else
	  pixmap.resize( 100,100);
     pixLabel->setPixmap( pixmap);
     pixLabel->setFixedSize( pixLabel->width(), pixLabel->height());

     loginLabel = new QLabel( klocale->translate("Login:"), this);
     set_min( loginLabel);
     loginEdit = new QLineEdit( this);

     // The line-edit look _very_ bad if you don't give them 
     // a resonal height that observes a proportional aspect.
     // -- Bernd
     int leheight;
     leheight = QMAX( 30,loginEdit->sizeHint().height());
     loginEdit->setFixedHeight( leheight);
     loginEdit->setFocus();

     passwdLabel = new QLabel( klocale->translate("Password:"), this);
     set_min( passwdLabel);
     passwdEdit = new QLineEdit( this);

     int pweheight;
     pweheight = QMAX( 30,passwdEdit->sizeHint().height());
     passwdEdit->setFixedHeight( pweheight);

     passwdEdit->setEchoMode( QLineEdit::NoEcho);
     vbox->addLayout( hbox1);
     vbox->addLayout( hbox2);
     hbox1->addWidget( pixLabel, 0, AlignTop);
     hbox1->addLayout( grid, 3);
     
     QFrame* sepFrame = new QFrame( this);
     sepFrame->setFrameStyle( QFrame::HLine| QFrame::Sunken);
     sepFrame->setFixedHeight( sepFrame->sizeHint().height());

     grid->addWidget( loginLabel , 0, 0);
     grid->addWidget( loginEdit  , 0, 1);
     grid->addWidget( passwdLabel, 1, 0);
     grid->addWidget( passwdEdit , 1, 1);
     grid->addMultiCellWidget( sepFrame, 3, 3, 0, 1);
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
	  sbw = shutdownButton->width();
     } else
#endif
	  sbw = 0;
     vbox->activate();
     int w = QMAX( 
	  QMAX( 
	       QMAX( loginLabel->width(), passwdLabel->width()) + 
	       pixLabel->width(), welcomeLabel->width()), 
	  failedLabel->width() + sessionargLabel->width() 
	  + sessionargBox->width() + goButton->width() + 
	  cancelButton->width() + sbw);
     if( user_view) {
	  int tmp = user_view->sizeHintHeight( w);
	  tmp = QMIN( tmp, winFrame->minimumSize().height());
	  user_view->setMinimumSize( w, tmp);
     }
     vbox->activate();
     setMinimumSize( winFrame->minimumSize());
     timer = new QTimer( this );
     // Signals/Slots
     connect( timer, SIGNAL(timeout()),
           this , SLOT(timerDone()) );
     if( user_view)
	  connect( user_view, SIGNAL(selected(int)), 
		   this, SLOT(slot_user_name( int)));
     adjustSize();
     // Center on screen:
     move( QApplication::desktop()->width()/2  - width()/2,
	   QApplication::desktop()->height()/2 - height()/2 );  
}

void 
KGreeter::slot_user_name( int i)
{
     loginEdit->setText( kdmcfg->users()->at( i)->text());
     passwdEdit->setFocus();
     load_wm();
}

void 
KGreeter::SetTimer()
{
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
KGreeter::cancel_button_clicked()
{
     loginEdit->setText("");
     passwdEdit->setText("");
     loginEdit->setFocus();
}

void 
KGreeter::shutdown_button_clicked()
{
     timer->stop();
     KDMShutdown k( kdmcfg->shutdownButton(),
		    this, "Shutdown",
		    kdmcfg->shutdown()->data(), 
		    kdmcfg->restart()->data());
     k.exec();
     SetTimer();
}

void
KGreeter::save_wm()
{
     // read passwd
     struct passwd *pwd = getpwnam(greet->name);
     endpwent();
     if (!pwd) return;

     QString file;
     file.sprintf("%s/"WMRC, pwd->pw_dir);
     QFile f(file);

     // open file as user which is loging in
     seteuid(pwd->pw_uid);
     bool noerr = f.open(IO_WriteOnly) != 0;

     if ( noerr ) {
	  QTextStream t;
	  t.setDevice( &f );
	  t << sessionargBox->text( sessionargBox->currentItem()) << endl;
	  f.close();
     }
     seteuid(0);
}

void
KGreeter::load_wm()
{
     // read passwd
     passwd *pwd = getpwnam(loginEdit->text());
     endpwent();
     if (!pwd) return;

     // Take care of bogus user name:
     if( !pwd) return;

     QString file;
     file.sprintf("%s/"WMRC, pwd->pw_dir);
     QFile f(file);

     // open file as user which is loging in
     seteuid(pwd->pw_uid);
     bool noerr = f.open(IO_ReadOnly) != 0;

     // set default wm
     int wm = 0;
     if ( noerr ) {
	  QTextStream t( &f );
	  QString s;
	  if ( !t.eof() ) s = t.readLine();
	  f.close();

	  for (int i = 0; i < sessionargBox->count(); i++)
	       if (strcmp(sessionargBox->text(i), s) == 0) {
		    wm = i;
		    break;
	       }
     }
     seteuid(0);
     sessionargBox->setCurrentItem(wm);
}

/* This stuff should doesn't really belong to the kgreeter, but verify.c is
 * just C, not C++.
 *
 * A restrict_host() should be added.
 */
bool
KGreeter::restrict()
{
     bool rval = false;

     pwd = getpwnam(greet->name);
     endpwent();
     if (!pwd) return false;

#ifdef HAVE_LOGIN_CAP_H
     lc = login_getpwclass(pwd);
#endif
#if USESHADOW
     swd = getspnam(greet->name);
     endspent();
     if (!swd) return false;
#endif

     if (restrict_nologin() ||
	 restrict_nohome() ||
	 restrict_expired() ||
	 restrict_time())
       rval = true;

#ifdef HAVE_LOGIN_CAP_H
     login_close(lc);
     lc = NULL;
#endif

     return rval;
}

#ifdef HAVE_LOGIN_CAP_H
bool
KGreeter::restrict_time()
{
     // don't deny root to log in
     if (!pwd->pw_uid) return false;

     if(!auth_timeok(lc, time(NULL))) {
         QMessageBox::critical(NULL, NULL,
			       i18n("Logins not available right now."),
			       i18n("&OK"));
         return true;
     }

     return false;
}
#else
bool
KGreeter::restrict_time()
{
     return false;
}
#endif

#ifdef USE_PAM
bool
KGreeter::restrict_nologin()
{
     // PAM handles /etc/nologin itself.
     return false;
}
#else /* !USE_PAM */
bool
KGreeter::restrict_nologin()
{
     // don't deny root to log in
     if (pwd && !pwd->pw_uid) return false;

#ifdef HAVE_LOGIN_CAP_H
     char *file = login_getcapstr(lc, "nologin", "", NULL);
     if (login_getcapbool(lc, "ignorenologin", 0)) return false;
#else
#ifndef _PATH_NOLOGIN
#define _PATH_NOLOGIN "/etc/nologin"
#endif
     char *file = _PATH_NOLOGIN;
#endif
     if (!file) return false;
     QFile f(file);
     QString s;  
     if ( f.open(IO_ReadOnly) ) {
       QTextStream t( &f ); 
       while ( !t.eof() )
         s += t.readLine() + "\n";  
       f.close();
       QMessageBox::critical(NULL, NULL, s, i18n("&OK"));
       return true;
     }

     return false;
}
#endif /* !USE_PAM */

#ifdef BSD
bool
KGreeter::restrict_expired(){
#define DEFAULT_WARN  (2L * 7L * 86400L)  /* Two weeks */
     // don't deny root to log in
     if (!pwd->pw_uid) return false;

#ifdef HAVE_LOGIN_CAP_H
     bool quietlog = login_getcapbool(lc, "hushlogin", 0);
     time_t warntime = login_getcaptime(lc, "warnexpire",
				 DEFAULT_WARN, DEFAULT_WARN);
#else
     bool quietlog = false;
     time_t warntime = DEFAULT_WARN;
#endif
     if (pwd->pw_expire)
	  if (pwd->pw_expire <= time(NULL)) {
	       QMessageBox::critical(NULL, NULL,
				     i18n("Sorry -- your account has expired."), 
				     i18n("&OK"));
	       return true;
	  } else if (pwd->pw_expire - time(NULL) < warntime && !quietlog) {
	       QString str;
	       str.sprintf(i18n("Warning: your account expires on %s"), 
			   ctime(&pwd->pw_expire));  // use locales
	       QMessageBox::critical(NULL, NULL,
				     str,
				     i18n("&OK"));
	  }

     return false;
}
#elif USESHADOW
bool
KGreeter::restrict_expired(){
#define DEFAULT_WARN  (2L * 7L * 86400L)  /* Two weeks */
     // don't deny root to log in
     if (!pwd->pw_uid) return false;

     time_t warntime = DEFAULT_WARN;
     time_t expiresec = swd->sp_expire*86400L; //sven: ctime uses seconds
     
     if (swd->sp_expire != -1)
	 if (expiresec <= time(NULL)) {
	     QMessageBox::critical(NULL, NULL,
				   i18n("Sorry -- your account has expired."),
				   i18n("&OK"));
	     return true;
	 } else if (expiresec - time(NULL) < warntime) {
             QString str;
	     str.sprintf(i18n("Warning: your account expires on %s"),
			 ctime(&expiresec));  // use locales
	     QMessageBox::critical(NULL, NULL,
				   str,
				   i18n("&OK"));
	 }

     return false;
}
#else */!USESHADOW*/
bool
KGreeter::restrict_expired()
{
     return false;
}
#endif

#ifdef HAVE_LOGIN_CAP_H
bool
KGreeter::restrict_nohome(){
     // don't deny root to log in
     if (!pwd->pw_uid) return false;

     seteuid(pwd->pw_uid);
     if (!*pwd->pw_dir || chdir(pwd->pw_dir) < 0) {
	  if (login_getcapbool(lc, "requirehome", 0)) {
	       QMessageBox::critical(NULL, NULL, 
				     i18n("Home directory not available"), 
				     i18n("&OK"));
	       return true;
	  }
     }
     seteuid(0);

     return false;
}
#else
bool
KGreeter::restrict_nohome()
{
     return false;
}
#endif


void 
KGreeter::go_button_clicked()
{
     greet->name = qstrdup(loginEdit->text());
     greet->password = qstrdup(passwdEdit->text());
     
     if (!Verify (d, greet, verify)){
	  failedLabel->show();
	  goButton->setEnabled( false);
	  cancel_button_clicked();
	  timer->start( 2000, true );
	  return;
     }
     // Set session argument:
     verify->argv = parseArgs( verify->argv, 
			       sessionargBox->text( 
				    sessionargBox->currentItem()));
     if (restrict())
       SessionExit (d, OBEYSESS_DISPLAY, FALSE);

     save_wm();
     //qApp->desktop()->setCursor( waitCursor);
     qApp->setOverrideCursor( waitCursor);
     hide();
     DeleteXloginResources( d, dpy);
     qApp->exit();
}

void
KGreeter::ReturnPressed()
{
     if( !goButton->isEnabled())
	  return;
     if( loginEdit->hasFocus()) {
	  passwdEdit->setFocus();
          load_wm();
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
     // Give focus to root window:
     QApplication::desktop()->setActiveWindow();
     delete kgreeter;
}

#include "kgreeter.moc"

#ifdef TEST_KDM

int main(int argc, char **argv)
{
     MyApp app(argc, argv);
     
     kdmcfg = new KDMConfig();

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
 
     struct sigaction sig;
 
     /* KApplication trashes xdm's signal handlers :-( */
     sigaction(SIGCHLD,NULL,&sig);
 
     argv[2] = d->name;
     MyApp* myapp = new MyApp( argc, argv );
     /*printf("LANG=%s, Domain=%s, appName=%s\n", getenv("LANG"), 
	    klocale->language().data(), kapp->appName().data());*/
     QApplication::setOverrideCursor( waitCursor );
     kdmcfg = new KDMConfig( );
     
     myapp->setFont( *kdmcfg->normalFont());
     myapp->setStyle( kdmcfg->style());

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
     
     sigaction(SIGCHLD,&sig,NULL);

     DoIt();
     
     /*
      * Run system-wide initialization file
      */
     if (source (verify->systemEnviron, d->startup) != 0)
     {
          QString buf;
	  ksprintf(&buf,
		  "Startup program %s exited with non-zero status.\n"
		  "Please contact your system administrator.\n",
		 d->startup);
	  qApp->restoreOverrideCursor();
	  QMessageBox::critical( 0, "Login aborted", buf, "Retry");
	  SessionExit (d, OBEYSESS_DISPLAY, FALSE);
     }

     // Clean up and log user in:
     XKillClient( qt_xdisplay(), AllTemporary);
     //qApp->desktop()->setCursor( arrowCursor);
     qApp->restoreOverrideCursor();
     delete kdmcfg;
     delete myapp;
     return Greet_Success;
}

/*
 * Local variables:
 * mode: c++
 * c-file-style: "k&r"
 * End:
 */
