#include <kapp.h>
#include <qmsgbox.h>

#include <klocale.h>

int main( int argc, char **argv )
{
    if ( argc != 2 )
	return 0;
    
    int i = 1;
    KApplication a( i, argv, "kfm" );
    
    QMessageBox::message( klocale->translate("KFM Warning"), argv[1] );
}

