#include <string.h>
#include <qlist.h>
#include <qmsgbox.h>
#include <kconfig.h>
#include <qlabel.h>
#include <qpushbt.h>

#include <sys/stat.h>

#include "kfmexec.h"
#include "kbind.h"
#include "config-kfm.h"
#include "kfmgui.h"
#include "kfmdlg.h"
#include "utils.h"

#include <klocale.h>
#include <kstring.h>

/***************************************************************************
 *
 * KFMExec
 *
 ***************************************************************************/

QList<KFMExec>* KFMExec::lstZombies = 0L;

KFMExec::KFMExec()
{
    // Create a list of currently running KFMExecs
    if ( lstZombies == 0L )
    {
	lstZombies = new QList<KFMExec>;
	lstZombies->setAutoDelete( TRUE );
    }

    lstZombies->clear();

    // We are not prepared to die yet
    bDone = FALSE;
    dlg = 0L;
    
    // We use the job to determine
    // a) the mimetype of the URL or
    // b) wether it is a directory 
    job = new KFMJob;
    connect( job, SIGNAL( error( int, const char* ) ), this, SLOT( slotError( int, const char* ) ) );
    connect( job, SIGNAL( newDirEntry( KIODirectoryEntry* ) ),
	     this, SLOT( slotNewDirEntry( KIODirectoryEntry* ) ) );
    connect( job, SIGNAL( mimeType( const char* ) ), this, SLOT( slotMimeType( const char* ) ) );
}

void KFMExec::openURL( const char *_url  )
{
    KURL u( _url );
    if ( u.isMalformed() )
    {
	QString tmp;
	tmp << klocale->translate("Malformed URL\n") << _url;
	QMessageBox::warning( 0, klocale->translate("KFM Error"), tmp );
	// We are a zombie now
	prepareToDie();
	return;
    }

    // A link to the web in form of a *.kdelnk file ?
    QString path = u.path();
    if ( !u.hasSubProtocol() && strcmp( u.protocol(), "file" ) == 0 && path.right(7) == ".kdelnk" )
    {
	// Try tp open the *.kdelnk file
	QFile file( path );
	if ( file.open( IO_ReadOnly ) )
	{
	    file.close();
	    KConfig config( path );
	    config.setGroup( "KDE Desktop Entry" );
	    QString typ = config.readEntry( "Type" );
	    // Is it a link ?
	    if ( typ == "Link" )
	    {
		// Is there a URL ?
		QString u2 = config.readEntry( "URL" );
		if ( !u2.isEmpty() )
		{
		    // It is a link and we have new URL => Recursion with the new URL
		    openURL( u2 );
		    return;
		}
		else
		{
		    // The *.kdelnk file is broken
		    QMessageBox::warning( 0, klocale->translate("KFM Error"), 
					  klocale->translate("The file does not contain a URL") );
		    // We are a zombie now
		    prepareToDie();
		    return;
		}
	    }
	    file.close();
	}
    }
    
    // Do we know that it is !really! a file ?
    // This gives us some speedup on local hard disks.
    if ( KIOServer::isDir( _url ) == 0 && !u.hasSubProtocol() && strcmp( u.protocol(), "file" ) == 0 )
    {    
	tryURL = openLocalURL( _url );
	if ( tryURL.isEmpty() )
	    return;
    }
    // Do we really know that it is a directory ?
    else if ( KIOServer::isDir( _url ) == 1 )
    {
	// Ok, lets open a new window
	KfmGui *m = new KfmGui( 0L, 0L, _url );
	m->show();
	return;
    }
    // We are not shure about the URL
    else
	// We try to load this URL now
	tryURL = _url;
    
    // Find out what to do with this URL.
    job->browse( tryURL, false );

    // Show the user that we are doing something, since it may take
    // us some time.
    dlg = new QDialog( 0L );
    dlg->resize( 300, 120 );
    QPushButton *pb = new QPushButton( klocale->translate("Cancel"), dlg );
    pb->setGeometry( 110, 70, 80, 30 );
    connect( pb, SIGNAL( clicked() ), this, SLOT( slotCancel() ) );
    QLabel* line1 = new QLabel( dlg );
    line1->setGeometry( 10, 10, 280, 20 );
    line1->setText( klocale->translate("Trying to open") );
    QLabel* line2 = new QLabel( dlg );
    line2->setGeometry( 10, 30, 280, 20 );
    line2->setText( tryURL );
    
    dlg->show();
}

void KFMExec::slotCancel()
{
    // We are a zombie now
    prepareToDie();
    return;
}

void KFMExec::slotError( int, const char * )
{
}

void KFMExec::slotNewDirEntry( KIODirectoryEntry * _entry )
{
    // Hack, this is no ideal way to find out wether the 'list' command succeded.
    // Did we receive some directory entry ?
    if ( strcmp( _entry->getName(), "." ) != 0 && strcmp( _entry->getName(), ".." ) != 0 &&
	 strcmp( _entry->getName(), "./" ) != 0 && strcmp( _entry->getName(), "../" ) != 0 )
    {
	// Ok, we are done, so lets stop the job ...
	job->stop();
	
	KURL u( tryURL );
	
	// ... and open a new window
	if ( tryURL.right(1) != "/" && u.hasPath() )
	    tryURL += "/";
	KfmGui *m = new KfmGui( 0L, 0L, tryURL );
	m->show();

	// We are a zombie now
	prepareToDie();
	return;
    }
}

void KFMExec::slotMimeType( const char *_type )
{
    char *typestr=0;
    const char *aType=0;
    const char *aCharset=0;
    if (_type)
    {
        printf("MimeType: %s\n",_type);
        typestr=new char[strlen(_type)+1];
        strcpy(typestr,_type);
	aType=strtok(typestr," ;\t\n");
	char *tmp;
	while((tmp=strtok(0," ;\t\n"))){
	    printf("token: %s\n",tmp);
            if ( strncmp(tmp,"charset=",8)==0 ) aCharset=tmp+8;
	}    
	if ( aCharset != 0 )
	{
	    printf("charset: %s\n",aCharset);
	    tmp=strpbrk(aCharset," ;\t\n");
	    if ( tmp != 0 ) *tmp=0;
	    printf("charset: %s\n",aCharset);
	}    
    }  
    
    // Stop browsing. We need an application
    job->stop();
    // delete job;
    // job = 0L;
    
    // GZIP
    if ( aType && strcmp( aType, "application/x-gzip" ) == 0L )
    {
	job->stop();
	tryURL += "#gzip:/";
	openURL( tryURL );
    }
    // TAR
    else if ( aType && strcmp( aType, "application/x-tar" ) == 0L )
    {
	// Is this tar file perhaps hosted in a gzipped file ?
	KURL u( tryURL );
	// ... then we already have a 'gzip' subprotocol
	if ( u.hasSubProtocol() )
	{
	    KURL u2( u.nestedURL() );
	    if ( strcmp( u2.protocol(), "gzip" ) == 0 )
	    {
		// Remove the 'gzip' protocol. It will only slow down the process,
		// since two subprotocols '#gzip:/#tar:/' are not very fast
		// right now.
		tryURL = u.parentURL();
	    }
	}
	
	job->stop();
	tryURL += "#tar:/";
	openURL( tryURL );
    }
    // No HTML ?
    else if ( aType == 0L || strcmp( aType, "text/html" ) != 0L )
    {
	// Do we know the mime type ?
	if ( aType != 0L )
	{
	    KMimeType *mime = KMimeType::findByName( aType );
	    // Try to run the URL if we know the mime type
	    if ( mime && mime->run( tryURL ) )
	    {
		// We are a zombie now
		prepareToDie();
	    }
	}
		
	// Ask the user what we should do
	DlgLineEntry l( klocale->translate("Open With:"), "", 0L, true );
	if ( l.exec() )
	{
	    QString pattern = l.getText();
	    // The user did not enter anything ?
	    if ( pattern.isEmpty() )
	    {
		// We are a zombie now
		prepareToDie();
	    }
	    else 
	    {
	        QStrList list;
	        list.append( tryURL );
	        openWithOldApplication( pattern, list );
	    
	        prepareToDie();
	    }	
	}
    }
    // It is HTML
    else
    {
	// Ok, lets open a new window
	KfmGui *m = new KfmGui( 0L, 0L, tryURL );
	if ( aCharset != 0 ) m->setCharset(aCharset);
	m->show();
	
	// We are a zombie now
	prepareToDie();
    }
    delete typestr;
}

QString KFMExec::openLocalURL( const char *_url )
{
    KURL u( _url );

    // Is it a local directory ?
    struct stat buff;
    if ( stat( u.path(), &buff ) == 0 && S_ISDIR( buff.st_mode ) )
    {
	KfmGui *w = KfmGui::findWindow( _url );
	if ( w != 0L )
	{
	    w->show();
	    return QString();
	}
	w = new KfmGui( 0L, 0L, _url );
	w->show();
	return QString();
    }
    
    QString tryURL;
    
    KMimeType *typ = KMimeType::getMagicMimeType( _url );

    // A HACK
    // We must support plugin protocols here!
    // Do we try to open a tar file?
    // tar files ( zipped ones ) can be recognized by extension very fast
    QString tmp = u.path();
    if ( tmp.right(4) == ".tgz" || tmp.right(7) == ".tar.gz" )
    {
	printf("!!!!!!!!!!!!!! TGZ !!!!!!!!!!!!!!!!\n");
	// We change the destination on the fly
	tryURL = "file:";
	tryURL += u.path();
	tryURL += "#tar:/";
    }	
    // Zipped file
    else if ( strcmp( typ->getMimeType(), "application/x-gzip" ) == 0L )
    {
	// We change the destination on the fly
	tryURL = "file:";
	tryURL += u.path();
	tryURL += "#gzip:/";
	// Dont forget the reference if there was one.
	if ( u.reference() != 0L && u.reference()[0] != 0 )
	{
	    tryURL += "#";
	    tryURL += u.reference();
	}
    }
    // Uncompressed tar file
    else if ( typ && strcmp( typ->getMimeType(), "application/x-tar" ) == 0L )
    {
	// We change the destination on the fly
	tryURL = "file:";
	tryURL += u.path();
	tryURL += "#tar:/";
    }	
    // HTML stuff is handled by us
    else if ( strcmp( typ->getMimeType(), "text/html" ) == 0L )
    {
	tryURL = _url;
    }
    // Executables
    /* else if ( strcmp( typ->getMimeType(), "application/x-executable" ) == 0L ||
	      strcmp( typ->getMimeType(), "application/x-shellscript" ) == 0L )
    {
	KMimeBind::runCmd( _url );
	// No URL left since we have done the job
	// => return an empty string
	return QString();
    } */
    else
    {
	// Execute the best matching binding for this URL.
	printf("IS A '%s'\n",typ->getMimeType());
	if ( typ->run( _url ) )
	    // No URL left since we have done the job
	    // => return an empty string
	    return QString();

	// We could not execute the mimetype
	tryURL = _url;
    }
    
    return tryURL;
}

void KFMExec::prepareToDie()
{
    if ( dlg )
    {
	delete dlg;
	dlg = 0L;
    }

    if ( job )
	disconnect( job, 0, this, 0 );

    bDone = true;
    lstZombies->append( this );
}

KFMExec::~KFMExec()
{
    if ( dlg )
    {
	delete dlg;
	dlg = 0L;
    }

    if ( job )
	delete job;
}

#include "kfmexec.moc"
