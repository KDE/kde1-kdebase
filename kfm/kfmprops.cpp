#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <Kconfig.h>

#include <qfile.h>
#include <qdir.h>
#include <qfiledlg.h>
#include <qmsgbox.h>
#include <qlist.h>
#include <qstrlist.h>

#include <kurl.h>

#include "kfmprops.h"
#include "kbind.h"
#include "kioserver.h"
#include "kfmgui.h"
#include "kfmpaths.h"
#include "root.h"
#include <config-kfm.h>

#include <klocale.h>

Properties::Properties( const char *_url ) : QObject()
{
    pageList.setAutoDelete( true );
    
    url = _url;
    url.detach();
    
    kurl = new KURL( url );
    if ( kurl->isMalformed() )
	delete this;
    
    tab = new QTabDialog( 0L, 0L );
    tab->setGeometry( tab->x(), tab->y(), 400, 400 );

    insertPages();

    tab->setOKButton(klocale->translate("OK")); 
    tab->setCancelButton(klocale->translate("Cancel"));

    connect( tab, SIGNAL( applyButtonPressed() ), this, SLOT( slotApply() ) );
    
    tab->show();
}

Properties::~Properties()
{
    pageList.clear();
    
    if ( kurl != 0L )
	delete kurl;
}

void Properties::emitPropertiesChanged( const char *_new_name )
{
    emit propertiesChanged( url.data(), _new_name );
}

void Properties::slotApply()
{
    PropsPage *page;
    for ( page = pageList.last(); page != 0L; page = pageList.prev() )
	page->applyChanges();

    QString s = getURL();
    s.detach();

    // Strip the filename
    if ( !KIOServer::isDir( s.data() ) )
    {
	int i = s.findRev( "/" );
	// Should never happen
	if ( i == -1 )
	return;
	s = s.left( i + 1 );
    }
    
    KIOServer::sendNotify( s.data() );
}

void Properties::insertPages()
{
    if ( FilePropsPage::supports( kurl ) )
    {
	PropsPage *p = new FilePropsPage( this );
	tab->addTab( p, p->getTabName() );
	pageList.append( p );
    }

    if ( FilePermissionsPropsPage::supports( kurl ) )
    {
	PropsPage *p = new FilePermissionsPropsPage( this );
	tab->addTab( p, p->getTabName() );
	pageList.append( p );
    }

    if ( ExecPropsPage::supports( kurl ) )
    {
	PropsPage *p = new ExecPropsPage( this );
	tab->addTab( p, p->getTabName() );
	pageList.append( p );
    }

    if ( ApplicationPropsPage::supports( kurl ) )
    {
	PropsPage *p = new ApplicationPropsPage( this );
	tab->addTab( p, p->getTabName() );
	pageList.append( p );
    }

    if ( BindingPropsPage::supports( kurl ) )
    {
	PropsPage *p = new BindingPropsPage( this );
	tab->addTab( p, p->getTabName() );
	pageList.append( p );
    }

    if ( URLPropsPage::supports( kurl ) )
    {
	PropsPage *p = new URLPropsPage( this );
	tab->addTab( p, p->getTabName() );
	pageList.append( p );
    }

    if ( DirPropsPage::supports( kurl ) )
    {
	PropsPage *p = new DirPropsPage( this );
	tab->addTab( p, p->getTabName() );
	pageList.append( p );
    }

    if ( DevicePropsPage::supports( kurl ) )
    {
	PropsPage *p = new DevicePropsPage( this );
	tab->addTab( p, p->getTabName() );
	pageList.append( p );
    }
}


PropsPage::PropsPage( Properties *_props ) : QWidget( _props->getTab(), 0L )
{
    properties = _props;
}

FilePropsPage::FilePropsPage( Properties *_props ) : PropsPage( _props )
{
    QString path = properties->getKURL()->path();
    KURL::decodeURL( path );
    QString filename = properties->getKURL()->filename();
    KURL::decodeURL( filename );
    
    QString tmp = path.data();
    if ( tmp.right(1) != "/" )
      tmp += "/";
    bool isTrash = false;
    // is it the trash bin ?
    if ( strcmp( properties->getKURL()->protocol(), "file" ) == 0L &&
	 tmp == KFMPaths::TrashPath())
           isTrash = true;
    
    struct stat buff;
    stat( path, &buff );

    struct stat lbuff;
    lstat( path, &lbuff );

    QLabel *l;
    int y = 10;
 
    l = new QLabel( klocale->translate("Name"), this );
    l->setGeometry( 10, y, 200, 20 );
    y += 25;
    
    name = new QLineEdit( this );
    name->setGeometry( 10, y, 200, 30 );
    name->setText( filename );
    if ( isTrash )
	name->setEnabled( false );
    oldName = filename;
    oldName.detach();
    y += 35;

    l = new QLabel( klocale->translate("Full Name"), this );
    l->setGeometry( 10, y, 200, 20 );
    y += 25;
    
    fname = new QLineEdit( this );
    fname->setGeometry( 10, y, 200, 30 );
    fname->setText( path );
    fname->setEnabled( false );
    y += 35;
    
    y += 10;

    if ( isTrash )
    {
	l = new QLabel( klocale->translate( "Is the Trash Bin"), this );
	l->setGeometry( 10, y, 200, 20 );
	y += 25;
    }
    else if ( S_ISDIR( buff.st_mode ) )
    {
	l = new QLabel( klocale->translate("Is a Directory"), this );
	l->setGeometry( 10, y, 200, 20 );
	y += 25;
    }
    if ( S_ISLNK( lbuff.st_mode ) )
    {
	l = new QLabel( klocale->translate( "Points to" ), this );
	l->setGeometry( 10, y, 200, 20 );
	y += 25;
    
	lname = new QLineEdit( this );
	lname->setGeometry( 10, y, 200, 30 );
	lname->setText( path );
	lname->setEnabled( false );
	y += 35;

	char buffer[1024];
	int n = readlink( path, buffer, 1022 );
	if ( n > 0 )
	{
	    buffer[ n ] = 0;
	    lname->setText( buffer );
	}
    }
    else if ( S_ISREG( buff.st_mode ) )
    {
	char buffer[1024];
	int size = buff.st_size;
	sprintf( buffer, klocale->translate("Size: %i"), size );
	l = new QLabel( buffer, this );
	l->setGeometry( 10, y, 200, 20 );
	y += 25;
    }
    
    char buffer[1024];
    struct tm *t = localtime( &lbuff.st_atime );
    sprintf( buffer, "%s: %02i:%02i %02i.%02i.%04i", 
	     klocale->translate("Last Access"),
	     t->tm_hour,t->tm_min,
	     t->tm_mday,t->tm_mon + 1,t->tm_year + 1900 );             
    l = new QLabel( buffer, this );
    l->setGeometry( 10, y, 200, 20 );
    y += 25;

    t = localtime( &lbuff.st_mtime );
    sprintf( buffer, "%s: %02i:%02i %02i.%02i.%04i", 
	     klocale->translate("Last Modified"),
	     t->tm_hour,t->tm_min,
	     t->tm_mday,t->tm_mon + 1,t->tm_year + 1900 );          
    l = new QLabel( buffer, this );
    l->setGeometry( 10, y, 200, 20 );
    y += 25;
}

bool FilePropsPage::supports( KURL *_kurl )
{
    KURL u( _kurl->url().data() );
    KURL u2( u.nestedURL() );
    
    if ( strcmp( u2.protocol(), "file" ) != 0 )
	return false;

    return true;
}

void FilePropsPage::applyChanges()
{
    QString path = properties->getKURL()->path();
    KURL::decodeURL( path );
    QString fname = properties->getKURL()->filename();
    KURL::decodeURL( fname );

    // Do we need to rename the file ?
    if ( strcmp( oldName.data(), name->text() ) != 0 )
    {
	QString s = properties->getURL();
	s.detach();
	int i = s.findRev( "/" );
	// Should never happen
	if ( i == -1 )
	    return;
	s = s.left( i );
	QString tmp = s.data();
	tmp.detach();
	s += "/";
	s += name->text();
	KURL u( s.data() );
	QString t( u.path() );
	KURL::decodeURL( t );
	rename( path, t );
	properties->emitPropertiesChanged( name->text() );
    }
}

FilePermissionsPropsPage::FilePermissionsPropsPage( Properties *_props ) : PropsPage( _props )
{
    QString path = properties->getKURL()->path();
    KURL::decodeURL( path );
    QString fname = properties->getKURL()->filename();
    KURL::decodeURL( fname );

    struct stat buff;
    stat( path, &buff );
    struct passwd * user = getpwuid( buff.st_uid );
    struct group * g = getgrgid( buff.st_gid );
    
    permissions = buff.st_mode & ( S_IRWXU | S_IRWXG | S_IRWXO );
    strOwner = "";
    strGroup = "";
    if ( user != 0L )
    {
	strOwner = user->pw_name;
	strOwner.detach();
    }    
    if ( g != 0L )
    {
	strGroup = g->gr_name;
	strGroup.detach();
    }    

    QLabel *l;
    // QBoxLayout *bl2;
    int y = 10;
    
    l = new QLabel( klocale->translate("Access permissions"), this );
    l->setGeometry( 10, y, 200, 20 );
    y += 25;
    permUR = new QCheckBox( klocale->translate("User Read"), this );
    permUR->setGeometry( 10, y, 100, 30 );
    permUR->setChecked( ( buff.st_mode & S_IRUSR ) == S_IRUSR );
    permUW = new QCheckBox( klocale->translate("User Write"), this );
    permUW->setGeometry( 110, y, 100, 30 );
    permUW->setChecked( ( buff.st_mode & S_IWUSR ) == S_IWUSR );
    permUX = new QCheckBox( klocale->translate("User Exec"), this );
    permUX->setGeometry( 210, y, 100, 30 );
    permUX->setChecked( ( buff.st_mode & S_IXUSR ) == S_IXUSR );
    permUS = new QCheckBox( klocale->translate("Set UID"), this );
    permUS->setGeometry( 310, y, 100, 30 );
    permUS->setChecked( ( buff.st_mode & S_ISUID ) == S_ISUID );      
    y += 35;

    permGR = new QCheckBox( klocale->translate("Group Read"), this );
    permGR->setGeometry( 10, y, 100, 30 );
    permGR->setChecked( ( buff.st_mode & S_IRGRP ) == S_IRGRP );
    permGW = new QCheckBox( klocale->translate("Group Write"), this );
    permGW->setGeometry( 110, y, 100, 30 );
    permGW->setChecked( ( buff.st_mode & S_IWGRP ) == S_IWGRP );
    permGX = new QCheckBox( klocale->translate("Group Exec"), this );
    permGX->setGeometry( 210, y, 100, 30 );
    permGX->setChecked( ( buff.st_mode & S_IXGRP ) == S_IXGRP );
    permGS = new QCheckBox( klocale->translate("Set GID "), this );
    permGS->setGeometry( 310, y, 100, 30 );
    permGS->setChecked( ( buff.st_mode & S_ISGID ) == S_ISGID );   

    y += 35;
    permOR = new QCheckBox( klocale->translate("Others Read"), this );
    permOR->setGeometry( 10, y, 100, 30 );
    permOR->setChecked( ( buff.st_mode & S_IROTH ) == S_IROTH );
    permOW = new QCheckBox( klocale->translate("Others Write"), this );
    permOW->setGeometry( 110, y, 100, 30 );
    permOW->setChecked( ( buff.st_mode & S_IWOTH ) == S_IWOTH );
    permOX = new QCheckBox( klocale->translate("Others Exec"), this );
    permOX->setGeometry( 210, y, 100, 30 );
    permOX->setChecked( ( buff.st_mode & S_IXOTH ) == S_IXOTH );
    permOS = new QCheckBox( klocale->translate("Sticky"), this );
    permOS->setGeometry( 310, y, 100, 30 );
    permOS->setChecked( ( buff.st_mode & S_ISVTX ) == S_ISVTX );    
    y += 35;

    y += 10;
    
    l = new QLabel( klocale->translate("Owner"), this );
    l->setGeometry( 10, y, 100, 30 );
    owner = new QLineEdit( this );
    owner->setGeometry( 60, y, 100, 30 );
    owner->setText( strOwner );
    y += 35;
    l = new QLabel( klocale->translate("Group"), this );
    l->setGeometry( 10, y, 100, 30 );
    grp = new QLineEdit( this );
    grp->setGeometry( 60, y, 100, 30 );
    grp->setText( strGroup );
    y += 35;
}

bool FilePermissionsPropsPage::supports( KURL *_kurl )
{
    KURL u( _kurl->url() );
    KURL u2( u.nestedURL() );
    
    if ( strcmp( u2.protocol(), "file" ) != 0 )
	return false;

    return true;
}

void FilePermissionsPropsPage::applyChanges()
{
    QString path = properties->getKURL()->path();
    KURL::decodeURL( path );
    QString fname = properties->getKURL()->filename();
    KURL::decodeURL( fname );

    int p = 0L;
    if ( permUR->isChecked() )
	p |= S_IRUSR;
    if ( permUW->isChecked() )
	p |= S_IWUSR;
    if ( permUX->isChecked() )
	p |= S_IXUSR;
    if ( permUS->isChecked() )
        p |= S_ISUID;      
    if ( permGR->isChecked() )
	p |= S_IRGRP;
    if ( permGW->isChecked() )
	p |= S_IWGRP;
    if ( permGX->isChecked() )
	p |= S_IXGRP;
    if ( permGS->isChecked() )
        p |= S_ISGID;                        
    if ( permOR->isChecked() )
	p |= S_IROTH;
    if ( permOW->isChecked() )
	p |= S_IWOTH;
    if ( permOX->isChecked() )
	p |= S_IXOTH;
    if ( permOS->isChecked() )
        p |= S_ISVTX;          

    if ( p != permissions )
    {
	struct stat buff;
	stat( path, &buff );
	// int mask = ~( S_IRWXU | S_IRWXG | S_IRWXO );
	// mask |= p;
	chmod( path, p );
    }
    
    if ( strcmp( owner->text(), strOwner.data() ) != 0 || strcmp( grp->text(), strGroup.data() ) != 0 )
    {
	struct passwd* pw = getpwnam( owner->text() );
	struct group* g = getgrnam( grp->text() );
	if ( pw == 0L )
	{
	    warning(klocale->translate(" ERROR: No user %s"),owner->text() );
	    return;
	}
	if ( g == 0L )
	{
	    warning(klocale->translate(" ERROR: No group %s"),grp->text() );
	    return;
	}
	chown( path, pw->pw_uid, g->gr_gid );
    }
}

ExecPropsPage::ExecPropsPage( Properties *_props ) : PropsPage( _props )
{
    execEdit = new QLineEdit( this, "LineEdit_1" );
    pathEdit = new QLineEdit( this, "LineEdit_2" );
    iconBox = new QComboBox( false, this, "ComboBox_1" );
    terminalCheck = new QCheckBox( this, "CheckBox_1" );
    terminalEdit = new QLineEdit( this, "LineEdit_4" );
    execBrowse = new QPushButton( this, "Button_1" );
    // iconView = new QWidget( this );
    
    QGroupBox* tmpQGroupBox;
    tmpQGroupBox = new QGroupBox( this, "GroupBox_1" );
    tmpQGroupBox->setGeometry( 10, 200, 320, 120 );
    tmpQGroupBox->setFrameStyle( 49 );
    tmpQGroupBox->setAlignment( 1 );

    execEdit->raise();
    execEdit->setGeometry( 10, 40, 210, 30 );
    execEdit->setText( "" );
    execEdit->setMaxLength( 256 );

    QLabel* tmpQLabel;
    tmpQLabel = new QLabel( this, "Label_1" );
    tmpQLabel->setGeometry( 10, 10, 100, 30 );
    tmpQLabel->setText( klocale->translate("Execute") );

    tmpQLabel = new QLabel( this, "Label_2" );
    tmpQLabel->setGeometry( 10, 130, 100, 30 );
    tmpQLabel->setText( klocale->translate("Icon") );

    tmpQLabel = new QLabel( this, "Label_3" );
    tmpQLabel->setGeometry( 10, 70, 120, 30 );
    tmpQLabel->setText( klocale->translate("Working Directory") );

    pathEdit->raise();
    pathEdit->setGeometry( 10, 100, 210, 30 );
    pathEdit->setMaxLength( 256 );
    
    iconBox->raise();
    iconBox->setGeometry( 10, 160, 120, 30 );

    // iconView->raise();
    // iconView->setGeometry( 140, 160, 32, 32 );

    terminalCheck->raise();
    terminalCheck->setGeometry( 20, 210, 150, 30 );
    terminalCheck->setText( klocale->translate("Run in terminal") );

    terminalEdit->raise();
    terminalEdit->setGeometry( 20, 280, 300, 30 );
    terminalEdit->setText( "" );

    tmpQLabel = new QLabel( this, "Label_5" );
    tmpQLabel->setGeometry( 20, 250, 100, 30 );
    tmpQLabel->setText( klocale->translate("Terminal Options") );

    execBrowse->raise();
    execBrowse->setGeometry( 230, 40, 100, 30 );
    execBrowse->setText( klocale->translate("Browse") );

    QFile f( _props->getKURL()->path() );
    if ( !f.open( IO_ReadOnly ) )
	return;
    
    QTextStream pstream( &f );
    KConfig config( &pstream );
    config.setGroup( "KDE Desktop Entry" );
    execStr = config.readEntry( "Exec" );
    pathStr = config.readEntry( "Path" );
    iconStr = config.readEntry( "Icon" );
    termStr = config.readEntry( "Terminal" );
    termOptionsStr = config.readEntry( "TerminalOptions" );

    if ( !pathStr.isNull() )
	pathEdit->setText( pathStr.data() );
    if ( !execStr.isNull() )
	execEdit->setText( execStr.data() );
    if ( !termStr.isNull() )
	terminalEdit->setText( termOptionsStr.data() );
    if ( !termStr.isNull() )
	terminalCheck->setChecked( termStr == "1" );
    if ( iconStr.isNull() )
	iconStr = KMimeType::getExecutablePixmap();
    
    // Load all pixmaps files in the combobox
    QDir d( KMimeType::getIconPath() );
    const QFileInfoList *list = d.entryInfoList();
    QFileInfoListIterator it( *list );      // create list iterator
    QFileInfo *fi;                          // pointer for traversing

    int index = -1;
    int i = 0;  
    while ( ( fi = it.current() ) )
    {
	// Is this the currently selected icon ?
	if ( strcmp( iconStr.data(), fi->fileName().data() ) == 0 )
	    index = i;
	iconBox->insertItem( fi->fileName().data(), i );
	i++;
	++it;                               // goto next list element
    }
    // The currently selected icon is not in the list .... strange ... ? Lets add it.
    if ( index == -1 && iconStr.length() > 0 )
    {
	iconBox->insertItem( iconStr.data(), i );
	index = i;
    }
    // Select the current icon
    iconBox->setCurrentItem( index );
    drawIcon();

    connect( iconBox, SIGNAL( activated( int ) ), this, SLOT( slotIconChanged( int ) ) );
    connect( execBrowse, SIGNAL( pressed() ), this, SLOT( slotBrowseExec() ) );
}

bool ExecPropsPage::supports( KURL *_kurl )
{
    KURL u( _kurl->url() );
    KURL u2( u.nestedURL() );
    
    if ( strcmp( u2.protocol(), "file" ) != 0 )
	return false;

    /* FILE *fh = fopen( _kurl->path(), "rb" );
    if ( fh == 0L )
	return false;
    
    char buffer[ 1024 ];
    buffer[0] = 0;
    fgets( buffer, 1023, fh );
    fclose( fh );
    
    if ( strstr( buffer, "[KDE Desktop Entry]" ) == 0L )
	return false; */

    QString t( _kurl->path() );
    KURL::decodeURL( t );
    QFile f( t );

    struct stat buff;
    stat( t, &buff );

    struct stat lbuff;
    lstat( t, &lbuff );

    if ( !S_ISREG( buff.st_mode ) || S_ISDIR( lbuff.st_mode ) )
	return false;

    if ( !f.open( IO_ReadOnly ) )
	return false;
    
    QTextStream pstream( &f );
    KConfig config( &pstream );
    config.setGroup( "KDE Desktop Entry" );

    QString type = config.readEntry( "Type" );
    if ( type.isNull() )
	return false;
    if ( type != "Application" )
	return false;
    
    return true;
}

void ExecPropsPage::applyChanges()
{
    QString path = properties->getKURL()->path();
    KURL::decodeURL( path );

    QFile f( path );
    if ( !f.open( IO_ReadWrite ) )
    {
	QMessageBox::message( klocale->translate("KFM Error"),
			      klocale->translate("Could not save properties\nPerhaps permissions denied") );
	return;
    }
    
    QTextStream pstream( &f );
    KConfig config( &pstream );
    config.setGroup( "KDE Desktop Entry" );
    config.writeEntry( "Exec", execEdit->text() );
    config.writeEntry( "Path", pathEdit->text() );

    int i = iconBox->currentItem();
    if ( i != -1 )
	config.writeEntry( "Icon", iconBox->text( i )  );

    if ( terminalCheck->isChecked() )
	config.writeEntry( "Terminal", "1" );
    else
	config.writeEntry( "Terminal", "0" );

    config.writeEntry( "TerminalOptions", terminalEdit->text() );
    config.sync();
    f.close();
}


void ExecPropsPage::slotBrowseExec()
{
    QString f = QFileDialog::getOpenFileName( 0, 0L, this );
    if ( f.isNull() )
	return;

    execEdit->setText( f.data() );
}


void ExecPropsPage::slotIconChanged( int )
{
    drawIcon();
}

void ExecPropsPage::paintEvent( QPaintEvent *_ev )
{
    QWidget::paintEvent( _ev );
    drawIcon();
}

void ExecPropsPage::drawIcon()
{
    int i = iconBox->currentItem();
    if ( i == -1 )
	return;
    
    const char *text = iconBox->text( i );
    QString file = KMimeType::getIconPath();
    file += "/";
    file += text;
    
    if ( file != pixmapFile )
    {
	pixmapFile = file.data();
	pixmapFile.detach();	
	pixmap.load( file.data() );
    }
    
    erase( 140, 140, 100, 84 );
    QPainter painter;
    painter.begin( this );
    painter.setClipRect( 140, 140, 100, 84 );
    painter.drawPixmap( QPoint( 140, 140 ), pixmap );
    painter.end();
}

URLPropsPage::URLPropsPage( Properties *_props ) : PropsPage( _props )
{
    URLEdit = new QLineEdit( this, "LineEdit_1" );
    iconBox = new QComboBox( false, this, "ComboBox_1" );

    URLEdit->raise();
    URLEdit->setGeometry( 10, 40, 210, 30 );
    URLEdit->setText( "" );
    URLEdit->setMaxLength( 256 );

    QLabel* tmpQLabel;
    tmpQLabel = new QLabel( this, "Label_1" );
    tmpQLabel->setGeometry( 10, 10, 100, 30 );
    tmpQLabel->setText( klocale->translate("URL") );
    
    iconBox->raise();
    iconBox->setGeometry( 10, 90, 120, 30 );

    QString path = _props->getKURL()->path();
    KURL::decodeURL( path );

    QFile f( path );
    if ( !f.open( IO_ReadOnly ) )
	return;
    
    QTextStream pstream( &f );
    KConfig config( &pstream );
    config.setGroup( "KDE Desktop Entry" );
    URLStr = config.readEntry(  "URL" );
    iconStr = config.readEntry( "Icon" );

    if ( !URLStr.isNull() )
	URLEdit->setText( URLStr.data() );
    if ( iconStr.isNull() )
	iconStr = KMimeType::getDefaultPixmap();
    
    // Load all pixmaps files in the combobox
    QDir d( KMimeType::getIconPath() );
    const QFileInfoList *list = d.entryInfoList();
    QFileInfoListIterator it( *list );      // create list iterator
    QFileInfo *fi;                          // pointer for traversing

    int index = -1;
    int i = 0;  
    while ( ( fi = it.current() ) )
    {
	if ( fi->fileName() != ".." && fi->fileName() != "." )
	{
	    // Is this the currently selected icon ?
	    if ( strcmp( iconStr.data(), fi->fileName().data() ) == 0 )
		index = i;
	    iconBox->insertItem( fi->fileName().data(), i );
	    i++;
	}
	++it;                               // goto next list element
    }
    // The currently selected icon is not in the list .... strange ... ? Lets add it.
    if ( index == -1 && iconStr.length() > 0 )
    {
	iconBox->insertItem( iconStr.data(), i );
	index = i;
    }
    // Select the current icon
    iconBox->setCurrentItem( index );
    drawIcon();

    connect( iconBox, SIGNAL( activated( int ) ), this, SLOT( slotIconChanged( int ) ) );
}

bool URLPropsPage::supports( KURL *_kurl )
{
    KURL u( _kurl->url() );
    KURL u2( u.nestedURL() );
    
    if ( strcmp( u2.protocol(), "file" ) != 0 )
	return false;

    /* FILE *fh = fopen( _kurl->path(), "rb" );
    if ( fh == 0L )
	return false;
    
    char buffer[ 1024 ];
    buffer[0] = 0;
    fgets( buffer, 1023, fh );
    fclose( fh );
    
    if ( strstr( buffer, "[KDE Desktop Entry]" ) == 0L )
	return false; */

    QString path = _kurl->path();
    KURL::decodeURL( path );
    QFile f( path );

    struct stat buff;
    stat( path, &buff );

    struct stat lbuff;
    lstat( path, &lbuff );

    if ( !S_ISREG( buff.st_mode ) || S_ISDIR( lbuff.st_mode ) )
	return false;
    if ( !f.open( IO_ReadOnly ) )
	return false;
    
    QTextStream pstream( &f );
    KConfig config( &pstream );
    config.setGroup( "KDE Desktop Entry" );

    QString URL = config.readEntry( "URL" );
    if ( URL.isNull() )
	return false;
    
    return true;    
}

void URLPropsPage::applyChanges()
{
    QString path = properties->getKURL()->path();
    KURL::decodeURL( path );

    QFile f( path );
    if ( !f.open( IO_ReadWrite ) )
    {
	QMessageBox::message(  klocale->translate("KFM Error"), 
			        klocale->translate("Could not save properties\nPerhaps permissions denied") );
	return;
    }
    
    QTextStream pstream( &f );
    KConfig config( &pstream );
    config.setGroup( "KDE Desktop Entry" );
    config.writeEntry( "URL", URLEdit->text() );

    int i = iconBox->currentItem();
    if ( i != -1 )
	config.writeEntry( "Icon", iconBox->text( i )  );

    config.sync();
    f.close();
}

void URLPropsPage::slotIconChanged( int )
{
    drawIcon();
}

void URLPropsPage::paintEvent( QPaintEvent *_ev )
{
    QWidget::paintEvent( _ev );
    drawIcon();
}

void URLPropsPage::drawIcon()
{
    int i = iconBox->currentItem();
    if ( i == -1 )
	return;
    
    const char *text = iconBox->text( i );
    QString file = KMimeType::getIconPath();
    file += "/";
    file += text;
    
    if ( file != pixmapFile )
    {
	pixmapFile = file.data();
	pixmapFile.detach();	
	pixmap.load( file.data() );
    }
    
    erase( 140, 90, 64, 64 );
    QPainter painter;
    painter.begin( this );
    painter.setClipRect( 140, 90, 64, 64 );
    painter.drawPixmap( QPoint( 140, 90 ), pixmap );
    painter.end();
}


DirPropsPage::DirPropsPage( Properties *_props ) : PropsPage( _props )
{
    iconBox = new QComboBox( false, this, "ComboBox_1" );
    iconBox->raise();
    iconBox->setGeometry( 10, 20, 120, 30 );

    wallBox = new QComboBox( false, this, "ComboBox_2" );
    wallBox->raise();
    wallBox->setGeometry( 10, 90, 120, 30 );

    applyButton = new QPushButton(  klocale->translate("Apply") , this );
    applyButton->setGeometry( 10, 230, 120, 30 );
    connect( applyButton, SIGNAL( clicked() ), this, SLOT( slotApply() ) );
    
    globalButton = new QPushButton(  klocale->translate("Apply global"),
				     this );
    globalButton->setGeometry( 140, 230, 120, 30 );
    connect( globalButton, SIGNAL( clicked() ), this, SLOT( slotApplyGlobal() ) );

    QString tmp = _props->getKURL()->path();
    KURL::decodeURL( tmp );
    if ( tmp.right(1) != "/" )
	tmp += "/.directory";
    else
	tmp += ".directory";

    QFile f( tmp.data() );
    if ( f.open( IO_ReadOnly ) )
    {
	QTextStream pstream( &f );
	KConfig config( &pstream );
	config.setGroup( "KDE Desktop Entry" );
	wallStr = config.readEntry( "BgImage" );
	iconStr = config.readEntry( "Icon" );
    }
    
    if ( iconStr.isNull() )
	iconStr = KMimeType::getFolderPixmap();
    
    // Load all pixmaps files in the combobox
    QDir d( KMimeType::getIconPath() );
    const QFileInfoList *list = d.entryInfoList();
    QFileInfoListIterator it( *list );      // create list iterator
    QFileInfo *fi;                          // pointer for traversing

    int index = -1;
    int i = 0;  
    while ( ( fi = it.current() ) )
    {
	if ( fi->fileName() != ".." && fi->fileName() != "." )
	{
	    // Is this the currently selected icon ?
	    if ( strcmp( iconStr.data(), fi->fileName().data() ) == 0 )
		index = i;
	    iconBox->insertItem( fi->fileName().data(), i );
	    i++;
	}
	++it;                               // goto next list element
    }
    // The currently selected icon is not in the list .... strange ... ? Lets add it.
    if ( index == -1 && iconStr.length() > 0 )
    {
	iconBox->insertItem( iconStr.data(), i );
	index = i;
    }
    // Select the current icon
    iconBox->setCurrentItem( index );
    drawIcon();

    connect( iconBox, SIGNAL( activated( int ) ), this, SLOT( slotIconChanged( int ) ) );

    // Load all wallpapers in the combobox
    tmp = kapp->kdedir();
    tmp += "/share/wallpapers";
    QDir d2( tmp.data() );
    list = d2.entryInfoList();
    QFileInfoListIterator it2( *list );      // create list iterator

    wallBox->insertItem(  klocale->translate("(None)"), 0 );
    
    index = -1;
    i = 1;  
    while ( ( fi = it2.current() ) )
    {
	if ( fi->fileName() != ".." && fi->fileName() != "." )
	{
	    // Is this the currently selected wallpaper ?
	    if ( wallStr == fi->fileName() )
		index = i;
	    wallBox->insertItem( fi->fileName().data(), i );
	    i++;
	}
	++it2;                               // goto next list element
    }
    // The currently selected image is not in the list .... strange ... ? Lets add it.
    if ( index == -1 && wallStr.length() > 0 )
    {
	wallBox->insertItem( wallStr.data(), i );
	index = i;
    }
    // Select the current icon
    if ( index != -1 )
	wallBox->setCurrentItem( index );
    drawWallPaper();

    connect( wallBox, SIGNAL( activated( int ) ), this, SLOT( slotWallPaperChanged( int ) ) );
}

bool DirPropsPage::supports( KURL *_kurl )
{
    // Is it the trash bin ?
    QString path = _kurl->path();
    KURL::decodeURL( path );

    QString tmp = path.data();
    if ( tmp.right(1) != "/" )
	tmp += "/";
    if ( strcmp( _kurl->protocol(), "file" ) == 0L &&
	 tmp == KFMPaths::TrashPath()) 
        return false;

    KURL u( _kurl->url() );
    KURL u2( u.nestedURL() );
    
    if ( strcmp( u2.protocol(), "file" ) != 0 )
	return false;

    if ( !KIOServer::isDir( path ) )
	return false;
    
    return true;    
}

void DirPropsPage::applyChanges()
{
    QString tmp = properties->getKURL()->path();
    KURL::decodeURL( tmp );
    if ( tmp.right(1) != "/" )
	tmp += "/.directory";
    else
	tmp += ".directory";

    QFile f( tmp.data() );
    if ( !f.open( IO_ReadWrite ) )
    {
      QMessageBox::message(  klocale->translate("Error"), 
			     klocale->translate("Could not write to\n") + tmp );
	return;
    }
    
    QTextStream pstream( &f );
    KConfig config( &pstream );
    config.setGroup( "KDE Desktop Entry" );

    int i = wallBox->currentItem();
    if ( i != -1 )
    {
	if ( strcmp( wallBox->text( i ),  klocale->translate("(None)") ) == 0 )
	    config.writeEntry( "BgImage", "" );
	else
	    config.writeEntry( "BgImage", wallBox->text( i ) );
    }
    
    i = iconBox->currentItem();
    if ( i != -1 )
	config.writeEntry( "Icon", iconBox->text( i )  );

    config.sync();
    f.close();

    // Send a notify to the parent directory since the
    // icon may have changed

    tmp = properties->getKURL()->url();
    tmp.detach();
    
    if ( tmp.right(1) == "/" )
	tmp = tmp.left( tmp.length() - 1 );
    
    i = tmp.findRev( "/" );
    // Should never happen
    if ( i == -1 )
	return;
    tmp = tmp.left( i + 1 );

    debugT("$$$$$$$$$$$$$ Sending notify to '%s'\n", tmp.data());
    KIOServer::sendNotify( tmp.data() );
}

void DirPropsPage::slotIconChanged( int )
{
    drawIcon();
}

void DirPropsPage::slotWallPaperChanged( int )
{
    drawWallPaper();
}

void DirPropsPage::paintEvent( QPaintEvent *_ev )
{
    QWidget::paintEvent( _ev );
    drawIcon();
    drawWallPaper();
}

void DirPropsPage::drawIcon()
{
    int i = iconBox->currentItem();
    if ( i == -1 )
    {
	erase( 140, 20, 64, 64 );
	return;
    }
    
    const char *text = iconBox->text( i );
    QString file = KMimeType::getIconPath();
    file += "/";
    file += text;
    
    if ( file != pixmapFile )
    {
	pixmapFile = file.data();
	pixmapFile.detach();	
	pixmap.load( file.data() );
    }
    
    erase( 140, 20, 64, 64 );
    QPainter painter;
    painter.begin( this );
    painter.setClipRect( 140, 20, 64, 64 );
    painter.drawPixmap( QPoint( 140, 20 ), pixmap );
    painter.end();
}

void DirPropsPage::drawWallPaper()
{
    int i = wallBox->currentItem();
    if ( i == -1 )
    {
	erase( 140, 90, 128, 128 );
	return;
    }
    
    const char *text = wallBox->text( i );
    if ( strcmp( text, "(None)" ) == 0 )
    {
	erase( 140, 90, 128, 128 );
	return;
    }

    QString file = kapp->kdedir();
    file += "/share/wallpapers/";
    file += text;
    
    if ( file != wallFile )
    {
	debugT("Loading WallPaper '%s'\n",file.data());
	wallFile = file.data();
	wallFile.detach();	
	wallPixmap.load( file.data() );
    }
    
    if ( wallPixmap.isNull() )
	debugT("Could not load\n");
    
    erase( 140, 90, 128, 128 );
    QPainter painter;
    painter.begin( this );
    painter.setClipRect( 140, 90, 128, 128 );
    painter.drawPixmap( QPoint( 140, 90 ), wallPixmap );
    painter.end();
}

void DirPropsPage::slotApply()
{
    applyChanges();

    KIOServer::sendNotify( properties->getURL() );
}

void DirPropsPage::slotApplyGlobal()
{
    KConfig *config = KApplication::getKApplication()->getConfig();
    
    config->setGroup( "KFM HTML Defaults" );

    int i = wallBox->currentItem();
    if ( i != -1 )
    {
	if ( strcmp( wallBox->text( i ),  klocale->translate("(None)") ) == 0 )
	    config->writeEntry( "BgImage", "" );
	else
	    config->writeEntry( "BgImage", wallBox->text( i ) );
    }

    config->setGroup( "Icons" );

    i = iconBox->currentItem();
    if ( i != -1 )
	config->writeEntry( "Icon", iconBox->text( i )  );

    config->sync();

    // Notify all opened windows
    QStrList strlist;
    QList<KfmGui>& list = KfmGui::getWindowList();
    KfmGui *win;
    for ( win = list.first(); win != 0L; win = list.next() )
	strlist.append( win->getURL() );
    
    char *s;
    for ( s = strlist.first(); s != 0L; s = strlist.next() )
	KIOServer::sendNotify( s );
}

/* ----------------------------------------------------
 *
 * ApplicationPropsPage
 *
 * -------------------------------------------------- */

ApplicationPropsPage::ApplicationPropsPage( Properties *_props ) : PropsPage( _props )
{
    binaryPatternEdit = new QLineEdit( this, "LineEdit_1" );
    commentEdit = new QLineEdit( this, "LineEdit_2" );

    extensionsList = new QListBox( this );
    availableExtensionsList = new QListBox( this );
    addExtensionButton = new QPushButton( "<-", this );
    delExtensionButton = new QPushButton( "->", this );

    protocolFTP = new QCheckBox( this );
    protocolFTP->setText( klocale->translate("FTP") );
    protocolFILE = new QCheckBox( this );
    protocolFILE->setText( klocale->translate("FILE") );
    protocolHTTP = new QCheckBox( this );
    protocolHTTP->setText( klocale->translate("HTTP") );
    protocolTAR = new QCheckBox( this );
    protocolTAR->setText( klocale->translate("TAR") );
    protocolMAN = new QCheckBox( this );
    protocolMAN->setText( klocale->translate("MAN") );
    protocolINFO = new QCheckBox( this );
    protocolINFO->setText( klocale->translate("INFO") );

    QGroupBox* tmpQGroupBox;
    tmpQGroupBox = new QGroupBox( this, "GroupBox_1" );
    tmpQGroupBox->setGeometry( 10, 140, 350, 70 );
    tmpQGroupBox->setFrameStyle( 49 );
    tmpQGroupBox->setAlignment( 1 );

    protocolFILE->setGeometry( 20, 150, 100, 20 );
    protocolFILE->raise();
    protocolFTP->setGeometry( 120, 150, 100, 20 );
    protocolFTP->raise();
    protocolHTTP->setGeometry( 220, 150, 100, 20 );    
    protocolHTTP->raise();
    protocolTAR->setGeometry( 20, 180, 100, 20 );
    protocolTAR->raise();
    protocolINFO->setGeometry( 120, 180, 100, 20 );
    protocolINFO->raise();
    protocolMAN->setGeometry( 220, 180, 100, 20 );
    protocolMAN->raise();

    binaryPatternEdit->raise();
    binaryPatternEdit->setGeometry( 10, 40, 210, 30 );
    binaryPatternEdit->setText( "" );
    binaryPatternEdit->setMaxLength( 512 );

    QLabel* tmpQLabel;
    tmpQLabel = new QLabel( this, "Label_1" );
    tmpQLabel->setGeometry( 10, 10, 300, 30 );
    tmpQLabel->setText(  klocale->translate("Binary Pattern ( netscape;Netscape; )") );

    tmpQLabel = new QLabel( this, "Label_3" );
    tmpQLabel->setGeometry( 10, 70, 120, 30 );
    tmpQLabel->setText(  klocale->translate("Comment") );

    commentEdit->raise();
    commentEdit->setGeometry( 10, 100, 210, 30 );
    commentEdit->setMaxLength( 256 );
    
    extensionsList->setGeometry( 10, 220, 130, 100 );
    availableExtensionsList->setGeometry( 230, 220, 130, 100 );
    addExtensionButton->setGeometry( 160, 230, 40, 40 );
    connect( addExtensionButton, SIGNAL( pressed() ), this, SLOT( slotAddExtension() ) );
    delExtensionButton->setGeometry( 160, 270, 40, 40 );    
    connect( delExtensionButton, SIGNAL( pressed() ), this, SLOT( slotDelExtension() ) );

    QString path = _props->getKURL()->path() ;
    KURL::decodeURL( path );	    
    QFile f( path );
    if ( !f.open( IO_ReadOnly ) )
	return;
    
    QTextStream pstream( &f );
    KConfig config( &pstream );
    config.setGroup( "KDE Desktop Entry" );
    commentStr = config.readEntry( "Comment" );
    binaryPatternStr = config.readEntry( "BinaryPattern" );
    protocolsStr = config.readEntry( "Protocols" );
    extensionsStr = config.readEntry( "MimeType" );

    if ( !commentStr.isNull() )
	commentEdit->setText( commentStr.data() );
    if ( !binaryPatternStr.isNull() )
	binaryPatternEdit->setText( binaryPatternStr.data() );
    if ( !extensionsStr.isNull() )
    {
	int pos = 0;
	int pos2 = 0;
	while ( ( pos2 = extensionsStr.find( ";", pos ) ) != -1 )
	{
	    extensionsList->inSort( extensionsStr.mid( pos, pos2 - pos ) );
	    pos = pos2 + 1;
	}
    }
    if ( !protocolsStr.isNull() )
    {
	if ( protocolsStr.find( "file;" ) == 0 || protocolsStr.find( ";file;" ) != -1 )
	    protocolFILE->setChecked( true );
	if ( protocolsStr.find( "ftp;" ) == 0 || protocolsStr.find( ";ftp;" ) != -1 )
	    protocolFTP->setChecked( true );
	if ( protocolsStr.find( "http;" ) == 0 || protocolsStr.find( ";http;" ) != -1 )
	    protocolHTTP->setChecked( true );
	if ( protocolsStr.find( "tar;" ) == 0 || protocolsStr.find( ";tar;" ) != -1 )
	    protocolTAR->setChecked( true );
	if ( protocolsStr.find( "man;" ) == 0 || protocolsStr.find( ";man;" ) != -1 )
	    protocolMAN->setChecked( true );
	if ( protocolsStr.find( "info;" ) == 0 || protocolsStr.find( ";info;" ) != -1 )
	    protocolINFO->setChecked( true );
    }

    KMimeType *ft;
    for ( ft = KMimeType::getFirstMimeType(); ft != 0L; ft = KMimeType::getNextMimeType() )
    {
	const char *name = ft->getMimeType();
	bool insert = true;
	
	for ( uint i = 0; i < extensionsList->count(); i++ )
	    if ( strcmp( name, extensionsList->text( i ) ) == 0 )
		insert = false;
	
	if ( insert && !ft->isApplicationPattern() )
	    availableExtensionsList->inSort( name );
    }
}

bool ApplicationPropsPage::supports( KURL *_kurl )
{
    KURL u( _kurl->url() );
    KURL u2( u.nestedURL() );
    
    if ( strcmp( u2.protocol(), "file" ) != 0 )
	return false;

    /* FILE *fh = fopen( _kurl->path(), "rb" );
    if ( fh == 0L )
	return false;
    
    char buffer[ 1024 ];
    buffer[0] = 0;
    fgets( buffer, 1023, fh );
    fclose( fh );
    
    if ( strstr( buffer, "[KDE Desktop Entry]" ) == 0L )
	return false; */

    QString path = _kurl->path();
    KURL::decodeURL( path );
    QFile f( path );

    struct stat buff;
    stat( path, &buff );

    struct stat lbuff;
    lstat( path, &lbuff );

    if ( !S_ISREG( buff.st_mode ) || S_ISDIR( lbuff.st_mode ) )
	return false;
    if ( !f.open( IO_ReadOnly ) )
	return false;
    
    QTextStream pstream( &f );
    KConfig config( &pstream );
    config.setGroup( "KDE Desktop Entry" );

    QString type = config.readEntry( "Type" );
    if ( type.isNull() )
	return false;
    if ( type != "Application" )
	return false;
    
    return true;
}

void ApplicationPropsPage::applyChanges()
{
    QString path = properties->getKURL()->path();
    KURL::decodeURL( path );

    QFile f( path );
    if ( !f.open( IO_ReadWrite ) )
    {
	QMessageBox::message(  klocale->translate("KFM Error"), 
			        klocale->translate("Could not save properties\nPerhaps permissions denied") );
	return;
    }
    
    QTextStream pstream( &f );
    KConfig config( &pstream );
    config.setGroup( "KDE Desktop Entry" );
    config.writeEntry( "Comment", commentEdit->text() );

    QString tmp = binaryPatternEdit->text();
    if ( tmp.length() > 0 )
	if ( tmp.right(1) != ";" )
	    tmp += ";";
    config.writeEntry( "BinaryPattern", tmp.data() );

    protocolsStr = "";
    if ( protocolFILE->isChecked() )
	protocolsStr += "file;";
    if ( protocolFTP->isChecked() )
	protocolsStr += "ftp;";
    if ( protocolHTTP->isChecked() )
	protocolsStr += "http;";
    if ( protocolTAR->isChecked() )
	protocolsStr += "tar;";
    if ( protocolMAN->isChecked() )
	protocolsStr += "man;";
    if ( protocolINFO->isChecked() )
	protocolsStr += "info;";
    config.writeEntry( "Protocols", protocolsStr.data() );

    extensionsStr = "";
    for ( uint i = 0; i < extensionsList->count(); i++ )
    {
	extensionsStr += extensionsList->text( i );
	extensionsStr += ";";
    }
    config.writeEntry( "MimeType", extensionsStr.data() );
    
    config.sync();
    f.close();

    KMimeType::clearAll();
    KMimeType::init();
    KRootWidget::getKRootWidget()->update();

    KfmGui *win;
    for ( win = KfmGui::getWindowList().first(); win != 0L; win = KfmGui::getWindowList().next() )
	win->updateView();
}

void ApplicationPropsPage::slotAddExtension()
{
    int pos = availableExtensionsList->currentItem();
   
    if ( pos == -1 )
	return;
    
    extensionsList->inSort( availableExtensionsList->text( pos ) );
    availableExtensionsList->removeItem( pos );
}

void ApplicationPropsPage::slotDelExtension()
{
    int pos = extensionsList->currentItem();
   
    if ( pos == -1 )
	return;
    
    availableExtensionsList->inSort( extensionsList->text( pos ) );
    extensionsList->removeItem( pos );
}

/* ----------------------------------------------------
 *
 * BindingPropsPage
 *
 * -------------------------------------------------- */

BindingPropsPage::BindingPropsPage( Properties *_props ) : PropsPage( _props )
{
    patternEdit = new QLineEdit( this, "LineEdit_1" );
    commentEdit = new QLineEdit( this, "LineEdit_3" );
    mimeEdit = new QLineEdit( this, "LineEdit_3" );
    iconBox = new QComboBox( false, this, "ComboBox_1" );
    appBox = new QComboBox( false, this, "ComboBox_2" );

    patternEdit->raise();
    patternEdit->setGeometry( 10, 40, 210, 30 );
    patternEdit->setText( "" );
    patternEdit->setMaxLength( 512 );

    commentEdit->raise();
    commentEdit->setGeometry( 10, 100, 210, 30 );
    commentEdit->setMaxLength( 256 );

    mimeEdit->raise();
    mimeEdit->setGeometry( 10, 160, 210, 30 );
    mimeEdit->setMaxLength( 256 );

    QLabel* tmpQLabel;
    tmpQLabel = new QLabel( this, "Label_1" );
    tmpQLabel->setGeometry( 10, 10, 300, 30 );
    tmpQLabel->setText(  klocale->translate("Pattern ( example: *.html;*.HTML; )") );

    tmpQLabel = new QLabel( this, "Label_2" );
    tmpQLabel->setGeometry( 180, 210, 100, 30 );
    tmpQLabel->setText(  klocale->translate("Icon") );

    tmpQLabel = new QLabel( this, "Label_2" );
    tmpQLabel->setGeometry( 10, 130, 100, 30 );
    tmpQLabel->setText(  klocale->translate("Mime Type") );

    tmpQLabel = new QLabel( this, "Label_3" );
    tmpQLabel->setGeometry( 10, 70, 120, 30 );
    tmpQLabel->setText(  klocale->translate("Comment") );
    
    iconBox->raise();
    iconBox->setGeometry( 180, 240, 120, 30 );

    tmpQLabel = new QLabel( this, "Label_2" );
    tmpQLabel->setGeometry( 10, 210, 170, 30 );
    tmpQLabel->setText(  klocale->translate("Default Application") );

    appBox->raise();
    appBox->setGeometry( 10, 240, 120, 30 );
    
    QFile f( _props->getKURL()->path() );
    if ( !f.open( IO_ReadOnly ) )
	return;
    
    QTextStream pstream( &f );
    KConfig config( &pstream );
    config.setGroup( "KDE Desktop Entry" );
    patternStr = config.readEntry( "Patterns" );
    appStr = config.readEntry( "DefaultApp" );
    iconStr = config.readEntry( "Icon" );
    commentStr = config.readEntry( "Comment" );
    mimeStr = config.readEntry( "MimeType" );

    if ( !patternStr.isNull() )
	patternEdit->setText( patternStr.data() );
    //if ( !appStr.isNull() )

    if ( !commentStr.isNull() )
	commentEdit->setText( commentStr.data() );
    if ( iconStr.isNull() )
	iconStr = KMimeType::getDefaultPixmap();
    if ( !mimeStr.isNull() )
	mimeEdit->setText( mimeStr.data() );
    
    // Load all pixmaps files in the combobox
    QDir d( KMimeType::getIconPath() );
    const QFileInfoList *list = d.entryInfoList();
    QFileInfoListIterator it( *list );      // create list iterator
    QFileInfo *fi;                          // pointer for traversing

    int index = -1;
    int i = 0;  
    while ( ( fi = it.current() ) )
    {
	// Is this the currently selected icon ?
	if ( strcmp( iconStr.data(), fi->fileName().data() ) == 0 )
	    index = i;
	iconBox->insertItem( fi->fileName().data(), i );
	i++;
	++it;                               // goto next list element
    }
    // The currently selected icon is not in the list .... strange ... ? Lets add it.
    if ( index == -1 && iconStr.length() > 0 )
    {
	iconBox->insertItem( iconStr.data(), i );
	index = i;
    }
    // Select the current icon
    iconBox->setCurrentItem( index );
    drawIcon();

    // Get list of all applications
    index = -1;
    i = 0;
    const char *p;
    for ( p = KMimeBind::getFirstApplication(); p != 0L; p = KMimeBind::getNextApplication() )
    {
	if ( appStr.data() != 0L )
	    if ( strcmp( p, appStr.data() ) == 0 )
		index = i;

	appBox->insertItem( p );
	i++;
    }
    appBox->insertItem( "" );
    
    // Set the default app
    if ( index == -1 )
	index = i;
    appBox->setCurrentItem( index );
    
    connect( iconBox, SIGNAL( activated( int ) ), this, SLOT( slotIconChanged( int ) ) );
}

bool BindingPropsPage::supports( KURL *_kurl )
{
    KURL u( _kurl->url() );
    KURL u2( u.nestedURL() );
    
    if ( strcmp( u2.protocol(), "file" ) != 0 )
	return false;

    /* FILE *fh = fopen( _kurl->path(), "rb" );
    if ( fh == 0L )
	return false;
    
    char buffer[ 1024 ];
    buffer[0] = 0;
    fgets( buffer, 1023, fh );
    fclose( fh );
    
    if ( strstr( buffer, "[KDE Desktop Entry]" ) == 0L )
	return false; */

    QString path = _kurl->path();
    KURL::decodeURL( path );
    QFile f( path );

    struct stat buff;
    stat( path, &buff );

    struct stat lbuff;
    lstat( path, &lbuff );

    if ( !S_ISREG( buff.st_mode ) || S_ISDIR( lbuff.st_mode ) )
	return false;
    if ( !f.open( IO_ReadOnly ) )
	return false;
    
    QTextStream pstream( &f );
    KConfig config( &pstream );
    config.setGroup( "KDE Desktop Entry" );

    QString type = config.readEntry( "Type" );
    if ( type.isNull() )
	return false;
    if ( type != "MimeType" )
	return false;
    
    return true;
}

void BindingPropsPage::applyChanges()
{
    QString path = properties->getKURL()->path();
    KURL::decodeURL( path );
    QFile f( path );
    if ( !f.open( IO_ReadWrite ) )
    {
	QMessageBox::message(  klocale->translate("KFM Error"),
			        klocale->translate("Could not save properties\nPerhaps permissions denied") );
	return;
    }
    
    QTextStream pstream( &f );
    KConfig config( &pstream );
    config.setGroup( "KDE Desktop Entry" );
    
    QString tmp = patternEdit->text();
    if ( tmp.length() > 1 )
	if ( tmp.right(1) != ";" )
	    tmp += ";";
    config.writeEntry( "Patterns", tmp.data() );
    config.writeEntry( "Comment", commentEdit->text() );
    config.writeEntry( "MimeType", mimeEdit->text() );
    
    if ( iconBox->currentItem() != -1 )
	config.writeEntry( "DefaultApp", appBox->text( appBox->currentItem() ) );
    
    int i = iconBox->currentItem();
    if ( i != -1 )
	config.writeEntry( "Icon", iconBox->text( i )  );

    config.sync();
    f.close();

    KMimeType::clearAll();
    KMimeType::init();
    KRootWidget::getKRootWidget()->update();

    KfmGui *win;
    for ( win = KfmGui::getWindowList().first(); win != 0L; win = KfmGui::getWindowList().next() )
	win->updateView();
}

void BindingPropsPage::slotIconChanged( int )
{
    drawIcon();
}

void BindingPropsPage::paintEvent( QPaintEvent *_ev )
{
    QWidget::paintEvent( _ev );
    drawIcon();
}

void BindingPropsPage::drawIcon()
{
    int i = iconBox->currentItem();
    if ( i == -1 )
	return;
    
    const char *text = iconBox->text( i );
    QString file = KMimeType::getIconPath();
    file += "/";
    file += text;
    
    if ( file != pixmapFile )
    {
	pixmapFile = file.data();
	pixmapFile.detach();	
	pixmap.load( file.data() );
    }
    
    erase( 310, 240, 64, 64 );
    QPainter painter;
    painter.begin( this );
    painter.setClipRect( 310, 240, 64, 64 );
    painter.drawPixmap( QPoint( 310, 240 ), pixmap );
    painter.end();
}


/* ----------------------------------------------------
 *
 * DevicePropsPage
 *
 * -------------------------------------------------- */

DevicePropsPage::DevicePropsPage( Properties *_props ) : PropsPage( _props )
{
    QLabel* tmpQLabel;
    tmpQLabel = new QLabel( this, "Label_1" );
    tmpQLabel->setGeometry( 10, 10, 140, 30 );
    tmpQLabel->setText(  klocale->translate("Device ( /dev/fd0 )") );
    
    device = new QLineEdit( this, "LineEdit_1" );
    device->setGeometry( 10, 40, 180, 30 );
    device->setText( "" );
    
    tmpQLabel = new QLabel( this, "Label_2" );
    tmpQLabel->setGeometry( 10, 80, 170, 30 );
    tmpQLabel->setText(  klocale->translate("Mount Point ( /floppy )") );
    
    mountpoint = new QLineEdit( this, "LineEdit_2" );
    mountpoint->setGeometry( 10, 110, 180, 30 );
    mountpoint->setText( "" );
    
    readonly = new QCheckBox( this, "CheckBox_1" );
    readonly->setGeometry( 220, 40, 100, 30 );
    readonly->setText(  klocale->translate("Readonly") );
    
    tmpQLabel = new QLabel( this, "Label_4" );
    tmpQLabel->setGeometry( 10, 150, 300, 30 );
    tmpQLabel->setText(  klocale->translate("Filesystems ( iso9660,msdos,minix,default )") );
    
    fstype = new QLineEdit( this, "LineEdit_3" );
    fstype->setGeometry( 10, 180, 280, 30 );
    fstype->setText( "" );
    
    tmpQLabel = new QLabel( this, "Label_5" );
    tmpQLabel->setGeometry( 10, 220, 100, 30 );
    tmpQLabel->setText(  klocale->translate("Mounted Icon") );
    
    tmpQLabel = new QLabel( this, "Label_6" );
    tmpQLabel->setGeometry( 170, 220, 100, 30 );
    tmpQLabel->setText(  klocale->translate("Unmounted Icon") );
    
    mounted = new QComboBox( false, this, "ComboBox_1" );
    mounted->setGeometry( 10, 250, 150, 30 );
    mounted->setSizeLimit( 10 );
    
    unmounted = new QComboBox( false, this, "ComboBox_2" );
    unmounted->setGeometry( 170, 250, 150, 30 );
    unmounted->setSizeLimit( 10 );
    
    QFile f( _props->getKURL()->path() );
    if ( !f.open( IO_ReadOnly ) )
	return;
    
    QTextStream pstream( &f );
    KConfig config( &pstream );
    config.setGroup( "KDE Desktop Entry" );
    deviceStr = config.readEntry( "Dev" );
    mountPointStr = config.readEntry( "MountPoint" );
    readonlyStr = config.readEntry( "ReadOnly" );
    fstypeStr = config.readEntry( "FSType" );
    mountedStr = config.readEntry( "Icon" );
    unmountedStr = config.readEntry( "UnmountIcon" );

    if ( !deviceStr.isNull() )
	device->setText( deviceStr.data() );
    if ( !mountPointStr.isNull() )
	mountpoint->setText( mountPointStr.data() );
    if ( !fstypeStr.isNull() )
	fstype->setText( fstypeStr.data() );
    if ( readonlyStr == "0" )
	readonly->setChecked( false );
    else
	readonly->setChecked( true );

    // Load all pixmaps files in the combobox
    QDir d( KMimeType::getIconPath() );
    const QFileInfoList *list = d.entryInfoList();
    QFileInfoListIterator it( *list );      // create list iterator
    QFileInfo *fi;                          // pointer for traversing

    int index1 = -1;
    int index2 = -1;
    int i = 0;  
    while ( ( fi = it.current() ) )
    {
	// Is this the currently selected icon ?
	if ( strcmp( mountedStr.data(), fi->fileName().data() ) == 0 )
	    index1 = i;
	// Is this the currently selected unmounted icon ?
	if ( strcmp( unmountedStr.data(), fi->fileName().data() ) == 0 )
	    index2 = i;
	mounted->insertItem( fi->fileName().data(), i );
	unmounted->insertItem( fi->fileName().data(), i );
	i++;
	++it;                               // goto next list element
    }
    // The currently selected icon is not in the list .... strange ... ? Lets add it.
    if ( index1 == -1 && mountedStr.length() > 0 )
    {
	mounted->insertItem( mountedStr.data(), i );
	index1 = i;
    }
    // The currently selected icon is not in the list .... strange ... ? Lets add it.
    if ( index2 == -1 && unmountedStr.length() > 0 )
    {
	unmounted->insertItem( unmountedStr.data(), i );
	index2 = i;
    }
    // Select the current icon
    mounted->setCurrentItem( index1 );
    unmounted->setCurrentItem( index2 );
    drawIcon2();
    drawIcon1();
    
    connect( mounted, SIGNAL( activated( int ) ), this, SLOT( slotIcon1Changed( int ) ) );
    connect( unmounted, SIGNAL( activated( int ) ), this, SLOT( slotIcon2Changed( int ) ) );
}

bool DevicePropsPage::supports( KURL *_kurl )
{
    KURL u( _kurl->url() );
    KURL u2( u.nestedURL() );
    
    if ( strcmp( u2.protocol(), "file" ) != 0 )
	return false;

    /* FILE *fh = fopen( _kurl->path(), "rb" );
    if ( fh == 0L )
	return false;
    
    char buffer[ 1024 ];
    buffer[0] = 0;
    fgets( buffer, 1023, fh );
    fclose( fh );
    
    if ( strstr( buffer, "[KDE Desktop Entry]" ) == 0L )
	return false; */

    QString path = _kurl->path();
    KURL::decodeURL( path );
    QFile f( path );

    struct stat buff;
    stat( path, &buff );

    struct stat lbuff;
    lstat( path, &lbuff );

    if ( !S_ISREG( buff.st_mode ) || S_ISDIR( lbuff.st_mode ) )
	return false;
    if ( !f.open( IO_ReadOnly ) )
	return false;
    
    QTextStream pstream( &f );
    KConfig config( &pstream );
    config.setGroup( "KDE Desktop Entry" );

    QString type = config.readEntry( "Type" );
    if ( type.isNull() )
	return false;
    if ( type != "FSDevice" )
	return false;
    
    return true;
}

void DevicePropsPage::applyChanges()
{
    QString path = properties->getKURL()->path();
    KURL::decodeURL( path );
    QFile f( path );
    if ( !f.open( IO_ReadWrite ) )
    {
	QMessageBox::message(  klocale->translate("KFM Error"), 
			        klocale->translate("Could not save properties\nPerhaps permissions denied") );
	return;
    }
    
    QTextStream pstream( &f );
    KConfig config( &pstream );
    config.setGroup( "KDE Desktop Entry" );
    
    config.writeEntry( "Dev", device->text() );
    config.writeEntry( "MountPoint", mountpoint->text() );
    config.writeEntry( "FSType", fstype->text() );
    
    if ( mounted->currentItem() != -1 )
	config.writeEntry( "Icon", mounted->text( mounted->currentItem() ) );
    if ( unmounted->currentItem() != -1 )
	config.writeEntry( "UnmountIcon", unmounted->text( unmounted->currentItem() ) );
    
    if ( readonly->isChecked() )
	config.writeEntry( "ReadOnly", "1" );
    else
	config.writeEntry( "ReadOnly", "0" );

    config.sync();
    f.close();
}

void DevicePropsPage::slotIcon1Changed( int )
{
    drawIcon1();
}

void DevicePropsPage::slotIcon2Changed( int )
{
    drawIcon2();
}

void DevicePropsPage::paintEvent( QPaintEvent *_ev )
{
    QWidget::paintEvent( _ev );
    drawIcon1();
    drawIcon2();
}

void DevicePropsPage::drawIcon1()
{
    int i = mounted->currentItem();
    if ( i == -1 )
	return;
    
    const char *text = mounted->text( i );
    QString file = KMimeType::getIconPath();
    file += "/";
    file += text;
    
    if ( file != pixmapFile )
    {
	pixmapFile = file.data();
	pixmapFile.detach();	
	pixmap.load( file.data() );
    }
    
    erase( 10, 290, 64, 64 );
    QPainter painter;
    painter.begin( this );
    painter.setClipRect( 10, 290, 64, 64 );
    painter.drawPixmap( QPoint( 10, 290 ), pixmap );
    painter.end();
}

void DevicePropsPage::drawIcon2()
{
    int i = unmounted->currentItem();
    if ( i == -1 )
	return;
    
    const char *text = unmounted->text( i );
    QString file = KMimeType::getIconPath();
    file += "/";
    file += text;
    
    if ( file != pixmapFile )
    {
	pixmapFile = file.data();
	pixmapFile.detach();	
	pixmap.load( file.data() );
    }
    
    erase( 170, 290, 64, 64 );
    QPainter painter;
    painter.begin( this );
    painter.setClipRect( 170, 290, 64, 64 );
    painter.drawPixmap( QPoint( 170, 290 ), pixmap );
    painter.end();
}

#include "kfmprops.moc"



