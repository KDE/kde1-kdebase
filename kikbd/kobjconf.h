#ifndef K_OBJ_CONF
#define K_OBJ_CONF

#include <qwidget.h>
#include <ksimpleconfig.h>
#include <kconfig.h>

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
class KObjectConfig: public KConfig {
 protected:
  bool                 emptyLocal;
  QStrList             groups;
  QList<KConfigObject> entries;
 public:
  /**
     Construct new Object Configuration class.
     @param globalFile The global file usually from $KDEDIR/share
     @param localFile The local or user file usually from $HOME/kde/share
  */
  KObjectConfig(const char* globalFile=0L, const char* localFile=0L);
  virtual ~KObjectConfig(){}
 public:
  /**
     set current Group for register object
     @param pGroup The name of the group
  */
  void setGroup(const char *pGroup);
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
  virtual void emptyLocalConfig() {}
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
};

#endif
