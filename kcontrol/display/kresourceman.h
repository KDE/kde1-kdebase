/* This file is part of the KDE libraries
    Copyright (C) 1998	Mark Donohoe <donohoe@kde.org>
						Stephan Kulow				  

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef _KRESOURCEMAN_H_
#define _KRESOURCEMAN_H_

#include <kapp.h>
#include <kprocess.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include <qobject.h>

#include <qdict.h>

/** 
 * The KResourceMan class is used for reading and writing desktop resources
 * to the _XA_RESOURCE_MANAGER property.
 *
 * KResourceMan is closely related to KConfig but differs in some respects.
 *
 * Configuration entries in the _XA_RESOURCE_MANAGER property.are of the form
 * "key: value". Each key describes a resource for an object heirarchy and uses
 * periods as object separator tokens. Wildcards are allowed. KResourceMan does
 * not support parsing of the object heirarchy. You must exactly specify the
 * object for the resource to be read or written. If you don't, KResourceMan
 * will try to read general resources using the wildcard "*".
 *
 * For exmaple, to read the resources
 *
 * *background: #d6d6d6
 * KWM.activeBackground: #000080
 *
 * use
 *
 * KResourceMan rm;
 * rm.readColorEntry( "background" );
 * rm.setKeyObject( "KWM" );
 * rm.readColorEntry( "activeBackground" );
 *
 * KResourceMan stores colors as hex. values and fonts as standard X font
 * strings.
 *
 * An important difference between KConfig and KResourceMan is that dirty
 * config. entries are not written to the _XA_RESOURCE_MANAGER property by the
 * destructor. You must expilicty call sync() to do this. A blocking xrdb
 * process is started by sync() to merge the dictionary resources in your
 * KResourceman object with the root window property..
 *
 * In future, the dependency on xrdb may be removed by assimilating its
 * functionality into KResourceMan.
 *
 * @see KConfig::KConfig
 * @author Mark Donohoe (donohe@kde.org)
 * @version 
*/
class KResourceMan : public QObject
{
	Q_OBJECT
	
private:	
  Display *kde_display;
  Window root;
  int screen;
  Atom at;
  QDict <QString> *propDict;
  QString propString;
  KProcess proc;
  QString prefix;

protected:

public:
	
	/** 
	* Create a KResourceMan object.
	*/
   KResourceMan();
   
	/** 
	* Destroys the KResourceMan.
	*/
  ~KResourceMan();

	/** 
	* Specify the group in which keys will be searched.
	*
	*/	
  void setGroup(const QString& rGroup="");

	/**
	* Read the value of an entry specified by rKey in the current property
	*
	* @param rKey	The key to search for.
	* @param pDefault A default value returned if the key was not found.
	* @return The value for this key or an empty string if no value
	*	  was found.
	*/	
  QString readEntry( const QString& rKey, 
  	  	  	  	  	  const char* pDefault = 0 ) const ;
					  
	/**
	* Read a numerical value. 
	*
	* Read the value of an entry specified by rKey in the current property 
	* and interpret it numerically.
	*
	* @param rKey The key to search for.
	* @param nDefault A default value returned if the key was not found.
	* @return The value for this key or 0 if no value was found.
	*/
  int readNumEntry( const QString& rKey, int nDefault = 0 ) const;
  
	/** 
	* Read a QFont.
	*
	* Read the value of an entry specified by rKey in the current property 
	* and interpret it as a font object.
	*
	* @param rKey		The key to search for.
	* @param pDefault	A default value returned if the key was not found.
	* @return The value for this key or a default font if no value was found.
	*/ 
  QFont readFontEntry( const QString& rKey, 
							  const QFont* pDefault = 0 ) const;

	/** 
	* Read a QColor.
	*
	* Read the value of an entry specified by rKey in the current property 
	* and interpret it as a color.
	*
	* @param rKey		The key to search for.
	* @param pDefault	A default value returned if the key was not found.
	* @return The value for this key or a default color if no value
	* was found.
	*/					  
  QColor readColorEntry( const QString& rKey,
								const QColor* pDefault = 0 ) const;
							  
	
	/** 
	* writeEntry() overridden to accept a const char * argument.
	*
	* This is stored to the current property when destroying the
	* config object or when calling Sync().
	*
	* @param rKey		The key to write.
	* @param rValue		The value to write.
	* @return The old value for this key. If this key did not exist, 
	*	  a NULL string is returned.	  
	*
	* @see #writeEntry
	*/				
  QString writeEntry( const QString& rKey, const QString& rValue );
  
	/** Write the key value pair.
	* Same as above, but write a numerical value.
	* @param rKey The key to write.
	* @param nValue The value to write.
	* @return The old value for this key. If this key did not
	* exist, a NULL string is returned.	  
	*/
  QString writeEntry( const QString& rKey, int nValue );
  
	/** Write the key value pair.
	* Same as above, but write a font
	* @param rKey The key to write.
	* @param rValue The value to write.
	* @return The old value for this key. If this key did not
	* exist, a NULL string is returned.	  
	*/
  QString writeEntry( const QString& rKey, const QFont& rFont );
  
	/** Write the key value pair.
	* Same as above, but write a color
	* @param rKey The key to write.
	* @param rValue The value to write.
	* @return The old value for this key. If this key did not
	*  exist, a NULL string is returned.	  
	*/
  QString writeEntry( const QString& rKey, const QColor& rColor );

	/** Flush the entry cache.
	* Start a blocking krdb process to merge to configuration entries
	* with the current _XA_RESOURCE_MANAGER property,
	*/	
	void sync();
};

#endif
