/*
 * kwmcom.C. Part of the KDE project.
 *
 * Copyright (C) 1997 Matthias Ettrich
 *
 */

#include <qapp.h>
#include <qstring.h>
#include <kwm.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]){
  QApplication a(argc, argv);
  
  if (argc != 2){
    printf("Usage: ");
    printf("%s <command>\n", argv[0]);
    exit(1);
  }
  else {
    KWM::sendKWMCommand(argv[1]);
  }
  return 0;
}
