// adopted from ksimpleconfig.h (Kalle Dahlheimer) by Matthias Hoelzer

#ifndef _KDELNK_H
#define _KDELNK_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kconfigbase.h>

class KKdelnk : public KConfigBase
{
  Q_OBJECT 

protected:

  virtual void parseConfigFiles();

public:

  KKdelnk( const char* pFile );

  virtual void sync() {};

  bool writeConfigFile( QFile& , bool = false)  { return TRUE; };

private:

  QString configFile;

};
  

 
#endif
