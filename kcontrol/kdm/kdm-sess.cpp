/* This file is part of the KDE Display Manager Configuration package
    Copyright (C) 1997-1998 Thomas Tanghus (tanghus@earthling.net)

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/  

#include "utils.h"
#include "kdm-sess.moc"



// Destructor
KDMSessionsWidget::~KDMSessionsWidget()
{
  if(gui)
  {
    delete restart_lined;
    delete shutdown_lined;
    delete session_lined;
    delete sdcombo;
    delete sessionslb;
  }
}


KDMSessionsWidget::KDMSessionsWidget(QWidget *parent, const char *name, bool init)
  : KConfigWidget(parent, name)
{
      gui = !init;
      loadSettings();      
      if(gui)
        setupPage(parent);
}

void KDMSessionsWidget::setupPage(QWidget *p)
{
      QLabel *a_label = new QLabel( klocale->translate("Allow to shutdown:"), this);
      a_label->setMinimumSize(a_label->sizeHint());

      sdcombo = new QComboBox( FALSE, this );
      connect(sdcombo, SIGNAL(highlighted(int)), SLOT(slotSetAllowShutdown(int)));
      sdcombo->move(a_label->width()+20, 20);
      sdcombo->insertItem(klocale->translate("None"), 0);
      sdcombo->insertItem(klocale->translate("All"), 1);
      sdcombo->insertItem(klocale->translate("Root Only"), 2);
      sdcombo->insertItem(klocale->translate("Console Only"), 3);
      sdcombo->setCurrentItem(sdMode);
      sdcombo->setFixedSize(sdcombo->sizeHint());

      QGroupBox *group1 = new QGroupBox( klocale->translate("Commands"), this );
      //group1->move( 5, sdcombo->y()+sdcombo->height()+10 );
      QLabel *label = new QLabel(klocale->translate("Shutdown"), group1);
      label->move(5, 20);
      label->setFixedSize(label->sizeHint());
      shutdown_lined = new QLineEdit(group1);
      shutdown_lined->setGeometry(label->width()+10,20,
                       p->width()-(label->width()+50 ),
                       shutdown_lined->sizeHint().height());

      label = new QLabel(klocale->translate("Restart"), group1);
      label->move(5, shutdown_lined->y()+shutdown_lined->height()+10);
      label->adjustSize();

      restart_lined = new QLineEdit(group1);
      restart_lined->setGeometry(shutdown_lined->x(),label->y(),
                       shutdown_lined->width(),shutdown_lined->height());

      group1->adjustSize();
      group1->setMinimumSize(group1->size());

      //int y = group->y() + group->height();
      QGroupBox *group2 = new QGroupBox( klocale->translate("Session types"), this );
      //group->move(5, y+10);
      
      label = new QLabel(klocale->translate("New type"), group2);
      label->move(5, 20);
      label->adjustSize();

      session_lined = new QLineEdit(group2);
      session_lined->setGeometry(5,label->height()+30,
                       (p->width()/2)-50,session_lined->sizeHint().height());
      connect(session_lined, SIGNAL(returnPressed()),
              SLOT(slotAddSessionType()));

      label = new QLabel(klocale->translate("Available types"), group2);
      label->move(session_lined->width()+15, 20);
      label->adjustSize();

      sessionslb = new QListBox(group2);
      sessionslb->setGeometry(session_lined->width()+15,
              session_lined->y(),(p->width()/2)-20, 100);
      sessionslb->insertStrList(&sessions);

      QButton *btnrm = new QPushButton( klocale->translate("Remove"), group2 );
      btnrm->adjustSize();
      btnrm->move( (session_lined->width()+5)-btnrm->width(),
                    session_lined->y()+session_lined->height()+10);
      connect( btnrm, SIGNAL( clicked() ), SLOT( slotRemoveSessionType() ) );
      QButton *btnadd = new QPushButton( klocale->translate("Add"), group2 );
      btnadd->setGeometry( btnrm->x(), btnrm->y()+btnrm->height()+10,
                           btnrm->width(), btnrm->height());
      connect( btnadd, SIGNAL( clicked() ), SLOT( slotAddSessionType() ) );

      group2->adjustSize();
      group2->setMinimumSize(group2->size());

      QBoxLayout *main = new QVBoxLayout( this, 10 );
      QBoxLayout *box1 = new QHBoxLayout();
      QBoxLayout *box2 = new QHBoxLayout();
      QBoxLayout *box3 = new QHBoxLayout();

      main->addLayout(box1);
      main->addLayout(box2);
      main->addLayout(box3);

      box1->addWidget(a_label);
      box1->addWidget(sdcombo);
      box1->addStretch(1);

      box2->addWidget(group1);
      box3->addWidget(group2);
      main->addStretch(1);
      main->activate();
}

void KDMSessionsWidget::slotAddSessionType()
{
  if(strlen(session_lined->text()) > 0)
  {
    sessionslb->inSort(session_lined->text());
    session_lined->setText("");
  }
}

void KDMSessionsWidget::slotRemoveSessionType()
{
  int i = sessionslb->currentItem();
  if(i > -1)
    sessionslb->removeItem(i);
}

void KDMSessionsWidget::slotSetAllowShutdown(int s)
{
  sdMode = s;
}

void KDMSessionsWidget::applySettings()
{
  //debug("KDMSessionsWidget::applySettings()");
  QString fn(CONFIGFILE);
  KSimpleConfig *c = new KSimpleConfig(fn);

  c->setGroup("KDM");

  if(strlen(shutdown_lined->text()) > 0)
    c->writeEntry("ShutDown", shutdown_lined->text(), true);
  if(strlen(restart_lined->text()) > 0)
    c->writeEntry("Restart", restart_lined->text(), true);

  // write shutdown auth
  switch ( sdMode )
  {
    case Non:
	c->writeEntry( "ShutDownButton", "None" );
	break;
    case All:
	c->writeEntry( "ShutDownButton", "All" );
	break;
    case RootOnly:
	c->writeEntry( "ShutDownButton", "RootOnly" );
	break;
    case ConsoleOnly:
	c->writeEntry( "ShutDownButton", "ConsoleOnly" );
	break;
    default:
	break;
  }

  if(sessionslb->count() > 0)
  {
    QString sesstr;
    for(uint i = 0; i < sessionslb->count(); i++)
    {
      sesstr.append(sessionslb->text(i));
      sesstr.append(";");
    }
    c->writeEntry( "SessionTypes", sesstr.data() );
  }

  delete c;
}

void KDMSessionsWidget::loadSettings()
{
  QString fn(CONFIGFILE), str;
  
  // Get config object
  KSimpleConfig *c = new KSimpleConfig(fn);
  c->setGroup("KDM");

  // read restart and shutdown cmds
  restartstr = c->readEntry("Restart", "/sbin/reboot");
  shutdownstr = c->readEntry("Shutdown", "/sbin/shutdown");

  str = c->readEntry("ShutDownButton", "All");
  if(str == "All")
    sdMode = All;
  else if(str == "None")
    sdMode = Non;
  else if(str == "RootOnly")
    sdMode = RootOnly;
  else
    sdMode = ConsoleOnly;

  str = c->readEntry( "SessionTypes");
  if(!str.isEmpty())
    semsplit( str, sessions);	  
  //for(uint i = 0; i < sessions.count(); i++)
    //debug("session type: %s", sessions.at(i));

  delete c;
}



