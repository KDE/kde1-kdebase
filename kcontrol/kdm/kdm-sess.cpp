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

void KDMSessionsWidget::setupPage(QWidget *)
{
      QGroupBox *group0 = new QGroupBox( i18n("Allow to shutdown"), this );

      sdcombo = new QComboBox( FALSE, group0 );
      connect(sdcombo, SIGNAL(highlighted(int)), SLOT(slotSetAllowShutdown(int)));
      sdcombo->insertItem(klocale->translate("None"), 0);
      sdcombo->insertItem(klocale->translate("All"), 1);
      sdcombo->insertItem(klocale->translate("Root Only"), 2);
      sdcombo->insertItem(klocale->translate("Console Only"), 3);
      sdcombo->setCurrentItem(sdMode);
      sdcombo->setFixedSize(sdcombo->sizeHint());

      QGroupBox *group1 = new QGroupBox( klocale->translate("Commands"), this );
      QLabel *shutdown_label = new QLabel(klocale->translate("Shutdown"), group1);
      shutdown_label->setFixedSize(shutdown_label->sizeHint());
      shutdown_lined = new QLineEdit(group1);
      shutdown_lined->setFixedHeight(shutdown_lined->sizeHint().height());

      QLabel *restart_label = new QLabel(klocale->translate("Restart"), group1);
      restart_label->setFixedSize(restart_label->sizeHint());

      restart_lined = new QLineEdit(group1);
      restart_lined->setFixedHeight(shutdown_lined->height());

      QGroupBox *group2 = new QGroupBox( klocale->translate("Session types"), this );
      
      QLabel *type_label = new QLabel(klocale->translate("New type"), group2);
      type_label->setFixedSize(type_label->sizeHint());

      session_lined = new QLineEdit(group2);
      session_lined->setFixedHeight(session_lined->sizeHint().height());
      connect(session_lined, SIGNAL(returnPressed()),
              SLOT(slotAddSessionType()));

      QLabel *types_label = new QLabel(klocale->translate("Available types"), group2);
      types_label->setFixedSize(types_label->sizeHint());

      sessionslb = new QListBox(group2);
      sessionslb->insertStrList(&sessions);

      QButton *btnrm = new QPushButton( klocale->translate("Remove"), group2 );
      btnrm->setFixedSize(btnrm->sizeHint());
      connect( btnrm, SIGNAL( clicked() ), SLOT( slotRemoveSessionType() ) );

      QButton *btnadd = new QPushButton( klocale->translate("Add"), group2 );
      btnadd->setFixedSize(btnadd->sizeHint());
      connect( btnadd, SIGNAL( clicked() ), SLOT( slotAddSessionType() ) );

      QBoxLayout *main = new QVBoxLayout( this, 10 );
      QBoxLayout *lgroup0 = new QVBoxLayout( group0, 10 );

      QBoxLayout *lgroup1 = new QVBoxLayout( group1, 10 );
      QBoxLayout *lgroup1a = new QHBoxLayout();
      QBoxLayout *lgroup1b = new QHBoxLayout();

      QBoxLayout *lgroup2 = new QVBoxLayout( group2, 10 );
      QBoxLayout *lgroup2sub = new QHBoxLayout();
      QBoxLayout *lgroup2a = new QVBoxLayout();
      QBoxLayout *lgroup2b = new QVBoxLayout();

      main->addWidget(group0);
      main->addWidget(group1);
      main->addWidget(group2);

      lgroup0->addSpacing(10);
      lgroup0->addWidget(sdcombo);

      lgroup1->addSpacing(10);
      lgroup1->addLayout(lgroup1a);
      lgroup1->addLayout(lgroup1b);
      lgroup1a->addWidget(shutdown_label);
      lgroup1a->addWidget(shutdown_lined);

      lgroup1b->addWidget(restart_label);
      lgroup1b->addWidget(restart_lined);
      lgroup1->activate();

      lgroup2->addSpacing(10);
      lgroup2->addLayout(lgroup2sub);
      lgroup2sub->addLayout(lgroup2a);
      lgroup2sub->addLayout(lgroup2b);
      lgroup2a->addWidget(type_label, 10, AlignLeft);
      lgroup2a->addWidget(session_lined);
      lgroup2a->addWidget(btnrm, 10, AlignRight);
      lgroup2a->addWidget(btnadd, 10, AlignRight);
      lgroup2b->addWidget(types_label, 10, AlignLeft);
      lgroup2b->addWidget(sessionslb);
      lgroup2->activate();

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



