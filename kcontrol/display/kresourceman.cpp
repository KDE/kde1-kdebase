#include <qdir.h>
#include <time.h>

#include "kresourceman.h"
#include "kresourceman.moc"

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp/"
#endif

KResourceMan::KResourceMan()
{
	Atom type;
	int format;
	unsigned long nitems;
	unsigned long offset = 0;
	unsigned long bytes_after = 1;
	char *buf;
	
	prefix.sprintf("*");
	
	QString rProp( "RESOURCE_MANAGER" );
	
	propDict = new QDict <QString> ( 199 );
	
	kde_display = KApplication::desktop()->x11Display();
	screen = DefaultScreen(kde_display);
    root = RootWindow(kde_display, screen);
	at = 0;
	
	if( !rProp.isEmpty() ) {
  		at = XInternAtom( kde_display, rProp.data(), False);
		
		QString s = "";
		
		while( bytes_after > 0 ) {
			XGetWindowProperty( kde_display, root, at, offset, 256,
				False, XA_STRING, &type, &format, &nitems, &bytes_after,
				(unsigned char **)&buf);

			s.append( buf );
			offset += 256;
		}
			
		// Parse through the property string stripping out key value pairs
		// and putting them in the dictionary
		
		//debug(s);
		QString keypair;
		int i=0;
		QString key;
		QString value;
		
		while(s.length() >0 ) {
			
			// parse the string for first key-value pair separator '\n'
			
			i = s.find("\n");
			if(i == -1)
				i = s.length();
		
			// extract the key-values pair and remove from string
			
			keypair = s.left(i).simplifyWhiteSpace();
			s.remove(0,i+1);
			
			// split key and value and add to dictionary
			
			i = keypair.find( ":" );
			if( i != -1 ) {
				key = keypair.left( i ).simplifyWhiteSpace();
				value = keypair.right( keypair.length() - i - 1 ).simplifyWhiteSpace();
				//debug("%s -> %s", key.data(), value.data() );
				propDict->insert( key.data(), new QString( value.data() ) );
			}
		}
	}
}

KResourceMan::~KResourceMan()
{
	//sync();
	delete propDict;
}

void KResourceMan::sync()
{

	if ( !propDict->isEmpty() ) {
		
		time_t timestamp;
		::time( &timestamp );
		
		QDictIterator <QString> it( *propDict );
		QString keyvalue;

    	while ( it.current() ) {

	    QString *value = propDict->find( it.currentKey() );
			
	    keyvalue.sprintf( "%s: %s\n", it.currentKey(), value->data() );
	    propString += keyvalue;
// 	    if (it.currentKey() == "font"){
// 		// dirty hack, makes font to fontList for x-resources
// 		keyvalue.sprintf( "%s: %s\n", it.currentKey(), value->data() );
// 		propString += keyvalue;
		
// 	    }
	    ++it;
	}
		
		QString fileName;
		fileName.sprintf(_PATH_TMP"/krdb.%ld", timestamp);
		
		QFile f( fileName );
		if ( f.open( IO_WriteOnly ) ) {
			f.writeBlock( propString.data(), propString.length() );
			f.close();
		}
		
		proc.setExecutable("xrdb");
		proc << "-merge" << fileName.data();
		
		proc.start( KProcess::Block );
		
		QDir d( _PATH_TMP );
 		if ( d.exists() )
 			d.remove( fileName );
	
		propDict->clear();
	}
}

void KResourceMan::setGroup( const QString& rGroup )
{
	QString s("General");
	if ( rGroup == s )
		prefix.sprintf( "*" );
	else
		prefix.sprintf( "%s.", rGroup.data() );
}

QString KResourceMan::readEntry( const QString& rKey,
			    const char* pDefault ) const
{
	if( !propDict->isEmpty() ) {
		
		QString *fullKey = new QString(rKey.data());
		fullKey->prepend( prefix );
		
		QString *aValue = 0;
		aValue = propDict->find( fullKey->data() );
		
		if (!aValue && pDefault ) {
			aValue = new QString;
			aValue->sprintf( pDefault );
		}
		return *aValue;
	} else {
	
		QString aValue;
		
		if ( pDefault )
			aValue.sprintf( pDefault );
			
		return aValue;
	}
}

int KResourceMan::readNumEntry( const QString& rKey, int nDefault ) const
{
  bool ok;
  int rc;

  QString aValue = readEntry( rKey );
  if( aValue.isNull() )
	return nDefault;
  else
	{
	  rc = aValue.toInt( &ok );
	  return( ok ? rc : 0 );
	}
}


QFont KResourceMan::readFontEntry( const QString& rKey,
							  const QFont* pDefault ) const
{
  QFont aRetFont;

  QString aValue = readEntry( rKey );
  if( !aValue.isNull() )
	{
	  // find first part (font family)
	  int nIndex = aValue.find( ',' );
	  if( nIndex == -1 )
		return aRetFont;
	  aRetFont.setFamily( aValue.left( nIndex ) );
	
	  // find second part (point size)
	  int nOldIndex = nIndex;
	  nIndex = aValue.find( ',', nOldIndex+1 );
	  if( nIndex == -1 )
		return aRetFont;
	  aRetFont.setPointSize( aValue.mid( nOldIndex+1,
										 nIndex-nOldIndex-1 ).toInt() );

	  // find third part (style hint)
	  nOldIndex = nIndex;
	  nIndex = aValue.find( ',', nOldIndex+1 );
	  if( nIndex == -1 )
		return aRetFont;
	  aRetFont.setStyleHint( (QFont::StyleHint)aValue.mid( nOldIndex+1,
													nIndex-nOldIndex-1 ).toUInt() );

	  // find fourth part (char set)
	  nOldIndex = nIndex;
	  nIndex = aValue.find( ',', nOldIndex+1 );
	  if( nIndex == -1 )
		return aRetFont;
	  aRetFont.setCharSet( (QFont::CharSet)aValue.mid( nOldIndex+1,
									   nIndex-nOldIndex-1 ).toUInt() );

	  // find fifth part (weight)
	  nOldIndex = nIndex;
	  nIndex = aValue.find( ',', nOldIndex+1 );
	  if( nIndex == -1 )
		return aRetFont;
	  aRetFont.setWeight( aValue.mid( nOldIndex+1,
									  nIndex-nOldIndex-1 ).toUInt() );

	  // find sixth part (font bits)
	  uint nFontBits = aValue.right( aValue.length()-nIndex-1 ).toUInt();
	  if( nFontBits & 0x01 )
		aRetFont.setItalic( true );
	  if( nFontBits & 0x02 )
		aRetFont.setUnderline( true );
	  if( nFontBits & 0x04 )
		aRetFont.setStrikeOut( true );
	  if( nFontBits & 0x08 )
		aRetFont.setFixedPitch( true );
	  if( nFontBits & 0x20 )
		aRetFont.setRawMode( true );
	}
  else if( pDefault )
	aRetFont = *pDefault;

  return aRetFont;
}


QColor KResourceMan::readColorEntry( const QString& rKey,
								const QColor* pDefault ) const
{
  QColor aRetColor;
  int nRed = 0, nGreen = 0, nBlue = 0;

  QString aValue = readEntry( rKey );
  if( !aValue.isNull() )
	{
  	  bool bOK;
	
	  // Support #ffffff style colour naming.
	  if( aValue.find("#") == 0 ) {
	  	aRetColor.setNamedColor( aValue );
		return aRetColor;
	  }
		
	  // find first part (red)
	  int nIndex = aValue.find( ',' );
	  if( nIndex == -1 )
		return aRetColor;
	  nRed = aValue.left( nIndex ).toInt( &bOK );
	
	  // find second part (green)
	  int nOldIndex = nIndex;
	  nIndex = aValue.find( ',', nOldIndex+1 );
	  if( nIndex == -1 )
		return aRetColor;
	  nGreen = aValue.mid( nOldIndex+1,
						   nIndex-nOldIndex-1 ).toInt( &bOK );

	  // find third part (blue)
	  nBlue = aValue.right( aValue.length()-nIndex-1 ).toInt( &bOK );

	  aRetColor.setRgb( nRed, nGreen, nBlue );
	}
  else if( pDefault )
	aRetColor = *pDefault;

  return aRetColor;
}

QString KResourceMan::writeEntry( const QString& rKey, const QString& rValue )
{
	QString *aValue = new QString();
	
	QString *fullKey = new QString(rKey.data());
	fullKey->prepend( prefix );
	
	if( propDict->find( fullKey->data() ) ) {
		aValue = propDict->find( fullKey->data() );
		propDict->replace( fullKey->data(), new QString( rValue.data() ) );
	} else {
		propDict->insert( fullKey->data(), new QString( rValue.data() ) );
	}
	
	if ( !aValue )
		aValue->sprintf(rValue);
	
	return *aValue;
}

QString KResourceMan::writeEntry( const QString& rKey, int nValue )
{
  QString aValue;

  aValue.setNum( nValue );

  return writeEntry( rKey, aValue );
}

QString KResourceMan::writeEntry( const QString& rKey, const QFont& rFont )
{
	QString aValue;
#if QT_VERSION > 140
	aValue = rFont.rawName();
	return writeEntry( rKey, aValue );
#endif
	QFontInfo fi( rFont );

	aValue.sprintf( "-*-" );
	aValue += fi.family();
	
	if ( fi.bold() )
		aValue += "-bold";
	else
		aValue += "-medium";
	
	if ( fi.italic() )
		aValue += "-i";
	else
		aValue += "-r";
	
	//
	// For setting font size the code I have commented out here should be
	// correct according to the technical documentation of Qt and X. However, the
	// actual code used below does a better job of matching Qt fonts with X
	// server fonts. MD 8 Apr 98
	//	
	//aValue += "-normal--*-";
	//	
	//QString s;
	//s.sprintf( "%d-*-*-*-*-", 10*fi.pointSize() );
	//aValue += s;
	
	aValue += "-normal-*-";
		
	QString s;
	s.sprintf( "%d-*-*-*-*-*-", fi.pointSize() );
	aValue += s;
	
	switch ( fi.charSet() ) {
		case QFont::Latin1:
			aValue += "iso8859-1";
			break;
		case QFont::AnyCharSet:
		default:
			aValue += "*-*";
			break;
		case QFont::Latin2:
			aValue += "iso8859-2";
			break;
		case QFont::Latin3:
			aValue += "iso8859-3";
			break;
		case QFont::Latin4:
			aValue += "iso8859-4";
			break;
		case QFont::Latin5:
			aValue += "iso8859-5";
			break;
		case QFont::Latin6:
			aValue += "iso8859-6";
			break;
		case QFont::Latin7:
			aValue += "iso8859-7";
			break;
		case QFont::Latin8:
			aValue += "iso8859-8";
			break;
		case QFont::Latin9:
			aValue += "iso8859-9";
			break;
	}
  return writeEntry( rKey, aValue );
}

QString KResourceMan::writeEntry( const QString& rKey, const QColor& rColor )
{
  QString aValue;
  aValue.sprintf( "#%02x%02x%02x", rColor.red(), rColor.green(), rColor.blue() );

  return writeEntry( rKey, aValue );
}
