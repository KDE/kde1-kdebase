#include <string.h>
#include <stdio.h>

#include <qapp.h>
#include <qpushbt.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qdialog.h>
#include <qaccel.h>
#include <qmsgbox.h>
#include <qchkbox.h>

#include "passworddialog.h"
#include "passworddialog.moc"
#include "config-kfm.h"

#include <klocale.h>
#include <kapp.h>

PasswordDialog::PasswordDialog(const char *head, QWidget* parent, const char* name, bool modal, WFlags wflags)
   : QDialog(parent, name, modal, wflags)
{
    debugT("Here we go!!\n");
    
   _head = head;

   // (David) Layout management
   QVBoxLayout *vLay = new QVBoxLayout(this, 10 /* border */, 5);

   //
   // Bei Bedarf einen kleinen Kommentar als Label einfuegen
   // English please !!! David.
   if (_head)
   {
      QLabel *l;
      
      l = new QLabel(_head, this);
      l->adjustSize();
      l->setMinimumSize(l->size());
      vLay->addWidget(l);
      // l->setGeometry( 10, 10, 200, 20 );
   }

   // The horizontal layout for label + lineedit
   QHBoxLayout *hLay = new QHBoxLayout(5);
   vLay->addLayout(hLay);
   //
   // Die eine oder zwei Zeile(n) mit der Passwortabfrage
   //
   QLabel *l_password = new QLabel(klocale->translate("Password"), this);
   l_password->adjustSize();
   l_password->setMinimumSize(l_password->size());
   hLay->addWidget(l_password,0);
   //l_password->setGeometry( 10, 40, 80, 30 );
   
   _w_password = new QLineEdit( this );
   //_w_password->setGeometry( 90, 40, 100, 30 );
   _w_password->setEchoMode( QLineEdit::Password );
   _w_password->adjustSize();
   _w_password->setFixedHeight(_w_password->height());
   hLay->addWidget(_w_password,10);
   
   //
   // Connect vom LineEdit herstellen und Accelerator
   //
   QAccel *ac = new QAccel(this);
   ac->connectItem( ac->insertItem(Key_Escape), this, SLOT(reject()) );
   
   connect( _w_password, SIGNAL(returnPressed()), SLOT(accept()) );
   
   //
   // Eine vertikale Linie erzeugen
   //
   QFrame *f = new QFrame(this);
   f->setLineWidth(1);
   f->setMidLineWidth(1);
   f->setFrameStyle( QFrame::HLine|QFrame::Raised);
   //f->setGeometry( 10, 80, 180, 2 );
   f->setMinimumHeight(5);
   vLay->addWidget(f,0);
   
   //
   // Die Buttons "OK" & "Cancel" erzeugen
   //
   QHBoxLayout *hBLay = new QHBoxLayout(5);
   vLay->addLayout(hBLay);

   hBLay->addStretch();

   QPushButton *b1, *b2;
   b1 = new QPushButton(klocale->translate("OK"), this);
   //b1->setGeometry( 10, 90, 80, 30 );
   b1->adjustSize();
   b1->setFixedSize(b1->size());   
   hBLay->addWidget(b1);

   hBLay->addStretch();
   //or hBLay->addSpacing(10);
   
   b2 = new QPushButton(klocale->translate("Cancel"), this);
   //b2->setGeometry( 110, 90, 80, 30 );
   b2->adjustSize();
   b2->setFixedSize(b2->size());   
   hBLay->addWidget(b2);

   hBLay->addStretch();
   
   // Buttons mit Funktionaliataet belegen
   connect( b1, SIGNAL(clicked()), SLOT(accept()) );
   connect( b2, SIGNAL(clicked()), SLOT(reject()) );
   
   // Fenstertitel
   setCaption(klocale->translate("Password"));
   
   // Focus
   _w_password->setFocus();
   
   vLay->activate();
   setMinimumSize(200, 50);
   setMaximumSize(300, 100);

   debugT("Hi folks, thats it\n");
}

const char * PasswordDialog::password()
{
   if ( _w_password )
      return _w_password->text();
   else
      return "";
}



