    /*

    Shutdown dialog
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
#ifdef KDE_PAM_SERVICE
#define KDE_PAM KDE_PAM_SERVICE
#else  
#define KDE_PAM "xdm"  /* default PAM service called by kdm */
#endif 
#else /* ! USE_PAM */
# ifdef USESHADOW
#  include <shadow.h>
# endif
#endif /* USE_PAM */
#include <sys/types.h>
#include <kapp.h>

#include "dm.h"

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

#ifdef PAM_MESSAGE_NONCONST
typedef struct pam_message pam_message_type;
#else
typedef const struct pam_message pam_message_type;
#endif

static int PAM_conv (int num_msg,
		     pam_message_type **msg,
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
#endif /* USE_PAM */
     struct passwd *pws = getpwnam( superuser);
     CHECK_PTR( pws);
#ifndef USE_PAM
#ifdef USESHADOW
     // If USESHADOW is defined, kdm will work for both shadow and non
     // shadow systems
     if( spws != NULL) {
       char* tmp = pws->pw_passwd;
       pws->pw_passwd = spws->sp_pwdp;
       spws->sp_pwdp = tmp;
     }
     endspent();
#endif /* USESHADOW */
     if( strcmp( crypt( pw, pws->pw_passwd), pws->pw_passwd)) {
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
     pam_error = pam_start(KDE_PAM, superuser, &PAM_conversation, &pamh);
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
     QString shutdownmsg =  klocale->translate( "Shutdown or restart?");
     if( mode == KDMConfig::RootOnly) {
	  shutdownmsg += '\n';
	  shutdownmsg += klocale->translate( "(Enter Root Password)");
     }
     label = new QLabel( shutdownmsg, winFrame);
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
     rb->setText( klocale->translate("Restart X Server"));//better description
     set_min( rb);
     rb->setFocusPolicy( StrongFocus);
     h += rb->height() + 10;
     w = QMAX( rb->width(), w);

     box->addWidget( rb);
     btGroup->insert( rb);

     // Passwd line edit
     if( mode == KDMConfig::RootOnly) {
	  pswdEdit = new QLineEdit( winFrame);
	  //set_min( pswdEdit);
	  pswdEdit->setMinimumHeight( pswdEdit->sizeHint().height());
	  pswdEdit->setEchoMode( QLineEdit::NoEcho);
	  /*QColorGroup   passwdColGroup(
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
	  */
	  pswdEdit->setFocusPolicy( StrongFocus);
	  pswdEdit->setFocus();
	  h+= pswdEdit->height() + 10;
	  box->addWidget( pswdEdit);
     }

     QBoxLayout* box3 = new QBoxLayout( QBoxLayout::LeftToRight, 10);
     box->addLayout( box3);

     okButton = new QPushButton( klocale->translate("OK"), winFrame);
     set_min( okButton);
     okButton->setFocusPolicy( StrongFocus);
     cancelButton = new QPushButton( klocale->translate("Cancel"), winFrame);
     set_min( cancelButton);
     //cancelButton->setDefault( true);
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
     if( cur_action) {
	  if( fork() == 0) {
	       sleep(1);
	       system( cur_action);
	       exit( UNMANAGE_DISPLAY);
	  } else {
	       exit( UNMANAGE_DISPLAY);
	  }
     } else
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

/*  
 * Local variables:  
 * mode: c++  
 * c-file-style: "k&r"  
 * End:  
*/
