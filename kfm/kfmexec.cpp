#include <qlist.h>
#include <qmsgbox.h>
#include <Kconfig.h>
#include <qlabel.h>
#include <qpushbt.h>

#include "kfmexec.h"
#include "kbind.h"
#include "config-kfm.h"
#include "kfmgui.h"
#include "kfmdlg.h"

#include <klocale.h>

/***************************************************************************
 *
 * KFMExec
 *
 ***************************************************************************/

QList<KFMExec> *execList = 0L;

KFMExec::KFMExec()
{
    // Create a list of currently running KFMExecs
    if ( execList == 0L )
    {
	execList = new QList<KFMExec>;
	execList->setAutoDelete( TRUE );
    }
    
    // Delete all KFMExecs which are prepared to die
    KFMExec *e = 0L;
    for ( e = execList->first(); e != 0L; e = execList->next() )
	if ( e->isDone() )
	    execList->removeRef( e );
    execList->append( this );
    
    // We are not prepared to die yet
    bDone = FALSE;
    dlg = 0L;
    
    // We use the job to determine
    // a) the mimetype of the URL or
    // b) wether it is a directory 
    job = new KFMJob;
    connect( job, SIGNAL( error( const char* ) ), this, SLOT( slotError( const char* ) ) );
    connect( job, SIGNAL( newDirEntry( KIODirectoryEntry* ) ),
	     this, SLOT( slotNewDirEntry( KIODirectoryEntry* ) ) );
    connect( job, SIGNAL( mimeType( const char* ) ), this, SLOT( slotMimeType( const char* ) ) );
}

void KFMExec::openURL( const char *_url, bool _reload )
{
    debugT("Interested in %s\n", _url );
    
    KURL u( _url );
    if ( u.isMalformed() )
    {
	QString tmp;
	tmp.sprintf(klocale->translate("Malformed URL\n%s"), _url );
	QMessageBox::message( klocale->translate("KFM Error"), tmp );
	slotSetDone();
	return;
    }

    // A link to the web in form of a *.kdelnk file ?
    QString path = u.path();
    if ( !u.hasSubProtocol() && strcmp( u.protocol(), "file" ) == 0 && path.right(7) == ".kdelnk" )
    {
	KURL::decodeURL( path );
    
	// Try tp open the *.kdelnk file
	QFile file( path );
	if ( file.open( IO_ReadOnly ) )
	{
	    QTextStream pstream( &file );
	    KConfig config( &pstream );
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
		    openURL( u2, _reload );
		    return;
		}
		else
		{
		    // The *.kdelnk file is broken
		    QMessageBox::message( klocale->translate("KFM Error"), 
					  klocale->translate("The file does not contain a URL") );
		    slotSetDone();
		    return;
		}
	    }
	    file.close();
	}
    }
    
    // Do we know that it is !really! a file ?
    // This gives us some speedup on local hard disks.
    if ( KIOServer::isDir( _url ) == 0 )
    {    
	// A HACK
	// We must support plugin protocols here!
	// Do we try to open a tar file?
	KMimeType *typ = KMimeType::findType( _url );
	if ( strcmp( typ->getMimeType(), "application/tar" ) == 0L )
	{
	    // We change the destination on the fly
	    tryURL = _url;
	    tryURL += "#tar:/";
	}
	// HTML stuff is handled by us
	else if ( strcmp( typ->getMimeType(), "text/html" ) == 0L )
	{
	    tryURL = _url;
	}
	else
	{
	    printf("EXEC MIMETYPE\n");
	    // Can we run some default binding ?
	    if ( KMimeBind::runBinding( _url ) )
	    {
		slotSetDone();
		return;
	    }
	    else // Ok, lets find out wether it is perhaps HTML
		tryURL = _url;
	}
    }
    // Do we really know that it is a directory ?
    else if ( KIOServer::isDir( _url ) == 1 )
    {
	// Ok, lets open a new window
	KfmGui *m = new KfmGui( 0L, 0L, _url );
	m->show();
	slotSetDone();
	return;
    }
    // We are not shure about the URL
    else
	// We try to load this URL now
	tryURL = _url;
    
    job->browse( tryURL, _reload );

    // Show the user that we are doing something, since it may take
    // us some time.
    dlg = new QDialog( 0L );
    dlg->resize( 300, 120 );
    QPushButton *pb = new QPushButton( klocale->translate("Cancel"), dlg );
    pb->setGeometry( 110, 70, 80, 30 );
    connect( pb, SIGNAL( clicked() ), this, SLOT( slotSetDone() ) );
    QLabel* line1 = new QLabel( dlg );
    line1->setGeometry( 10, 10, 280, 20 );
    line1->setText( klocale->translate("Trying to open") );
    QLabel* line2 = new QLabel( dlg );
    line2->setGeometry( 10, 30, 280, 20 );
    line2->setText( tryURL );
    
    dlg->show();
}

void KFMExec::slotError( const char * )
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
	// ... and open a new window
	if ( tryURL.right(1) != "/" )
	    tryURL += "/";
	KfmGui *m = new KfmGui( 0L, 0L, tryURL );
	m->show();

	// We have done our job, so lets set the "ready to die" flag
	slotSetDone();
    }
}

void KFMExec::slotMimeType( const char *_type )
{
    // Set the "ready to die" flag
    slotSetDone();
    // Stop browsing. We need an application
    job->stop();

    if ( _type == 0L || strcmp( _type, "text/html" ) != 0L )
    {
	// Try to run the default binding on the URL since we
	// know that it is a real file ( directories have no
	// mime-type :-) 
	if ( KMimeBind::runBinding( tryURL ) )
	{
	    debugT("Our job is done\n");
	    return;
	}
	
	// Ask the user what we should do
	DlgLineEntry l( klocale->translate("Open With:"), "", 0L, true );
	debugT("OPENING DLG\n");
	if ( l.exec() )
	{
	    QString pattern = l.getText();
	    if ( pattern.isEmpty() )
		return;

	    QString decoded( tryURL );
	    KURL::decodeURL( decoded );
	    decoded = KIOServer::shellQuote( decoded ).data();
	    
	    QString cmd;
	    cmd = l.getText();
	    cmd += " ";
	    cmd += "\"";
	    cmd += decoded;
	    cmd += "\"";
	    debugT("Executing stuff '%s'\n", cmd.data());
	    
	    KMimeBind::runCmd( cmd.data() );
	}
    }
    else
    {
	// Ok, lets open a new window
	KfmGui *m = new KfmGui( 0L, 0L, tryURL );
	m->show();
    }
}

void KFMExec::slotSetDone()
{
    if ( dlg )
    {
	delete dlg;
	dlg = 0L;
    }
    if ( job )
	job->stop();
    bDone = TRUE;
}
    
KFMExec::~KFMExec()
{
    delete job;
}

#include "kfmexec.moc"
