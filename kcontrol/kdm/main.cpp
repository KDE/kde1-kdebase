/*
  This file is part of the KDE Display Manager Configuration package
  Copyright (C) 1997 Thomas Tanghus (tanghus@earthling.net)
  
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


#include "utils.h"
#include "kdm-appear.h"
#include "kdm-font.h"
#include "kdm-bgnd.h"
#include "kdm-users.h"
#include "kdm-sess.h"
#include <kwm.h>


class KDMConfigApplication : public KControlApplication
{
public:

  KDMConfigApplication(int &argc, char **arg, const char *name);

  void apply();

private:

  KDMAppearanceWidget *appearance;
  KDMFontWidget       *font;
  KDMBackgroundWidget *background;
  KDMUsersWidget      *users;
  KDMSessionsWidget   *sessions;
  QStrList            *pages;
};


KDMConfigApplication::KDMConfigApplication(int &argc, char **argv, const char *name)
  : KControlApplication(argc, argv, name)
{
  appearance = 0;
  font = 0;
  background = 0;
  users = 0;
  sessions = 0;

  pages = getPageList();

  if (runGUI())
  {
    KWM::setMiniIcon(KWM::activeWindow(), kapp->getMiniIcon());
    KWM::setIcon(KWM::activeWindow(), kapp->getIcon());
    //if(geteuid() != 0)
    QString fn(CONFIGFILE);
    //debug("KDMAppl. Configfile: %s", fn.data());
    QFileInfo fi(fn.data());
    //if(!fi.exists())
      //debug("Doesn't exist");
    if(fi.isReadable() && fi.isWritable())
    {
/*
#ifdef HAVE_LIBGIF
      QImageIO::defineIOHandler("GIF", "^GIF[0-9][0-9][a-z]", 
  			      0, read_gif_file, NULL);
#endif
*/
#ifdef HAVE_LIBJPEG
      QImageIO::defineIOHandler("JFIF","^\377\330\377\340", 
			      0, read_jpeg_jfif, NULL);
#endif

      KIconLoader *iconloader = kapp->getIconLoader();
      if(iconloader)
      {
        QString idir(kapp->kdedir()+"/share/apps/kdm/pics/users");
        iconloader->insertDirectory(0, idir.data());
        idir = kapp->kdedir()+"/share/apps/kdm/pics";
        iconloader->insertDirectory(0, idir.data());
        idir = kapp->kdedir()+"/share/wallpapers";
        iconloader->insertDirectory(0, idir.data());
        //idir = kapp->kdedir()+"/share/icons";
        //iconloader->insertDirectory(0, idir.data());
      }

      if (!pages || pages->contains("appearance"))
	  addPage(appearance = new KDMAppearanceWidget(dialog, "appearance", FALSE),
		  klocale->translate("&Appearance"), 
		  "kdm-appear.html");
      if (!pages || pages->contains("font"))
        addPage(font = new KDMFontWidget(dialog, "font", FALSE),
		klocale->translate("&Fonts"),
		"kdm-fonts.html");
      if (!pages || pages->contains("background"))
	  addPage(background = new KDMBackgroundWidget(dialog, "background", FALSE),
		  klocale->translate("&Background"), "kdm-backgnd.html");
      if (!pages || pages->contains("users"))
        addPage(users = new KDMUsersWidget(dialog, "users", FALSE),
                                  klocale->translate("&Users"), "kdm-users.html");
      if (!pages || pages->contains("sessions"))
        addPage(sessions = new KDMSessionsWidget(dialog, "sessions", FALSE),
                                  klocale->translate("&Sessions"), "kdm-sess.html");

      dialog->show();
    }
    else
    {
      QString msg = klocale->translate("Sorry, but you don't have read/write\n"
				       "permission to the KDM setup file.");
      KMsgBox::message( dialog, klocale->translate("Missing privileges"), msg);
      this->exit(-1);
    }
  }
}

/*
void KDMConfigApplication::init()
{
  KDMConfigWidget *kdmconfig = new KDMConfigWidget(0, 0, TRUE);
  delete kdmconfig;
}
*/

void KDMConfigApplication::apply()
{
  //debug("KDMConfigApplication::apply()");
  QApplication::setOverrideCursor( waitCursor );

  if (appearance)
    appearance->applySettings();
  if (font)
    font->applySettings();
  if (background)
    background->applySettings();
  if (users)
    users->applySettings();
  if (sessions)
    sessions->applySettings();

  QApplication::restoreOverrideCursor( );
}


int main(int argc, char **argv)
{
  KDMConfigApplication app(argc, argv, "kdmconfig");
  app.setTitle(klocale->translate("KDM Configuration"));
  
  if (app.runGUI())
    return app.exec();
  else
    {
//      app.init();
      return 0;
    }
}
