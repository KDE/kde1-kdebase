// (c) Torben Weis 1998

#include <kwm.h>
#include <kcontrol.h>
#include <ksimpleconfig.h>

#include <unistd.h>

#include "htmlopts.h"
#include "miscopts.h"

KConfigBase *g_pConfig = 0L;

class KfmApplication : public KControlApplication
{
public:
  KfmApplication(int &argc, char **arg, const char *name);
    
  virtual void init();
  virtual void apply();
  virtual void defaultValues();
  
private:
  KFontOptions *m_pFontOptions;
  KColorOptions *m_pColorOptions;
  KMiscOptions *m_pMiscOptions;
};

KfmApplication::KfmApplication(int &argc, char **argv, const char *name)
  : KControlApplication(argc, argv, name)
{
  m_pFontOptions = 0L;
  m_pColorOptions = 0L;
  m_pMiscOptions = 0L;

  if ( runGUI() )
  {
    if (!pages || pages->contains("font"))
      addPage( m_pFontOptions = new KFontOptions( dialog, "font" ), i18n("&Font"), "" );
    if (!pages || pages->contains("color"))
      addPage( m_pColorOptions = new KColorOptions( dialog, "color"), i18n("&Color"), "" );
    if (!pages || pages->contains("misc"))
      addPage( m_pMiscOptions = new KMiscOptions( dialog, "misc"), i18n("&Other"), "" );
	    
    if ( m_pFontOptions || m_pColorOptions )
    {
      QSize t;
      int w = 0, h = 0;
		
      if ( m_pFontOptions )
      {
	t = m_pFontOptions->minimumSize();
	w = t.width();
	h = t.height();
      }
      if ( m_pColorOptions )
      {
	t = m_pColorOptions->minimumSize();
	if ( w < t.width() )
	  w = t.width();
	if ( h < t.height() )
	  h = t.height();
      }
      if ( m_pMiscOptions )
      {
	t = m_pMiscOptions->minimumSize();
	if ( w < t.width() )
	  w = t.width();
	if ( h < t.height() )
	  h = t.height();
      }
      
      dialog->resize(w,h);
      dialog->show();
    }
    else
    {
      fprintf(stderr, i18n("usage: kcmkfm [-init | {font,color}]\n"));
      justInit = true;
    }  
  }
}


void KfmApplication::init()
{
  if ( m_pFontOptions )
    m_pFontOptions->loadSettings();
  if ( m_pColorOptions )
    m_pColorOptions->loadSettings();
  if ( m_pMiscOptions )
    m_pMiscOptions->loadSettings();
}

void KfmApplication::defaultValues()
{
  if ( m_pFontOptions )
    m_pFontOptions->defaultSettings();
  if ( m_pColorOptions )
    m_pColorOptions->defaultSettings();
  if ( m_pMiscOptions )
    m_pMiscOptions->defaultSettings();
}

void KfmApplication::apply()
{
  if ( m_pFontOptions )
    m_pFontOptions->applySettings();
  if ( m_pColorOptions )
    m_pColorOptions->applySettings();
  if ( m_pMiscOptions )
    m_pMiscOptions->applySettings();
  if ( fork() == 0 )
  { 
      // execute 'kfmclient configure'
      execl(kapp->kde_bindir()+"/kfmclient","kfmclient","configure",0);
      warning("Error launching 'kfmclient configure' !");
      exit(1);
  }
}


int main(int argc, char **argv)
{
  g_pConfig = new KConfig( KApplication::kde_configdir() + "/kfmrc", 
			   KApplication::localconfigdir() + "/kfmrc" );
  KfmApplication app( argc, argv, "kcmkfm" );
    
  app.setTitle(i18n( "KFM Configuration"));

  int ret;
  if (app.runGUI())
    ret = app.exec();
  else
    ret = 0;

  delete g_pConfig;
  return ret;
}



