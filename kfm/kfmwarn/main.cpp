#include <qapp.h>
#include <qmsgbox.h>

int main( int argc, char **argv )
{
    if ( argc != 2 )
	return 0;
    
    int i = 1;
    QApplication a( i, argv );
    
    QMessageBox::message( "KFM Warning", argv[1] );
}

