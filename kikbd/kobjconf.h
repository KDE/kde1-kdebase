/*
   - 

  written 1998 by Alexander Budnik <budnik@linserv.jinr.ru>
  
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
#ifndef K_OBJ_CONF
#define K_OBJ_CONF

#include <qwidget.h>
#include "ksimpleconfig.h"

//@Include: kconfobjs.h
class KObjectConfig;
/**
   This is abstract base class for using with KObjectConfig.
*/
class KConfigObject: public QObject {
  Q_OBJECT
 protected:
  bool         deleteData, persistent, global, NLS;
  QStrList     keys;
  const char*  group;
  void*        data;
 protected:
  /**
     Reading object from configuration file. Must be reimplemented
     in child to read specific data.
     @param config The pointer to parent KObjectConfig class
  */
  virtual void readObject(KObjectConfig*) {}
  /**
     Writing object back to configuration file. Must be reimplemented
     in child to read specific data.
     @param config The pointer to parent KObjectConfig class
  */
  virtual void writeObject(KObjectConfig*){}
  friend class KObjectConfig;
 public:
  /**
     Constructor of abstract object
     @param pData data reference
     @param pDeleteData If is TRUE the data referenced by pData will be
     deleted by delete at destruction time.
     @param key if not NULL used as key for this object
  */
  KConfigObject(void* pData=0L, bool pDeleteData=FALSE, const char* key=0L);
  virtual ~KConfigObject();
  /**
     Return active key for this object.
  */
  const QString   getKey () const {return keys.current();}
  /**
     Return all keys for this object. Usually object contains only one key.
  */
  const QStrList& getKeys() const {return keys;}
  /**
     Set DeleteData parameter.
  */
  void  setDeleteData(bool f){deleteData = f;}
  /**
     Configure writing parameters.
     @see KConfigBase::writeEntry KConfigBase class
  */
  void  configure(bool bPersistent = TRUE, bool bGlobal = FALSE,
		  bool bNLS = FALSE) {
    persistent = bPersistent;
    global = bGlobal;
    NLS = bNLS;
  }
};



/**
   This is hi level user interface to KConfig class.
   It can be used in KApplication to read configuration from single or
   multiple files and also it can be used in KControlApplication to
   help create widgets and load/save data.

   It should be very easy to load/save data with KObjectConfig. This is an
   example:

       main() {
         KObjectConfig config("test.config");
	 bool myBool;
	 QColor myColor;

	 config.registerBool("MyBool", myBool);
	 config.registerColor("MyColor", myColor);
	 config.loadConfig();

	 ... do some job
	 ... may be change values

	 config.saveConfig();
       }

   If you need to create KControlApplication or in other reason you
   need to change value of one of registered objects in GUI:

          ...
	  QWidget *w = config.createBoolWidget(myBool, "Set My Bool", parent);
	  QWidget *w = config.createColorWidget(myColor, parent);
	  ...

   @author Alexander Budnik (budnik@linserv.jinr.ru)
   @short KDE Object Configuration base class
*/
class KObjectConfig: public QObject {
  Q_OBJECT
 protected:
  KConfigBase         *config;
  QString              configFile;
  QStrList             groups;
  QList<KConfigObject> entries;
  bool                 deleteConfig;
  bool                 readOnly;
  short                configType;
  float                version;
 protected:
   void                init();
   void                initConfig();
 public:
   enum {External, AppRc, UserFromSystemRc, AppData, File};
  /** Construct with existing KConfigBase object.
      @param config     KConfigBase class, for example returned by kapp->getConfig()
      @param autoDelete if TRUE class pointed by config will be deleted at destruction time
  */
  KObjectConfig(KConfigBase* config, bool autoDelete = FALSE);
  /** Constructor
      @param type   if AppRc, name is ignored and Application rc file is used
                       (user or if does not exists, system).
                    if File,  name is absolute name of opened file.
		    if UserFromSystemRc, like AppRc, but if User rc file does not exists,
		       it will be created with as copy of system rc file or from compiled
		       in defaults and signal newUserRc will be emited.
		    if AppData, name is the file name relative to the Application share directory
		       (~/.kde/share/apps/<app name> or $KDEDIR/.share/apps/<app name>).
      @param name   this interpreted acording to the value of type
  */
  KObjectConfig(int type = AppRc, const char *name = 0L);
  ~KObjectConfig();
 public:
  KConfigBase* getConfig() {return config;}
  /**
     set current Group for register object
     @param pGroup The name of the group
  */
  void setGroup(const char *pGroup);
  /** Return current group
  */
  const char* group();
  /**
     Load all registered objects from configuration file.
     Usualy you do not need to reimplement this in the child class.
  */
  virtual void loadConfig();
  /**
     Write back all dirty objects to configuration file.
     Usualy you do not need to reimplement this in the child class.
  */
  virtual void saveConfig();
  /**
     Register object in current group. This object will be deleted
     automatically at destruction time.
     @param obj object created by new
  */
  void registerObject(KConfigObject* obj);
  /**
     Find previously registered object by looking for a data reference.
     @param data pointer to data registered with one of object
  */
  KConfigObject* find(void* data);
  /**
     Same as above but especially for bool variable.
     Automatically create new object. Same as :
     registerObject(new KConfigBoolObject(key, val));
     @param key The key to look in the registered group
     @param existent value to load data in and to write from
  */
  // register(char*, bool);
  void registerBool(const char* key, bool& val);
  /**
     Same as above but for int variable.
  */
  void registerInt(const char* key, int& val);
  /**
     Same as above but for String variable.
  */
  void registerString(const char* key, QString& val);
  /**
     Same as above but for String List variable.
  */
  void registerStrList(const char* key, QStrList& val, char pSep=',');
  /**
     Same as above but for Color variable.
  */
  void registerColor(const char* key, QColor&val);
  /**
     Same as above but for Font variable.
  */
  void registerFont(const char* key, QFont&val);
  /**
     Create widget to interactively change object value.
     Use QCheckBox for changing bool variable.
     @param val The variable registered with bool object
     @param label Text label for QCheckBox
     @param parent parent widget
  */
  QWidget* createBoolWidget(bool& val, const char* label, QWidget* parent=0L);
  /**
     Same as above but for Color Widget. Create colored QPushButton connected
     to KColorDialog.
  */
  QWidget* createColorWidget(QColor& val, QWidget* parent=0L);
  /**
     Same as above but for Font Widget. Create labeled button connected
     to KFontDialog.
  */
  QWidget* createFontWidget(QFont& val, QWidget* parent=0L);
  QWidget* createComboWidget(QString& val, const char** list=0L,
			     QWidget* parent=0L);
  QWidget* createComboWidget2(QString& val, const char** list=0L,
			      const char* name=0L, QWidget* parent=0L);
  /**
     This set config file version and enables version control 
     (disabled by default). Version number follow the standart convension:
     major number before dot, minor number after dot.
  */
  void  setVersion(float v) {version = v;}
  float getVersion() const {return version;}

 signals:
  /** This signal emited when new user rc file created by using UserFromSystemRc
  */
  void newUserRcFile();
  void noUserRcFile(const char*);
  /** This signal emited when where is no user Application rc file
  */
  /** This signal emited when where is no system Application rc file
  */
  void noSystemRcFile(const char*);
  /** This signal emited when where is no user Application data file
  */
  void noUserDataFile(const char*);
  /** This signal emited when where is no system Application data file
  */
  void noSystemDataFile(const char*);
  /** This signal is emited when version control enabled by setVersion and
      readed file major version older then expected
  */
  void olderVersion();
  /** This signal is emited when version control enabled by setVersion and
      readed file major version newer then expected
  */
  void newerVersion();
};

#endif
