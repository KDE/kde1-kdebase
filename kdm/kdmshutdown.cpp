//                              -*- Mode: C++ -*- 
// Title            : kdmshutdown.cpp
// 
// Description      : Shutdown dialog
// Author           : Steffen Hansen
// Created On       : Mon Apr 28 21:52:11 1997
// Last Modified By : Steffen Hansen
// Last Modified On : Fri Jul 11 14:16:14 1997
// Update Count     : 12
// Status           : Unknown, Use with caution!
// 

#include "kdmshutdown.h"
#include "kdmconfig.h" // for shutdown-modes
#include <pwd.h>
#ifdef HAVE_CRYPT_H
# include <crypt.h>
#endif
#ifdef USE_PAM
extern "C" {
# include <security/pam_appl.h>
}
#else /* ! USE_PAM */
# ifdef USESHADOW
#  include <shadow.h>
# endif
#endif USE_PAM
#include <sys/types.h>
#include <kapp.h>

extern "C" {
#include "dm.h"
}

static inline void
set_min( QWidget* w)
{
     w->adjustSize();
     w->setMinimumSize( w->size());
}
static inline void
set_fixed( QWidget* w)
{
     w->adjustSize();
     w->setFixedSize( w->size());
}

#ifdef USE_PAM
static const char *PAM_password;

static int PAM_conv (int num_msg,
		     const struct pam_message **msg,
		     struct pam_response **resp,
		     void *) {
     int count = 0, replies = 0;
     struct pam_response *reply = NULL;
     int size = sizeof(struct pam_response);
     
     #define GET_MEM if (reply) realloc(reply, size);\
                     else reply = (struct pam_response *)malloc(size); \
	             if (!reply) return PAM_CONV_ERR; \
	             size += sizeof(struct pam_response)
     #define COPY_STRING(s) (s) ? strdup(s) : (char *)NULL

     for (count = 0; count < num_msg; count++) {
	  switch (msg[count]->msg_style) {
	  case PAM_PROMPT_ECHO_ON:
	       /* user name given to PAM already */
	       return PAM_CONV_ERR;
	  case PAM_PROMPT_ECHO_OFF:
	       /* wants password */
	       GET_MEM;
	       reply[replies].resp_retcode = PAM_SUCCESS;
	       reply[replies++].resp = COPY_STRING(PAM_password);
	       /* PAM frees resp */
	       break;
	  case PAM_TEXT_INFO:
	       break;
	  default:
	       /* unknown or PAM_ERROR_MSG */
	       if (reply) free (reply);
	       return PAM_CONV_ERR;
	  }
     }
     if (reply) *resp = reply;
     return PAM_SUCCESS;
}

static struct pam_conv PAM_conversation = {
	&PAM_conv,
	NULL
};
#endif

static bool
verify_root_pw( const char* pw)
{
     const char* superuser = "root";
#ifdef USESHADOW
     struct spwd *spws = getspnam( superuser);
#endif
#ifdef USE_PAM
     pam_handle_t *pamh;
     int pam_error;
#endif USE_PAM
     struct passwd *pws = getpwnam( superuser);
     CHECK_PTR( pws);
#ifndef USE_PAM
#ifdef USESHADOW
     if (spws == NULL) {
	  printf("getspnam() failed.  Are you root?\n");
	  return false;
     }
     endspent();
     
     if( strcmp( crypt( pw, spws->sp_pwdp), spws->sp_pwdp)) {
#else
     if( strcmp( crypt( pw, pws->pw_passwd), pws->pw_passwd)) {
#endif /* USESHADOW */
	  printf("Root passwd verification failed\n");
	  
	  return false;
     }
#else /* USE_PAM */
     #define PAM_BAIL \
        if (pam_error != PAM_SUCCESS) { \
	pam_end(pamh, 0); \
        return false; \
      }
     PAM_password = pw;
     pam_error = pam_start("xdm", superuser, &PAM_conversation, &pamh);
     PAM_BAIL;
     pam_error = pam_authenticate( pamh, 0);
     PAM_BAIL;
     /* OK, if we get here, the user _should_ be root */
     pam_end( pamh, PAM_SUCCESS);
#endif /* USE_PAM */
     return true;
}

KDMShutdown::KDMShutdown( int mode, QWidget* _parent, const char* _name,
			  const char* _shutdown, 
			  const char* _restart)
     : FDialog( _parent, _name, true)
{
     shutdown = _shutdown;
     restart  = _restart;
     int h = 10, w = 0;
     QFrame* winFrame = new QFrame( this);
     winFrame->setFrameStyle( QFrame::WinPanel | QFrame::Raised);
     QBoxLayout* box = new QBoxLayout( winFrame, QBoxLayout::TopToBottom, 
				       10, 10);
     label = new QLabel( klocale->translate("Shutdown or restart?"), winFrame);
     set_fixed( label);
     h += label->height() + 10;
     w = label->width();

     box->addWidget( label, 0, AlignCenter);

     QFrame* sepFrame = new QFrame( this);
     sepFrame->setFrameStyle( QFrame::HLine| QFrame::Sunken);
     h += sepFrame->height(); 
     box->addWidget( sepFrame);

     btGroup = new QButtonGroup( /* this */);
     
     QRadioButton *rb;
     rb = new QRadioButton( winFrame /*btGroup*/);
     rb->setText( klocale->translate("Shutdown"));
     set_min( rb);
     rb->setFocusPolicy( StrongFocus);
     // Default action
     rb->setChecked( true);
     rb->setFocus();
     cur_action = shutdown;
     
     h += rb->height() + 10;
     w = QMAX( rb->width(), w);

     box->addWidget( rb);
     btGroup->insert( rb);
     rb = new QRadioButton( winFrame /*btGroup*/);
     rb->setText( klocale->translate("Shutdown and restart"));
     set_min( rb);
     rb->setFocusPolicy( StrongFocus);
     h += rb->height() + 10;
     w = QMAX( rb->width(), w);

     box->addWidget( rb);
     btGroup->insert( rb);
     rb = new QRadioButton( winFrame /*btGroup*/);
     rb->setText( klocale->translate("Exit kdm"));
     set_min( rb);
     rb->setFocusPolicy( StrongFocus);
     h += rb->height() + 10;
     w = QMAX( rb->width(), w);

     box->addWidget( rb);
     btGroup->insert( rb);

     // Passwd line edit
     if( mode == KDMConfig::RootOnly) {
	  pswdEdit = new QLineEdit( winFrame);
	  set_min( pswdEdit);
	  //pswdEdit->setEchoMode( QLineEdit::Password);
	  QColorGroup   passwdColGroup(
	       QApplication::palette()->normal().foreground(),
	       QApplication::palette()->normal().background(),
	       QApplication::palette()->normal().light(),
	       QApplication::palette()->normal().dark(),
	       QApplication::palette()->normal().mid(),
	       QApplication::palette()->normal().base(),
	       QApplication::palette()->normal().base());
	  QPalette passwdPalette( passwdColGroup, passwdColGroup, 
				  passwdColGroup);
	  pswdEdit->setPalette( passwdPalette);
	  pswdEdit->setFocusPolicy( StrongFocus);
	  pswdEdit->setFocus();
	  h+= pswdEdit->height() + 10;
	  box->addWidget( pswdEdit);
     }

     QBoxLayout* box3 = new QBoxLayout( QBoxLayout::LeftToRight, 10);
     box->addLayout( box3);

     okButton = new QPushButton( klocale->translate("Ok"), winFrame);
     set_min( okButton);
     okButton->setFocusPolicy( StrongFocus);
     cancelButton = new QPushButton( klocale->translate("Cancel"), winFrame);
     set_min( cancelButton);
     cancelButton->setDefault( true);
     cancelButton->setFocusPolicy( StrongFocus);
     h += cancelButton->height() + 10;
     w = QMAX( (okButton->width() + 10 + cancelButton->width()), w);

     box3->addWidget( okButton);
     box3->addWidget( cancelButton);
     // Connections
     connect( okButton, SIGNAL(clicked()), SLOT(bye_bye()));
     connect( cancelButton, SIGNAL(clicked()), SLOT(reject()));
     connect( btGroup, SIGNAL(clicked(int)), SLOT(rb_clicked(int)));
     if( mode == KDMConfig::RootOnly) {
	  okButton->setEnabled( false);
	  connect( pswdEdit, SIGNAL( returnPressed()), this, SLOT( pw_entered()));
     } else
	  cancelButton->setFocus();
     resize( 20 + w, h);
     winFrame->setGeometry( 0, 0, width(), height());
}

void
KDMShutdown::rb_clicked( int id)
{
     /*    if( id)
	  cur_action = restart;
     else
	  cur_action = shutdown;
	  */
     switch( id) {
     case 0:
	  cur_action = shutdown;
	  break;
     case 1:
	  cur_action = restart;
	  break;
     case 2:
	  cur_action = 0L;
	  break;
     }
}

void
KDMShutdown::pw_entered()
{
     if( verify_root_pw( pswdEdit->text())) {
	  okButton->setEnabled( true);
     } else {
	  okButton->setEnabled( false);
     }
     pswdEdit->setText("");
}

void
KDMShutdown::bye_bye()
{
     if( cur_action)
	  system( cur_action);
     else
	  exit( RESERVER_DISPLAY);
}

#include "kdmshutdown.moc"

#ifdef TEST_KDM_SHUTDOWN

#include <qapp.h>

int main(int argc, char **argv)
{
     QApplication app( argc, argv);
     app.setFont( QFont( "helvetica", 18));
     KDMShutdown sd( 0, 0,"Hej", "echo shutdown", "echo restart");
     app.setMainWidget( &sd);
     return sd.exec();
}

#endif /* TEST_KDM */
