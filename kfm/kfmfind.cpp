#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <qpopmenu.h>
#include <qpainter.h>
#include <qapp.h>
#include <qkeycode.h>
#include <qaccel.h>
#include <qpushbt.h>
#include <qlist.h>

#include "kfmfind.h"
#include "kurl.h"

KFindWindow::KFindWindow( QWidget *parent, const char *name, const char* _url ) :
    KFileWindow( parent, name )
{
    printf("1\n");
    init( _url );
    printf("2\n");
    toolDir->setText( _url );
    printf("3\n");
}

void KFindWindow::initMenu()
{
    QPopupMenu *file = new QPopupMenu;
    CHECK_PTR( file );
    file->insertItem( "Close", this, SLOT(slotClose()) );
    file->insertItem( "Quit",  qApp, SLOT(quit()), ALT+Key_Q );

    QPopupMenu *edit = new QPopupMenu;
    CHECK_PTR( edit );
    int copyID = edit->insertItem( "Copy", this, SLOT(slotCopy()), CTRL+Key_C );
    int deleteID = edit->insertItem( "Delete", this, SLOT(slotDelete()) );

    QPopupMenu *help = new QPopupMenu;
    CHECK_PTR( help );
    help->insertItem( "About", this, SLOT(slotAbout()) );

    menu = new QMenuBar( this );
    CHECK_PTR( menu );
    menu->insertItem( "File", file );
    menu->insertItem( "Edit", edit );
    menu->insertItem( "Help", help );
}

void KFindWindow::initToolBar()
{
    toolbar = new QGroupBox( this );
    toolbar->setGeometry( 0, menu->height(), width(), 32 );
        
    topOffset = menu->height() + toolbar->height(); 

    toolLabel1 = new QLabel( "Start in", toolbar );
    toolLabel1->adjustSize();
    toolLabel1->setAutoResize( TRUE );
    toolLabel1->setGeometry( 10, 6, toolLabel1->width(), toolLabel1->height() );
    
    toolDir = new QLineEdit( toolbar );
    toolDir->setGeometry( toolLabel1->x() + toolLabel1->width() + 10, 6, 120, 20 );

    toolLabel2 = new QLabel( "searching for", toolbar );
    toolLabel2->adjustSize();
    toolLabel2->setAutoResize( TRUE );
    toolLabel2->setGeometry( toolDir->x() + toolDir->width() + 10, 6, 
			     toolLabel2->width(), toolLabel2->height() );
    
    toolName = new QLineEdit( toolbar );
    toolName->setGeometry( toolLabel2->x() + toolLabel2->width() + 10, 6, 100, 20 );

    toolStart = new QPushButton( "Do it", toolbar );
    toolStart->adjustSize();
    toolStart->setAutoResize( TRUE );
    toolStart->setGeometry( toolName->x() + toolName->width() + 10, 5, toolStart->width(), toolStart->height() );

    connect( toolStart, SIGNAL( clicked() ), this, SLOT( slotStart() ) );
}

void KFindWindow::initFileManagers()
{
    KFileWindow::initFileManagers( );
    findManager = new KFindManager( this, view );
}

void KFindWindow::slotStart()
{
    QString buffer;

    const char* dir = toolDir->text();
    const char* name = toolName->text();

    KURL u( dir );
    if ( u.isMalformed() )
    {
	printf("Error: Malformed URL\n");
	return;
    }
    
    if ( strcmp( u.protocol(), "file" ) != 0 )
    {
	printf("ERROR: Can only search in protocol file\n");
	return;
    }
    
    QString msg;
    msg.sprintf( "<h1>Find is running...</h1>Search starts at <b>%s/b><br>Searching for <b>%s</b>",dir,name );
    findManager->message( msg.data() );
    refresh( findManager );

    int t = time( 0L );
    
    tmpFile.sprintf( "/tmp/find%i", t );
    outFile.sprintf( "/tmp/out%i", t );
    buffer.sprintf( "rm %s; find %s -name \"%s\" > %s; touch %s &", tmpFile.data(), u.path(), name, outFile.data(), tmpFile.data() );
    printf("CMD %s\n",buffer.data());
    system( buffer.data() );

    timerID = startTimer( 1000 );
}

void KFindWindow::timerEvent( QTimerEvent *e )
{
    QFile f( tmpFile.data() );
    if ( f.exists() )
    {
	killTimer( timerID );
	printf("???????????? WE GOT IT ??????????????\n");

	findManager->openDir( outFile.data() );
	refresh( findManager );

	unlink( tmpFile.data() );
	unlink( outFile.data() );
    }
}

KFindWindow::~KFindWindow()
{
    if ( toolDir != 0L )
	delete toolDir;
    if ( toolName != 0L )
	delete toolName;
    if ( toolLabel1 != 0L )
	delete toolLabel1;
    if ( toolLabel2 != 0L )
	delete toolLabel2;
    if ( findManager != 0L )
	delete findManager;
}

KFindManager::KFindManager( KFileWindow * _w, KFileView * _v ) :
			   KFileManager( _w, _v )
{
    printf("A\n");
}

/*
 * Does not do any printing 
 */
void KFindManager::openDir( const char *_tmp_file )
{
    printf("B\n");
    
    if ( _tmp_file == 0L )
	return;
    if ( _tmp_file[0] == 0 )
	return;
    
    printf("Loading %s\n", _tmp_file );
    
    FILE *f = fopen( _tmp_file, "rb" );
    if ( f == 0 )
	return;
    
    char str[ 1024 ];
    QList<QString> list;
    list.setAutoDelete( TRUE );
    
    while ( !feof( f ) )
    {
	str[0] = 0;
	fgets( str, 1023, f );
	if ( str[0] != 0 )
	{
	    // Delete trailing '\n'
	    str[ strlen( str ) - 1 ] = 0;
	    QString *s = new QString( str );
	    s->detach();
	    list.append( s );
	}
    }
    
    fclose( f );
    
    view->begin();
    view->write( "<html><title>" );
    view->write( "Find Tool" );
    view->write( "</title><body>" );
    
    QString *s;
    
    for ( s = list.first(); s != 0L; s = list.next() )
    {
	strcpy( str, "file:" );
	strcat( str, s->data() );

	view->write( "<a href=" );
	view->write( str );
	view->write( "><img src=" ); 
	view->write( KFileType::findType( str )->getPixmapFile( str ) );
	view->write( "> " );
	view->write( s->data() );
	view->write( "</a><br>" );
    }

    if ( list.count() == 0 )
	view->write( "<h1>No match</h1>" );
	
    view->write( "</body></html>" );
    view->end();

    view->parse();
}

void KFindManager::message( const char *_msg )
{
    view->begin();
    view->write( "<html><title>" );
    view->write( "Find is running" );
    view->write( "</title><body>" );
    view->write( _msg );
    view->write( "</body></html>" );
    view->end();

    view->parse();
}

KFindManager::~KFindManager()
{
}

#include "kfmfind.moc"
