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
#include <unistd.h>

#if defined( HAVE_INITGROUPS) && defined( HAVE_GETGROUPS) && defined( HAVE_SETGROUPS)
#  include <grp.h>
#endif

#include <qbitmap.h>
#include <qmsgbox.h>
#include <kstring.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>

#include "dm.h"
#include "greet.h"

#include <kiconloader.h>
#include <kimgio.h>

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
#include <shadow.h>
#endif

#ifdef HAVE_LOGIN_CAP_H
#include <login_cap.h>		/* BSDI-like login classes */
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

void 
KLoginLineEdit::focusOutEvent( QFocusEvent *e) 
{
     emit lost_focus();
     QLineEdit::focusOutEvent( e);
}

class MyApp : public KApplication {
public:
     MyApp( int &argc, char **argv );
     virtual ~MyApp();
     virtual bool x11EventFilter( XEvent * );
};

MyApp::MyApp(int &argc, char **argv ) : KApplication(argc, argv)
{}

MyApp::~MyApp()
{}

bool 
MyApp::x11EventFilter( XEvent * ev){
     if( ev->type == KeyPress && kgreeter){
	  // This should go away
	  KeySym ks = XLookupKeysym(&(ev->xkey),0);
	  if (ks == XK_Return ||
	      ks == XK_KP_Enter)
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

// Misc. functions
static inline int my_seteuid( uid_t euid)
{
#ifdef HAVE_SETEUID
     return seteuid(euid);
#else
     return setreuid(-1, euid);
#endif // HAVE_SETEUID    
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

     kimgioRegister();

     QPixmap pixmap;
     if( QFile::exists( kdmcfg->logo()->data()))
	  pixmap.load( kdmcfg->logo()->data());
     else
	  pixmap.resize( 100,100);
     pixLabel->setPixmap( pixmap);
     pixLabel->setFixedSize( pixLabel->width(), pixLabel->height());

     loginLabel = new QLabel( klocale->translate("Login:"), this);
     set_min( loginLabel);
     loginEdit = new KLoginLineEdit( this);

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

     QStrListIterator it ( *kdmcfg->sessionTypes());
     sessiontags.clear();
     KIconLoader iconLoader;
     for( ; it.current(); ++it) {
#if 0
	  QString output = QString(it.current());
	  QPixmap sesspix(iconLoader.loadIcon(QString("session_") 
					      + it.current() + ".xpm"));
	  QPainter p;
	  QPainter pmask;
	  QFontMetrics fm = sessionargBox->fontMetrics();
	  QPixmap pm( sesspix.width() + fm.width( it.current() ) + 6,
		      QMAX( sesspix.height(), fm.lineSpacing() + 1 ));
	  QBitmap mask( sesspix.width() + fm.width( it.current() ) + 6,
			QMAX( sesspix.height(), fm.lineSpacing() + 1 ), true);
	  pm.fill(colorGroup().background());
	  p.begin(&pm);
	  pmask.begin( &mask);
	  p.drawPixmap( 3, 0, sesspix);
	  if( !sesspix.isNull())
	       pmask.drawPixmap( 3, 0, sesspix.createHeuristicMask());
	  int yPos;
	  if ( sesspix.height() < fm.height() )
	       yPos = fm.ascent() + fm.leading()/2;
	  else
	       yPos = sesspix.height()/2 - fm.height()/2 + fm.ascent();
	  p.drawText( sesspix.width() + 5, yPos, it.current() );
	  pmask.drawText( sesspix.width() + 5, yPos, it.current() );
	  p.end();
	  pmask.end();
	  pm.setMask( mask);
	  sessionargBox->insertItem( pm);
#else
	  sessionargBox->insertItem( it.current());
#endif
	  sessiontags.append(it.current());
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
     connect( loginEdit, SIGNAL(lost_focus()),
	      this, SLOT( load_wm()));
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
	  loginEdit->setEnabled( true);
	  passwdEdit->setEnabled( true);
	  loginEdit->setFocus();
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

// Switch uid/gids to user described by pwd. Return old gid set
// or 0 if an error occurs
#if defined( HAVE_INITGROUPS) && defined( HAVE_GETGROUPS) && defined( HAVE_SETGROUPS)

static inline gid_t* switch_to_user( int *gidset_size, 
				     const struct passwd *pwd)
{
     *gidset_size = getgroups(0,0);
     gid_t *gidset = new gid_t[*gidset_size];
     if( getgroups( *gidset_size, gidset) == -1 ||
	 initgroups(pwd->pw_name, pwd->pw_gid) != 0 ||
	 my_seteuid(pwd->pw_uid) != 0) {
	  // Error, back out
	  my_seteuid(0);
	  setgroups( *gidset_size, gidset);
	  delete[] gidset;
	  return 0;
     }
     return gidset;
}

// Switch uid back to root, and gids to gidset
static inline void switch_to_root( int gidset_size, gid_t *gidset)
{
     my_seteuid(0);
     setgroups( gidset_size, gidset);
     delete[] gidset;
}

void
KGreeter::save_wm()
{
     // read passwd
     struct passwd *pwd = getpwnam(greet->name);
     endpwent();
     if (!pwd) return;
     // we don't need the password
     memset(pwd->pw_passwd, 0, strlen(pwd->pw_passwd));

     QString file;
     ksprintf(&file, "%s/"WMRC, pwd->pw_dir);
     QString sesstype (sessiontags.at( sessionargBox->currentItem()));

     // open file as user which is loging in
     int gidset_size;
     // Go user
     gid_t *gidset = switch_to_user( &gidset_size, pwd);
     if( gidset == 0) return;

     FILE *f = fopen(file.data(), "w");
     if (f) {
	  fprintf(f, "%s\n", sesstype.data());
	  fclose(f);
     }

     // Go root
     switch_to_root( gidset_size, gidset);
}

void
KGreeter::load_wm()
{
     // read passwd
     passwd *pwd = getpwnam(loginEdit->text());
     endpwent();
     if (!pwd) return;
     // we don't need the password
     memset(pwd->pw_passwd, 0, strlen(pwd->pw_passwd));

     QString file;
     ksprintf(&file, "%s/"WMRC, pwd->pw_dir);
     
     int wm = -1;
     int gidset_size;
     // Go user
     gid_t *gidset = switch_to_user( &gidset_size, pwd);
     if( gidset == 0) return;

     // open file as user which is loging in
     FILE *f = fopen(file.data(), "r");
     if (f) {
	  char s[255];
	  
	  fgets(s, sizeof s, f);
	  fclose(f);
	 
	  if (char *p = strchr(s, '\n')) *p = 0;     
	  wm = sessiontags.find( s);
     }

     //Go root
     switch_to_root( gidset_size, gidset);
     if( wm != -1) sessionargBox->setCurrentItem(wm);
}
#else
void
KGreeter::load_wm()
{
}
void
KGreeter::save_wm()
{
}
#endif /* HAVE_INITGROUPS && HAVE_SETGROUPS && HAVE_GETGROUPS */

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
     // we don't need the password
     memset(pwd->pw_passwd, 0, strlen(pwd->pw_passwd));

#ifdef USESHADOW
     swd = getspnam(greet->name);
     endspent();
     if (!swd) return false;
     // we don't need the password
     memset(swd->sp_pwdp, 0, strlen(swd->sp_pwdp));
#endif

#ifdef HAVE_LOGIN_CAP_H
     lc = login_getpwclass(pwd);
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

#ifndef _PATH_NOLOGIN
#define _PATH_NOLOGIN "/etc/nologin"
#endif

#ifdef HAVE_LOGIN_CAP_H
     /* Do we ignore a nologin file? */
     if (login_getcapbool(lc, "ignorenologin", 0))
       return false;

     char *file;
     /* Note that <file> will be "" if there is no nologin capability */
     if ((file = login_getcapstr(lc, "nologin", "", NULL)) == NULL) {
       QMessageBox::critical(NULL, NULL, i18n("login_getcapstr() failed."),
	 i18n("&OK"));
       return true;
     }
#endif

     QFile f;

#ifdef HAVE_LOGIN_CAP_H
     if (*file) {
       f.setName(file);
       f.open(IO_ReadOnly);
     }
#endif

     if (f.handle() == -1) {
       f.setName(_PATH_NOLOGIN);
       f.open(IO_ReadOnly);
     }

     if (f.handle() != -1) {
       QString s;
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
#ifndef __osf__
     if (pwd->pw_expire)
	  if (pwd->pw_expire <= time(NULL)) {
	       QMessageBox::critical(NULL, NULL,
				     i18n("Sorry -- your account has expired."), 
				     i18n("&OK"));
	       return true;
	  } else if (pwd->pw_expire - time(NULL) < warntime && !quietlog) {
	       QString str;
	       ksprintf(&str, i18n("Warning: your account expires on %s"), 
			   ctime(&pwd->pw_expire));  // use locales
	       QMessageBox::critical(NULL, NULL,
				     str,
				     i18n("&OK"));
	  }
#endif // __osf__

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
	     ksprintf(&str, i18n("Warning: your account expires on %s"),
			 ctime(&expiresec));  // use locales
	     QMessageBox::critical(NULL, NULL,
				   str,
				   i18n("&OK"));
	 }

     return false;
}
#else /*!USESHADOW*/
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

     my_seteuid(pwd->pw_uid);
     if (!*pwd->pw_dir || chdir(pwd->pw_dir) < 0) {
	  if (login_getcapbool(lc, "requirehome", 0)) {
	       QMessageBox::critical(NULL, NULL, 
				     i18n("Home directory not available"), 
				     i18n("&OK"));
	       return true;
	  }
     }
     my_seteuid(0);

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
	  loginEdit->setEnabled( false);
	  passwdEdit->setEnabled( false);
	  cancel_button_clicked();
	  timer->start( 2000, true );
	  return;
     }

     if (restrict()) {
	  setActiveWindow();
	  cancel_button_clicked();
	  return;
     }

     // Set session argument:
     verify->argv = parseArgs( verify->argv, 
			       sessiontags.at(
				    sessionargBox->currentItem()));

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
     MyApp myapp( argc, argv );
     /*printf("LANG=%s, Domain=%s, appName=%s\n", getenv("LANG"), 
	    klocale->language().data(), kapp->appName().data());*/
     QApplication::setOverrideCursor( waitCursor );
     kdmcfg = new KDMConfig( );
     
     myapp.setFont( *kdmcfg->normalFont());
     myapp.setStyle( kdmcfg->style());

     *dpy = qt_xdisplay();
     
     RegisterCloseOnFork (ConnectionNumber (*dpy));
     SecureDisplay (d, *dpy);
     // this is for a real BackSpace-key
     // It's a hack, remove it!
     /*
     KeySym mysym = XK_BackSpace;
     XChangeKeyboardMapping(*dpy, 22, 1, &mysym, 1);
     */
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
     qApp->restoreOverrideCursor();
     delete kdmcfg;
     //delete myapp;
     return Greet_Success;
}

/*
 * Local variables:
 * mode: c++
 * c-file-style: "k&r"
 * End:
 */
