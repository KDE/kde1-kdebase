#include <qapp.h>
#include <qstring.h>
#include "kwm.h"

void main(int argc, char* argv[]){
  QApplication a(argc, argv);
  QString s1;
  QString s2;
  
  if (argc > 2){
    s1 = argv[1];
    s2 = argv[2];
    KWM::setSticky(s1.toLong(), s2.toInt());
  }
}
