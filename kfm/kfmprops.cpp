#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <kconfig.h>

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

    // Extract the directories name without path
    QString filename;
    QString tmp2 = properties->getKURL()->path();
    if ( tmp2.right(1) == "/" )
	tmp2.truncate( tmp2.length() - 1 );
    int i = tmp2.findRev( "/" );
    // Is is not the root directory ?
    if ( i != -1 )
	filename = tmp2.mid( i + 1, tmp2.length() );
    else
	filename = "/";
    
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
    // Dont rename trash or root directory
    if ( isTrash || filename == "/" )
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
    l = new QLabel( klocale->translate("Read"), this );
    l->setGeometry( 90, y, 80, 20 );
    l = new QLabel( klocale->translate("Write"), this );
    l->setGeometry( 170, y, 80, 20 );
    l = new QLabel( klocale->translate("Exec"), this );
    l->setGeometry( 250, y, 80, 20 );
    y += 25;

    l = new QLabel( klocale->translate("User"), this );
    l->setGeometry( 10, y, 80, 20 );
    permUR = new QCheckBox( "", this );
    permUR->setGeometry( 90, y, 100, 30 );
    permUR->setChecked( ( buff.st_mode & S_IRUSR ) == S_IRUSR );
    permUW = new QCheckBox( "", this );
    permUW->setGeometry( 170, y, 100, 30 );
    permUW->setChecked( ( buff.st_mode & S_IWUSR ) == S_IWUSR );
    permUX = new QCheckBox( "", this );
    permUX->setGeometry( 250, y, 100, 30 );
    permUX->setChecked( ( buff.st_mode & S_IXUSR ) == S_IXUSR );
    permUS = new QCheckBox( klocale->translate("UID"), this );
    permUS->setGeometry( 310, y, 100, 30 );
    permUS->setChecked( ( buff.st_mode & S_ISUID ) == S_ISUID );      
    y += 35;

    l = new QLabel( klocale->translate("Group"), this );
    l->setGeometry( 10, y, 80, 20 );
    permGR = new QCheckBox( "", this );
    permGR->setGeometry( 90, y, 100, 30 );
    permGR->setChecked( ( buff.st_mode & S_IRGRP ) == S_IRGRP );
    permGW = new QCheckBox( "", this );
    permGW->setGeometry( 170, y, 100, 30 );
    permGW->setChecked( ( buff.st_mode & S_IWGRP ) == S_IWGRP );
    permGX = new QCheckBox( "", this );
    permGX->setGeometry( 250, y, 100, 30 );
    permGX->setChecked( ( buff.st_mode & S_IXGRP ) == S_IXGRP );
    permGS = new QCheckBox( klocale->translate("GID "), this );
    permGS->setGeometry( 310, y, 100, 30 );
    permGS->setChecked( ( buff.st_mode & S_ISGID ) == S_ISGID );   

    y += 35;
    l = new QLabel( klocale->translate("Others"), this );
    l->setGeometry( 10, y, 80, 20 );
    permOR = new QCheckBox( "", this );
    permOR->setGeometry( 90, y, 100, 30 );
    permOR->setChecked( ( buff.st_mode & S_IROTH ) == S_IROTH );
    permOW = new QCheckBox( "", this );
    permOW->setGeometry( 170, y, 100, 30 );
    permOW->setChecked( ( buff.st_mode & S_IWOTH ) == S_IWOTH );
    permOX = new QCheckBox( "", this );
    permOX->setGeometry( 250, y, 100, 30 );
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
	if ( chmod( path, p ) != 0 )
	    QMessageBox::message( klocale->translate( "KFM Error" ),
				  klocale->translate( "Could not change permissions\nPerhaps access denied." ) );	    
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
	if ( chown( path, pw->pw_uid, g->gr_gid ) != 0 )
	    QMessageBox::message( klocale->translate( "KFM Error" ),
				  klocale->translate( "Could not change owner/group\nPerhaps access denied." ) );
    }    
}

ExecPropsPage::ExecPropsPage( Properties *_props ) : PropsPage( _props )
{
    execEdit = new QLineEdit( this, "LineEdit_1" );
    pathEdit = new QLineEdit( this, "LineEdit_2" );
    iconBox = new KIconLoaderButton( this );
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

    // tmpQLabel = new QLabel( this, "Label_2" );
    // tmpQLabel->setGeometry( 10, 130, 100, 30 );
    // tmpQLabel->setText( klocale->translate("Icon") );

    tmpQLabel = new QLabel( this, "Label_3" );
    tmpQLabel->setGeometry( 10, 70, 120, 30 );
    tmpQLabel->setText( klocale->translate("Working Directory") );

    pathEdit->raise();
    pathEdit->setGeometry( 10, 100, 210, 30 );
    pathEdit->setMaxLength( 256 );
    
    iconBox->raise();
    iconBox->setGeometry( 10, 140, 50, 50 );

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
    f.close();

    KConfig config( _props->getKURL()->path() );
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
	iconStr = KMimeType::getDefaultPixmap();
    
    iconBox->setIcon( iconStr ); 

    QStrList list;
    QString tmp( kapp->kdedir() );
    tmp += "/share/icons";
    list.append( tmp.data() );
    tmp = getenv( "HOME" );
    tmp += "/.kde/share/icons";
    list.append( tmp.data() );
    iconBox->iconLoaderDialog().setDir( &list );
    
    connect( execBrowse, SIGNAL( pressed() ), this, SLOT( slotBrowseExec() ) );
}

bool ExecPropsPage::supports( KURL *_kurl )
{
    KURL u( _kurl->url() );
    KURL u2( u.nestedURL() );
    
    if ( strcmp( u2.protocol(), "file" ) != 0 )
	return false;

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
    f.close();

    KConfig config( t );
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

    f.close();

    KConfig config( path );
    config.setGroup( "KDE Desktop Entry" );
    config.writeEntry( "Exec", execEdit->text() );
    config.writeEntry( "Path", pathEdit->text() );
    config.writeEntry( "Icon", iconBox->icon() );

    if ( terminalCheck->isChecked() )
	config.writeEntry( "Terminal", "1" );
    else
	config.writeEntry( "Terminal", "0" );

    config.writeEntry( "TerminalOptions", terminalEdit->text() );
    config.sync();
}


void ExecPropsPage::slotBrowseExec()
{
    QString f = QFileDialog::getOpenFileName( 0, 0L, this );
    if ( f.isNull() )
	return;

    execEdit->setText( f.data() );
}

URLPropsPage::URLPropsPage( Properties *_props ) : PropsPage( _props )
{
    URLEdit = new QLineEdit( this, "LineEdit_1" );
    iconBox = new KIconLoaderButton( this );

    URLEdit->raise();
    URLEdit->setGeometry( 10, 40, 210, 30 );
    URLEdit->setText( "" );
    URLEdit->setMaxLength( 256 );

    QLabel* tmpQLabel;
    tmpQLabel = new QLabel( this, "Label_1" );
    tmpQLabel->setGeometry( 10, 10, 100, 30 );
    tmpQLabel->setText( klocale->translate("URL") );
    
    iconBox->raise();
    iconBox->setGeometry( 10, 90, 50, 50 );

    QString path = _props->getKURL()->path();
    KURL::decodeURL( path );

    QFile f( path );
    if ( !f.open( IO_ReadOnly ) )
	return;
    f.close();

    KConfig config( path );
    config.setGroup( "KDE Desktop Entry" );
    URLStr = config.readEntry(  "URL" );
    iconStr = config.readEntry( "Icon" );

    if ( !URLStr.isNull() )
	URLEdit->setText( URLStr.data() );
    if ( iconStr.isNull() )
	iconStr = KMimeType::getDefaultPixmap();

    QStrList list;
    QString tmp( kapp->kdedir() );
    tmp += "/share/icons";
    list.append( tmp.data() );
    tmp = getenv( "HOME" );
    tmp += "/.kde/share/icons";
    list.append( tmp.data() );
    iconBox->iconLoaderDialog().setDir( &list );

    iconBox->setIcon( iconStr );
}

bool URLPropsPage::supports( KURL *_kurl )
{
    KURL u( _kurl->url() );
    KURL u2( u.nestedURL() );
    
    if ( strcmp( u2.protocol(), "file" ) != 0 )
	return false;

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
    f.close();

    KConfig config( path );
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
    f.close();

    KConfig config( path );
    config.setGroup( "KDE Desktop Entry" );
    config.writeEntry( "URL", URLEdit->text() );
    config.writeEntry( "Icon", iconBox->icon() );
    config.sync();
}

DirPropsPage::DirPropsPage( Properties *_props ) : PropsPage( _props )
{
    iconBox = new KIconLoaderButton( this );
    iconBox->raise();
    iconBox->setGeometry( 10, 20, 50, 50 );

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
	f.close();

	KConfig config( tmp );
	config.setGroup( "KDE Desktop Entry" );
	wallStr = config.readEntry( "BgImage" );
	iconStr = config.readEntry( "Icon" );
    }

    if ( iconStr.isEmpty() )
    {
	QString str( KMimeType::getMagicMimeType( properties->getKURL()->url() )->getPixmapFile() );
	KURL u( str );
	iconStr = u.filename();
    }
    
    QStrList list2;
    tmp = kapp->kdedir();
    tmp += "/share/icons";
    list2.append( tmp.data() );
    tmp = getenv( "HOME" );
    tmp += "/.kde/share/icons";
    list2.append( tmp.data() );
    iconBox->iconLoaderDialog().setDir( &list2 );

    iconBox->setIcon( iconStr );

    // Load all wallpapers in the combobox
    tmp = kapp->kdedir();
    tmp += "/share/wallpapers";
    QDir d2( tmp.data() );
    const QFileInfoList *list = d2.entryInfoList();  
    QFileInfoListIterator it2( *list );      // create list iterator
    QFileInfo *fi;                          // pointer for traversing  

    wallBox->insertItem(  klocale->translate("(Default)"), 0 );
    
    int index = 0;
    int i = 1;  
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
    // Select the current icon
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
    f.close();

    KConfig config( tmp );
    config.setGroup( "KDE Desktop Entry" );

    int i = wallBox->currentItem();
    if ( i != -1 )
    {
	if ( strcmp( wallBox->text( i ),  klocale->translate("(Default)") ) == 0 )
	    config.writeEntry( "BgImage", "" );
	else
	    config.writeEntry( "BgImage", wallBox->text( i ) );
    }

    // Get the default image
    QString str( KMimeType::getMagicMimeType( properties->getKURL()->url() )->getPixmapFile() );
    KURL u( str );
    QString str2 = u.filename();
    // Is it still the default ?
    if ( str2 == iconBox->icon() )
	config.writeEntry( "Icon", "" );
    else
	config.writeEntry( "Icon", iconBox->icon()  );
    
    config.sync();

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

    KIOServer::sendNotify( tmp.data() );
}

void DirPropsPage::slotWallPaperChanged( int )
{
    drawWallPaper();
}

void DirPropsPage::paintEvent( QPaintEvent *_ev )
{
    QWidget::paintEvent( _ev );
    drawWallPaper();
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
    if ( strcmp( text, klocale->translate( "(Default)" ) ) == 0 )
    {
	erase( 140, 90, 128, 128 );
	return;
    }

    QString file = kapp->kdedir();
    file += "/share/wallpapers/";
    file += text;
    
    if ( file != wallFile )
    {
	// debugT("Loading WallPaper '%s'\n",file.data());
	wallFile = file.data();
	wallFile.detach();	
	wallPixmap.load( file.data() );
    }
    
    if ( wallPixmap.isNull() )
	warning("Could not load wallpaper %s\n",file.data());
    
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
    nameEdit = new QLineEdit( this, "LineEdit_3" );

    extensionsList = new QListBox( this );
    availableExtensionsList = new QListBox( this );
    addExtensionButton = new QPushButton( "<-", this );
    delExtensionButton = new QPushButton( "->", this );

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

    tmpQLabel = new QLabel( this, "Label_4" );
    tmpQLabel->setGeometry( 10, 130, 300, 30 );
    tmpQLabel->setText(  klocale->translate("Name ( in your language )") );

    commentEdit->raise();
    commentEdit->setGeometry( 10, 100, 210, 30 );
    commentEdit->setMaxLength( 256 );

    nameEdit->raise();
    nameEdit->setGeometry( 10, 160, 210, 30 );
    nameEdit->setMaxLength( 256 );
    
    extensionsList->setGeometry( 10, 200, 130, 120 );
    availableExtensionsList->setGeometry( 230, 200, 130, 120 );
    addExtensionButton->setGeometry( 160, 220, 40, 40 );
    connect( addExtensionButton, SIGNAL( pressed() ), this, SLOT( slotAddExtension() ) );
    delExtensionButton->setGeometry( 160, 260, 40, 40 );    
    connect( delExtensionButton, SIGNAL( pressed() ), this, SLOT( slotDelExtension() ) );

    QString path = _props->getKURL()->path() ;
    KURL::decodeURL( path );	    
    QFile f( path );
    if ( !f.open( IO_ReadOnly ) )
	return;
    f.close();

    KConfig config( path );
    config.setGroup( "KDE Desktop Entry" );
    commentStr = config.readEntry( "Comment" );
    binaryPatternStr = config.readEntry( "BinaryPattern" );
    extensionsStr = config.readEntry( "MimeType" );
    nameStr = config.readEntry( "Name" );
    // Use the file name if no name is specified
    if ( nameStr.isEmpty() )
    {
	nameStr = _props->getKURL()->filename();
	if ( nameStr.right(7) == ".kdelnk" )
	    nameStr.truncate( nameStr.length() - 7 );
	KURL::decodeURL( nameStr );
    }
    
    if ( !commentStr.isNull() )
	commentEdit->setText( commentStr.data() );
    if ( !nameStr.isNull() )
	nameEdit->setText( nameStr.data() );
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
    f.close(); 

    KConfig config( path );
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
    
	f.close(); // kalle
	// kalle    QTextStream pstream( &f );
    KConfig config( path ); // kalle
    config.setGroup( "KDE Desktop Entry" );
    config.writeEntry( "Comment", commentEdit->text(), true, false, true );

    QString tmp = binaryPatternEdit->text();
    if ( tmp.length() > 0 )
	if ( tmp.right(1) != ";" )
	    tmp += ";";
    config.writeEntry( "BinaryPattern", tmp.data() );

    extensionsStr = "";
    for ( uint i = 0; i < extensionsList->count(); i++ )
    {
	extensionsStr += extensionsList->text( i );
	extensionsStr += ";";
    }
    config.writeEntry( "MimeType", extensionsStr.data() );
    config.writeEntry( "Name", nameEdit->text(), true, false, true );
    
    config.sync();
    f.close();

    KMimeType::clearAll();
    KMimeType::init();
    if ( KRootWidget::getKRootWidget() )
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
    iconBox = new KIconLoaderButton( this );
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

    // tmpQLabel = new QLabel( this, "Label_2" );
    // tmpQLabel->setGeometry( 180, 210, 100, 30 );
    // tmpQLabel->setText(  klocale->translate("Icon") );

    tmpQLabel = new QLabel( this, "Label_2" );
    tmpQLabel->setGeometry( 10, 130, 100, 30 );
    tmpQLabel->setText(  klocale->translate("Mime Type") );

    tmpQLabel = new QLabel( this, "Label_3" );
    tmpQLabel->setGeometry( 10, 70, 120, 30 );
    tmpQLabel->setText(  klocale->translate("Comment") );
    
    iconBox->raise();
    iconBox->setGeometry( 180, 210, 50, 50 );

    tmpQLabel = new QLabel( this, "Label_2" );
    tmpQLabel->setGeometry( 10, 210, 170, 30 );
    tmpQLabel->setText(  klocale->translate("Default Application") );

    appBox->raise();
    appBox->setGeometry( 10, 240, 120, 30 );
    
    QFile f( _props->getKURL()->path() );
    if ( !f.open( IO_ReadOnly ) )
	return;
    f.close();

    KConfig config( _props->getKURL()->path() );
    config.setGroup( "KDE Desktop Entry" );
    patternStr = config.readEntry( "Patterns" );
    appStr = config.readEntry( "DefaultApp" );
    iconStr = config.readEntry( "Icon" );
    commentStr = config.readEntry( "Comment" );
    mimeStr = config.readEntry( "MimeType" );

    if ( !patternStr.isEmpty() )
	patternEdit->setText( patternStr.data() );
    if ( !commentStr.isEmpty() )
	commentEdit->setText( commentStr.data() );
    if ( iconStr.isEmpty() )
	iconStr = KMimeType::getDefaultPixmap();
    if ( !mimeStr.isEmpty() )
	mimeEdit->setText( mimeStr.data() );
    
    QStrList list;
    QString tmp( kapp->kdedir() );
    tmp += "/share/icons";
    list.append( tmp.data() );
    tmp = getenv( "HOME" );
    tmp += "/.kde/share/icons";
    list.append( tmp.data() );
    iconBox->iconLoaderDialog().setDir( &list );

    iconBox->setIcon( iconStr );
    
    // Get list of all applications
    int index = -1;
    int i = 0;
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
    
	f.close(); // kalle
	// kalle    QTextStream pstream( &f );
    KConfig config( path ); // kalle
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
    f.close();

    KConfig config( path );
    config.setGroup( "KDE Desktop Entry" );
    
    QString tmp = patternEdit->text();
    if ( tmp.length() > 1 )
	if ( tmp.right(1) != ";" )
	    tmp += ";";
    config.writeEntry( "Patterns", tmp.data() );
    config.writeEntry( "Comment", commentEdit->text() );
    config.writeEntry( "MimeType", mimeEdit->text() );
    config.writeEntry( "Icon", iconBox->icon() );

    if ( appBox->currentItem() != -1 )
	config.writeEntry( "DefaultApp", appBox->text( appBox->currentItem() ) );

    config.sync();

    KMimeType::clearAll();
    KMimeType::init();
    if ( KRootWidget::getKRootWidget() )
	KRootWidget::getKRootWidget()->update();

    KfmGui *win;
    for ( win = KfmGui::getWindowList().first(); win != 0L; win = KfmGui::getWindowList().next() )
	win->updateView();
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
    readonly->setGeometry( 220, 40, 130, 30 );
    readonly->setText(  klocale->translate("Readonly") );
    
    tmpQLabel = new QLabel( this, "Label_4" );
    tmpQLabel->setGeometry( 10, 150, 300, 30 );
    tmpQLabel->setText(  klocale->translate("Filesystems ( iso9660,msdos,minix,default )") );
    
    fstype = new QLineEdit( this, "LineEdit_3" );
    fstype->setGeometry( 10, 180, 280, 30 );
    fstype->setText( "" );
    
    tmpQLabel = new QLabel( this, "Label_5" );
    tmpQLabel->setGeometry( 10, 220, 150, 30 );
    tmpQLabel->setText(  klocale->translate("Mounted Icon") );
    
    tmpQLabel = new QLabel( this, "Label_6" );
    tmpQLabel->setGeometry( 170, 220, 150, 30 );
    tmpQLabel->setText(  klocale->translate("Unmounted Icon") );
    
    mounted = new KIconLoaderButton( this );
    mounted->setGeometry( 10, 250, 50, 50 );
    
    unmounted = new KIconLoaderButton( this );
    unmounted->setGeometry( 170, 250, 50, 50 );

    QString path( _props->getKURL()->path() );
    KURL::decodeURL( path );
    
    QFile f( path );
    if ( !f.open( IO_ReadOnly ) )
	return;
    f.close();

    KConfig config( path );
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
    if ( mountedStr.isEmpty() )
	mountedStr = KMimeType::getDefaultPixmap();
    if ( unmountedStr.isEmpty() )
	unmountedStr = KMimeType::getDefaultPixmap();    

    mounted->setIcon( mountedStr ); 
    unmounted->setIcon( unmountedStr ); 

    QStrList list;
    QString tmp( kapp->kdedir() );
    tmp += "/share/icons";
    list.append( tmp.data() );
    tmp = getenv( "HOME" );
    tmp += "/.kde/share/icons";
    list.append( tmp.data() );
    mounted->iconLoaderDialog().setDir( &list );
    unmounted->iconLoaderDialog().setDir( &list );
}

bool DevicePropsPage::supports( KURL *_kurl )
{
    KURL u( _kurl->url() );
    KURL u2( u.nestedURL() );
    
    if ( strcmp( u2.protocol(), "file" ) != 0 )
	return false;

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
    f.close();

    KConfig config( path );
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
    f.close();

    KConfig config( path );
    config.setGroup( "KDE Desktop Entry" );
    
    config.writeEntry( "Dev", device->text() );
    config.writeEntry( "MountPoint", mountpoint->text() );
    config.writeEntry( "FSType", fstype->text() );
    
    config.writeEntry( "Icon", mounted->icon() );
    config.writeEntry( "UnmountIcon", unmounted->icon() );
    
    if ( readonly->isChecked() )
	config.writeEntry( "ReadOnly", "1" );
    else
	config.writeEntry( "ReadOnly", "0" );

    config.sync();
}

#include "kfmprops.moc"



