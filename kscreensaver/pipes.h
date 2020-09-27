#ifndef __PIPES_H__
#define __PIPES_H__

#if defined(HAVE_GL)

#include <qtimer.h>
#include <qlist.h>
#include <qdialog.h>
#include <qlined.h>
#include "saver.h"

// Pipes
// Copyright (c) 1998 by Lars Doelle <lars.doelle@on-line.de>
// GPL Version 2 applies.

#include <qgl.h>

// basic parameters

#define CUBESIZE  4 // cube radius   (> 0)
#define CELLSIZE  1 // cell radius   (> 0)

#define SPEED    16 // wait SPEED ms between steps

// non-parameters

#define MAXPIPES  6 // maximum pipes (> 0)
#define DEFPIPES  3 // default nr of pipes ( > 0 <= MAXPIPES)
#define DETAIL    6 // resolution (increase for more curved surfaces)

// derived parameters

#define SUBCELLS (2*(CELLSIZE)+2)
#define TOTALSIZE ((2*(CUBESIZE)+1)*(SUBCELLS))
#define FIXMETO1 5.4
//#define RADIUS 0.05
#define RADIUS (((float)(FIXMETO1))/(TOTALSIZE)/2)

class kPipesSaver;

class Pipe
{
public:
    Pipe(kPipesSaver* b, int c);
    bool chooseDir();
    void choosePos();
    kPipesSaver* box;
    int col;
    int x,y,z;
    int dir;
    bool running;
};

class kPipesSaver : public kScreenSaver
{
	Q_OBJECT

public:

//  PipeBox( QWidget* parent, const char* name );
    bool cube[2*CUBESIZE+1][2*CUBESIZE+1][2*CUBESIZE+1];

public slots:

    void    tick();
    void		paintGL();
    void    reinit();

    //FIXME: better make a proper Cube data type
    void    clearCube();
    bool    goodCubeLocation(int x, int y, int z);
    void    occupyCubeLocation(int x, int y, int z);

protected:

    void    makeStep();
    void    paintStep();

    void		resizeGL( int w, int h );

    virtual GLuint 	makeCoord();
    virtual GLuint 	makeStart();
    virtual GLuint 	makeArrow();

private:

    int pipes;
    int steps;
    int running;
    bool initial;

    Pipe*  pipe[MAXPIPES];

    GLuint coord;
    GLuint start;
    GLuint arrow;

    GLfloat xRot, yRot, zRot, scale;

public:
	kPipesSaver( Drawable drawable );
	virtual ~kPipesSaver();

	void setPipes( int n );

protected:
	void readSettings();

protected:
	int         colorContext;
  QTimer      timer;
};

class kPipesSetup : public QDialog
{
	Q_OBJECT
public:
	kPipesSetup( QWidget *parent = NULL, const char *name = NULL );

protected:
	void readSettings();

private slots:
	void slotPipes( int );
	void slotOkPressed();
	void slotAbout();

private:
	QWidget *preview;
	kPipesSaver *saver;
	int			pipes;
};

#endif

#endif

