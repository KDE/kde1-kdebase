/* This file is part of the KDE Display Manager Configuration package
    Copyright (C) 1997 Thomas Tanghus (tanghus@earthling.net)

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
#include "kdm-users.moc"



// Destructor
KDMUsersWidget::~KDMUsersWidget()
{
  if(gui)
  {
    delete userbutton;
    delete userpixdrop;
    delete usrGroup;
    delete shwGroup;
  }
}

KDMUsersWidget::KDMUsersWidget(QWidget *parent, const char *name, bool init)
  : KConfigWidget(parent, name)
{
      gui = !init;
      loadSettings();
      if(gui)
        setupPage(parent);
}

void KDMUsersWidget::setupPage(QWidget *)
{
      QLabel *label;
      label = new QLabel(klocale->translate("All users"), this);
      label->move( 10, 20 );
      label = new QLabel(klocale->translate("Selected users"), this);
      label->move( 170, 20 );
      label = new QLabel(klocale->translate("No-show users"), this);
      label->move( 170, 190 );

      QPushButton *all_to_no, *all_to_usr, *no_to_all, *usr_to_all;

      all_to_usr = new QPushButton( ">>", this );
      all_to_usr->setGeometry( 135, 50, 30, 30);
      connect( all_to_usr, SIGNAL( clicked() ), SLOT( slotAllToUsr() ) );

      usr_to_all = new QPushButton( "<<", this );
      usr_to_all->setGeometry( 135, 135, 30, 30);
      connect( usr_to_all, SIGNAL( clicked() ), SLOT( slotUsrToAll() ) );

      all_to_no  = new QPushButton( ">>", this );
      all_to_no->setGeometry( 135, 215, 30, 30);
      connect( all_to_no, SIGNAL( clicked() ), SLOT( slotAllToNo() ) );

      no_to_all = new QPushButton( "<<", this );
      no_to_all->setGeometry( 135, 320, 30, 30);
      connect( no_to_all, SIGNAL( clicked() ), SLOT( slotNoToAll() ) );

      QRadioButton *rb;

      usrGroup = new QButtonGroup( this );
      usrGroup->setExclusive( TRUE );

      rb = new QRadioButton( 
         klocale->translate("Show only\nselected users"), usrGroup );
      rb->setGeometry( 10, 10, 140, 25 );
      if(!showallusers)
        rb->setChecked(true);
      usrGroup->insert( rb, 0 );

      rb = new QRadioButton( 
         klocale->translate("Show all users\n but no-show users"), usrGroup );
      rb->setGeometry( 10, 50, 140, 25 );
      if(showallusers)
        rb->setChecked(true);
      usrGroup->insert( rb, 1 );
      usrGroup->setGeometry( 300, 160, 160, 90);
      connect( usrGroup, SIGNAL( clicked( int ) ), SLOT( slotUserShowMode( int ) ) );

      shwGroup = new QButtonGroup( this );
      cbusrshw = new QCheckBox(
         klocale->translate("Show users"), shwGroup);
      cbusrshw->setGeometry( 10, 10, 120, 25 );
      if(showusers)
        cbusrshw->setChecked(true);
      connect( cbusrshw, SIGNAL( toggled( bool ) ), SLOT( slotUserShow( bool ) ) );
      cbusrsrt = new QCheckBox(
         klocale->translate("Sort users"), shwGroup);
      cbusrsrt->setGeometry( 10, 50, 120, 25 );
      if(sortusers)
        cbusrsrt->setChecked(true);
      connect( cbusrsrt, SIGNAL( toggled( bool ) ), SLOT( slotUserSort( bool ) ) );
      shwGroup->insert( cbusrshw, 0);
      shwGroup->insert( cbusrsrt, 1);
      shwGroup->setGeometry( 300, 270, 160, 80);

      alluserlb = new QListBox(this);
      alluserlb->insertStrList(&allusers);
      alluserlb->setGeometry(10, 50, 120, 300);
      userlb = new QListBox(this);
      userlb->insertStrList(&users);
      userlb->setGeometry(170, 50, 120, 135);
      nouserlb = new QListBox(this);
      nouserlb->insertStrList(&no_users);
      nouserlb->setGeometry(170, 215, 120, 135);

      connect( userlb, SIGNAL( highlighted( int ) ),
                    SLOT( slotUserSelected( int ) ) );
      connect( alluserlb, SIGNAL( highlighted( int ) ),
                    SLOT( slotUserSelected( int ) ) );
      connect( nouserlb, SIGNAL( highlighted( int ) ),
                    SLOT( slotUserSelected( int ) ) );

      userlabel = new QLabel( this );
      userlabel->setGeometry(300, 50, 160, 25);

      userbutton = new KIconLoaderButton(iconloader, this);
      userbutton->setIcon("default.xpm");
      userbutton->move(300, 80);
      userbutton->setMaximumSize(80, 80);
      connect(userbutton, SIGNAL(iconChanged(const char*)),
              SLOT(slotUserPixChanged(const char*)));
      userbutton->adjustSize();
      QToolTip::add(userbutton, klocale->translate("Click or drop an image here"));
      userpixdrop = new KDNDDropZone(userbutton, DndURL);
      connect(userpixdrop, SIGNAL(dropAction(KDNDDropZone*)),
              SLOT(slotPixDropped(KDNDDropZone*)));
}

void KDMUsersWidget::slotUserPixChanged(const char*)
{
  QString msg, user(userlabel->text());
  if(user.isEmpty())
  {
    if(KMsgBox::yesNo(this, klocale->translate("No user selected"),
       klocale->translate("Save image as default image?")))
      user = "default";
    else
      return;
  }
  QString userpix = USERPIXDIR + user + ".xpm";
  const QPixmap *p = userbutton->pixmap();
  if(!p)
    return;
  if(!p->save(userpix, "XPM"))
  {
    msg  = klocale->translate("There was an error saving the image:\n>");
    msg += userpix;
    msg += klocale->translate("<");
    KMsgBox::message(this, klocale->translate("ERROR"), msg);
  }
  userbutton->adjustSize();
}

void KDMUsersWidget::slotPixDropped(KDNDDropZone *zone)
{
  // Find the widget on which the object was dropped
  QWidget *w = zone->getWidget();
  // Get the url
  KURL url(zone->getData());
  QString filename = url.filename();
  QString msg, userpixname;
  QString pixurl("file:"+kapp->kdedir()+"/share/apps/kdm/pics/"); 
  QString user(userlabel->text());
  QString userpixurl = pixurl + "users/";
  int last_dot_idx = filename.findRev('.');
  bool istmp = false;

  // CC: Now check for the extension
  QString ext(".xpm .xbm");
//#ifdef HAVE_LIBGIF
  ext += " .gif";
//#endif
#ifdef HAVE_LIBJPEG
  ext += " .jpg";
#endif

  if( !ext.contains(filename.right(filename.length()-last_dot_idx), false) )
  {
    msg =  klocale->translate("Sorry, but \n");
    msg += filename;
    msg += klocale->translate("\ndoes not seem to be an image file");
    msg += klocale->translate("\nPlease use files with these extensions\n");
    msg += ext;
    KMsgBox::message( this, klocale->translate("Improper File Extension"), msg);
  }
  else
  {
    // we gotta check if it is a non-local file and make a tmp copy at the hd.
    if(strcmp(url.protocol(), "file") != 0)
    {
      pixurl += url.filename();
      KFM *kfm = new KFM();
      kfm->copy(url.url().data(), pixurl.data());
      delete kfm;
      url = pixurl;
      istmp = true;
    }
    // By now url should be "file:/..."
    if(w == userbutton) // the drop is on a user
    {
      if(user.isEmpty())
      {
        if(KMsgBox::yesNo(this, klocale->translate("No user selected"),
           klocale->translate("Save image as default image?")))
          user = "default";
        else
          return;
      }
      // Finally we've got an image file to add to the user
      QPixmap p(url.path());
      if(!p.isNull())
      { // Save image as <userpixdir>/<user>.<ext>
        userbutton->setPixmap(p);
        userbutton->adjustSize();
        userpixurl += user;
        userpixurl += filename.right( filename.length()-(last_dot_idx) );
        //debug("destination: %s", userpixurl.data());
        // Let KFM copy the file. NB: network transparent
        KFM *kfm = new KFM();
        if(istmp)
          kfm->move(url.url(), userpixurl.data());
        else
          kfm->copy(url.url(), userpixurl.data());
        delete kfm;
      }
      else
      {
        msg  = klocale->translate("There was an error loading the image:\n>");
        msg += url.path();
        msg += klocale->translate("<\nIt will not be saved...");
        KMsgBox::message(this, klocale->translate("ERROR"), msg);
      }
    }
  }
}

void KDMUsersWidget::slotUserShow(bool show)
{
  showusers = show;
}

void KDMUsersWidget::slotUserSort(bool sort)
{
  sortusers = sort;
}

void KDMUsersWidget::slotAllToNo()
{
  int id = alluserlb->currentItem();
  if(id > -1)
  {
    nouserlb->inSort(alluserlb->text(id));
    alluserlb->removeItem(id);
  }
}

void KDMUsersWidget::slotNoToAll()
{
  int id = nouserlb->currentItem();
  if(id > -1)
  {
    alluserlb->inSort(nouserlb->text(id));
    nouserlb->removeItem(id);
  }
}

void KDMUsersWidget::slotAllToUsr()
{
  int id = alluserlb->currentItem();
  if(id > -1)
  {
    userlb->inSort(alluserlb->text(id));
    alluserlb->removeItem(id);
  }
}

void KDMUsersWidget::slotUsrToAll()
{
  int id = userlb->currentItem();
  if(id > -1)
  {
    alluserlb->inSort(userlb->text(id));
    userlb->removeItem(id);
  }
}

void KDMUsersWidget::slotUserShowMode( int m )
{
  showallusers = m;
}

void KDMUsersWidget::slotUserSelected(int)
{
  QString user_pix_dir(kapp->kdedir()+"/share/apps/kdm/pics/users/"); 
  QString name;
  QPixmap default_pix( user_pix_dir + "default.xpm");
  QListBox *lb;

  // Get the listbox with the focus
  // If this is not a listbox we segfault :-(
  QWidget *w = kapp->focusWidget();

  // Maybe this is enough?
  if(w->isA("QListBox"))
  {
    kapp->processEvents();
    // There seems to be an error in QListBox. When a listbox item is selected
    // for the first time it emits a "highlighted()" signal with the correct
    // index an emmidiatly after a signal with index 0.
    // Therefore I have to use currentItem instead of the index emitted.
    lb = (QListBox*)w;
    name = user_pix_dir + lb->text(lb->currentItem()) + ".xpm";
    QPixmap p( name.data());
    if(p.isNull())
      p = default_pix;
    userbutton->setPixmap(p);
    userbutton->adjustSize();
    userlabel->setText(lb->text(lb->currentItem()));
  }
  else
    debug("Not a QListBox");
}

void KDMUsersWidget::applySettings()
{
  //debug("KDMUsersWidget::applySettings()");
  QString fn(CONFIGFILE);
  KSimpleConfig *c = new KSimpleConfig(fn);

  c->setGroup("KDM");

  c->writeEntry( "UserView", showusers );
  c->writeEntry( "SortUsers", sortusers );

  if(nouserlb->count() > 0)
  {
    QString nousrstr;
    for(uint i = 0; i < nouserlb->count(); i++)
    {
      nousrstr.append(nouserlb->text(i));
      nousrstr.append(";");
    }
    c->writeEntry( "NoUsers", nousrstr.data() );
  }

  if((userlb->count() > 0) && (!showallusers))
  {
    QString usrstr;
    for(uint i = 0; i < userlb->count(); i++)
    {
      usrstr.append(userlb->text(i));
      usrstr.append(";");
    }
    c->writeEntry( "Users", usrstr.data() );
  }

  delete c;
}

void KDMUsersWidget::loadSettings()
{
  iconloader = kapp->getIconLoader();
  QString fn(CONFIGFILE), str;
  
  // Get config object
  KSimpleConfig *c = new KSimpleConfig(fn);
  c->setGroup("KDM");

  // Read users from kdmrc and /etc/passwd
  str = c->readEntry( "Users");
  if(!str.isEmpty())
  {
    semsplit( str, users);
    //for(uint i = 0; i < users.count(); i++)
      //debug("user: %s", users.at(i));
    showallusers = false;
  }
  else
    showallusers = true;
  str = c->readEntry( "NoUsers");
  if(!str.isEmpty())
    semsplit( str, no_users);	  
  //for(uint i = 0; i < no_users.count(); i++)
    //debug("nouser: %s", no_users.at(i));

  struct passwd *ps;
#define CHECK_STRING( x) (x != 0 && x[0] != 0)
  setpwent();
  while( (ps = getpwent()) != 0)
  {
    kapp->processEvents(50);
    if( CHECK_STRING(ps->pw_dir) && CHECK_STRING(ps->pw_shell) &&
      CHECK_STRING(ps->pw_gecos) && ( no_users.contains( ps->pw_name) == 0))
    {
      kapp->processEvents(50);
      // we might have a real user, insert him/her
      allusers.inSort( ps->pw_name);
    }
    //ps = getpwent();
  }
  endpwent();
#undef CHECK_STRING

  sortusers = c->readNumEntry("SortUsers", true);
  showusers = c->readNumEntry("UserView", true);

  delete c;
}


