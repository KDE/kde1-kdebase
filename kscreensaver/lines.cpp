//-----------------------------------------------------------------------------
//
// klines 0.1.1 - Basic screen saver for KDE
// by Dirk Staneker 1997
// based on kpolygon from Martin R. Jones 1996
// mailto:dirk.staneker@student.uni-tuebingen.de
//
// layout management added 1998/04/19 by Mario Weilguni <mweilguni@kde.org>


#include <stdlib.h>
#include <qcolor.h>
#include <qlabel.h>
#include <qpushbt.h>
#include <qscrbar.h>
#include <kapp.h>
#include <qmsgbox.h>
#include <time.h>

#include "kcolordlg.h"
#include "kslider.h"
#include "lines.h"
#include "lines.moc"

#include <qlayout.h>
#include <kbuttonbox.h>
#include "helpers.h"

// this refers to klock.po. If you want an extra dictionary, 
// create an extra KLocale instance here.
extern KLocale *glocale;

#define MAXLENGTH	256

static kLinesSaver *saver = NULL;

// Methods of the Lines-class
Lines::Lines(int x){
	uint i;
	numLn=x;
	offx1=12;
	offy1=16;
	offx2=9;
	offy2=10;
	start=new Ln;
	end=start;
	for(i=1; i<numLn; i++){
		end->next=new Ln;
		end=end->next;
	}
	end->next=start;
	akt=start;
}

Lines::~Lines(){
	uint i;
	for(i=0; i<numLn; i++){
		end=start->next;
		delete start;
		start=end;
	}
};

inline void Lines::reset(){	akt=start;	};
	
inline void Lines::getKoord(int& a, int& b, int& c, int& d){
	a=akt->x1; b=akt->y1;
	c=akt->x2; d=akt->y2;
	akt=akt->next;
};

inline void Lines::setKoord(const int& a, const int& b, const int& c, const int& d){
	akt->x1=a; akt->y1=b;
	akt->x2=c; akt->y2=d;
};

inline void Lines::next(void){ akt=akt->next; };

void Lines::turn(const int& w, const int& h){
	start->x1=end->x1+offx1;
	start->y1=end->y1+offy1;
	start->x2=end->x2+offx2;
	start->y2=end->y2+offy2;
	if(start->x1>=w) offx1=-8;
	if(start->x1<=0) offx1=7;
	if(start->y1>=h) offy1=-11;
	if(start->y1<=0) offy1=13;
	if(start->x2>=w) offx2=-17;
	if(start->x2<=0) offx2=15;
	if(start->y2>=h) offy2=-10;
	if(start->y2<=0) offy2=13;
	end->next=start;
	start=start->next;
	end=end->next;
};

//-----------------------------------------------------------------------------
// standard screen saver interface functions
//
void startScreenSaver(Drawable d){
	if(saver) return;
	saver=new kLinesSaver(d);
}

void stopScreenSaver(){
	if(saver) delete saver;
	saver=NULL; 
}

int setupScreenSaver(){
	kLinesSetup dlg;
	return dlg.exec();
}

const char *getScreenSaverName(){
	return glocale->translate("Lines");
}

void exposeScreenSaver( int x, int y, int width, int height )
{
        if ( saver )
        {
                saver->expose( x, y, width, height );
        }
} 

//-----------------------------------------------------------------------------
// dialog to setup screen saver parameters
//
kLinesSetup::kLinesSetup(QWidget *parent, const char *name):QDialog(parent, name, TRUE){
	saver=NULL;
	length=10;
	speed=50;

	readSettings();

	QString str;
	QLabel *label;
	QPushButton *button;
	KSlider *sb;

	setCaption(glocale->translate("Setup klines"));

	QVBoxLayout *tl = new QVBoxLayout(this, 10, 10);
	QHBoxLayout *tl1 = new QHBoxLayout;
	tl->addLayout(tl1);
	QVBoxLayout *tl11 = new QVBoxLayout(5);
	tl1->addLayout(tl11);

	label=new QLabel(glocale->translate("Length:"), this);
	min_size(label);
	tl11->addWidget(label);

	sb=new KSlider(KSlider::Horizontal, this);
	sb->setRange(1, MAXLENGTH+1);
	sb->setSteps(MAXLENGTH/4, MAXLENGTH/2);
	sb->setValue(length);
	connect(sb, SIGNAL(valueChanged(int)), SLOT(slotLength(int)));
	sb->setMinimumSize(90, 20);
	tl11->addWidget(sb);
	tl11->addSpacing(5);

	label=new QLabel(glocale->translate("Speed:"), this);
	min_size(label);
	tl11->addWidget(label);

	sb=new KSlider( KSlider::Horizontal, this );
	sb->setRange( 0, 100 );
	sb->setSteps( 25, 50 );
	sb->setValue( speed );
	connect( sb, SIGNAL( valueChanged( int ) ), SLOT( slotSpeed( int ) ) );
	sb->setMinimumSize(90, 20);
	tl11->addWidget(sb);
	tl11->addSpacing(5);

        label=new QLabel(glocale->translate("Beginning:"), this);
	min_size(label);
	tl11->addWidget(label);

        colorPush0=new QPushButton(this);
        QColorGroup colgrp0(black, colstart, colstart.light(), colstart.dark(),
			   colstart.dark(120), black, white);
        colorPush0->setPalette(QPalette(colgrp0, colgrp0, colgrp0));
        connect(colorPush0, SIGNAL(clicked()), SLOT(slotColstart()));
	min_width(colorPush0);
	colorPush0->setFixedHeight(20);
	tl11->addWidget(colorPush0);
	tl11->addSpacing(5);

        label=new QLabel(glocale->translate("Middle:"), this);
	min_size(label);
	tl11->addWidget(label);

        colorPush1=new QPushButton(this);
        QColorGroup colgrp1(black, colmid, colmid.light(), colmid.dark(),
                           colmid.dark(120), black, white);
        colorPush1->setPalette(QPalette(colgrp1, colgrp1, colgrp1));
        connect(colorPush1, SIGNAL(clicked()), SLOT(slotColmid()));
	colorPush1->setFixedHeight(20);
	tl11->addWidget(colorPush1);
	tl11->addSpacing(5);

        label=new QLabel(glocale->translate("End:"), this);
	min_size(label);
	tl11->addWidget(label);

        colorPush2=new QPushButton(this);
        QColorGroup colgrp2(black, colend, colend.light(), colend.dark(),
                           colend.dark(120), black, white);
        colorPush2->setPalette(QPalette(colgrp2, colgrp2, colgrp2));
        connect(colorPush2, SIGNAL(clicked()), SLOT(slotColend()));
	colorPush2->setFixedHeight(20);
	tl11->addWidget(colorPush2);
	tl11->addStretch(1);

	preview = new QWidget( this );
	preview->setFixedSize( 220, 170 );
	preview->setBackgroundColor( black );
	preview->show();    // otherwise saver does not get correct size
	saver=new kLinesSaver(preview->winId());
	tl1->addWidget(preview);

	KButtonBox *bbox = new KButtonBox(this);	
	button = bbox->addButton( glocale->translate("About"));
	connect( button, SIGNAL( clicked() ), SLOT(slotAbout() ) );
	bbox->addStretch(1);

	button = bbox->addButton( glocale->translate("OK"));	
	connect( button, SIGNAL( clicked() ), SLOT( slotOkPressed() ) );

	button = bbox->addButton(glocale->translate("Cancel"));
	connect( button, SIGNAL( clicked() ), SLOT( reject() ) );
	bbox->layout();
	tl->addWidget(bbox);

	tl->freeze();
}

// read settings from config file
void kLinesSetup::readSettings(){
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Settings" );

	QString str;

	str = config->readEntry("Length");
	if(!str.isNull()) length=atoi(str);
	if(length>MAXLENGTH) length=MAXLENGTH;
	else if(length<1) length=1;

	str=config->readEntry("Speed");
	if(!str.isNull()) speed=atoi(str);
	if(speed>100) speed=100;
	else if(speed<50) speed=50;

        str=config->readEntry("StartColor");
        if(!str.isNull()) colstart.setNamedColor(str);
        else colstart=white;
        str=config->readEntry("MidColor");
        if(!str.isNull()) colmid.setNamedColor(str);
        else colmid=blue;
        str=config->readEntry("EndColor");
        if(!str.isNull()) colend.setNamedColor(str);
        else colend=black;
}

void kLinesSetup::slotLength(int len){
	length=len;
	if(saver) saver->setLines(length);
}

void kLinesSetup::slotSpeed(int num){
	speed=num;
	if(saver) saver->setSpeed(speed);
}

void kLinesSetup::slotColstart(){
        if(KColorDialog::getColor(colstart)==QDialog::Rejected) return;
        QColorGroup colgrp(black, colstart, colstart.light(), colstart.dark(),
                           colstart.dark(120), black, white);
        colorPush0->setPalette(QPalette(colgrp, colgrp, colgrp));
        if(saver) saver->setColor(colstart, colmid, colend);
}

void kLinesSetup::slotColmid(){
        if(KColorDialog::getColor(colmid)==QDialog::Rejected) return;
        QColorGroup colgrp(black, colmid, colmid.light(), colmid.dark(),
                           colmid.dark(120), black, white);
        colorPush1->setPalette(QPalette(colgrp, colgrp, colgrp));
        if(saver) saver->setColor(colstart, colmid, colend);
}

void kLinesSetup::slotColend(){
        if(KColorDialog::getColor(colend)==QDialog::Rejected) return;
        QColorGroup colgrp(black, colend, colend.light(), colend.dark(),
                           colend.dark(120), black, white);
        colorPush2->setPalette(QPalette(colgrp, colgrp, colgrp));
        if(saver) saver->setColor(colstart, colmid, colend);
}

void kLinesSetup::slotAbout(){
	QMessageBox::message(glocale->translate("About Lines"),
			     glocale->translate("Lines Version 0.1.1\n\nwritten by Dirk Staneker 1997\ndirk.stanerker@student.uni-tuebingen.de"),
			     glocale->translate("OK"));
}

// Ok pressed - save settings and exit
void kLinesSetup::slotOkPressed(){
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup("Settings");

	QString slength;
	slength.setNum(length);
	config->writeEntry("Length", slength);

	QString sspeed;
	sspeed.setNum( speed );
	config->writeEntry( "Speed", sspeed );

        QString colName0(10), colName1(10), colName2(10);
        colName0.sprintf("#%02x%02x%02x", colstart.red(),
		         colstart.green(), colstart.blue() );
        config->writeEntry( "StartColor", colName0 );

        colName1.sprintf("#%02x%02x%02x", colmid.red(),
                         colmid.green(), colmid.blue() );
        config->writeEntry( "MidColor", colName1 );

        colName2.sprintf("#%02x%02x%02x", colend.red(),
                         colend.green(), colend.blue() );
        config->writeEntry( "EndColor", colName2 );

	config->sync();
	accept();
}

//-----------------------------------------------------------------------------


kLinesSaver::kLinesSaver(Drawable drawable):kScreenSaver(drawable){
	readSettings();
	lines=new Lines(numLines);
	srandom((int)time((time_t *)NULL));
	colorContext=QColor::enterAllocContext();
	blank();
	initialiseColor();
	initialiseLines();
	timer.start(speed);
	connect(&timer, SIGNAL(timeout()), SLOT(slotTimeout()));
}

kLinesSaver::~kLinesSaver(){
	timer.stop();
	QColor::leaveAllocContext();
	QColor::destroyAllocContext(colorContext);
	if(lines) delete lines;
}

// set lines properties
void kLinesSaver::setLines(int len){
	timer.stop();
	numLines=len;
	initialiseLines();
	initialiseColor();
	blank();
	timer.start(speed);
}

// set the speed
void kLinesSaver::setSpeed(int spd){
	timer.stop();
	speed=100-spd;
	timer.start(speed);
}

void kLinesSaver::setColor(const QColor& cs, const QColor& cm, const QColor& ce){
	colstart=cs;
	colmid=cm;
	colend=ce;
        initialiseColor();
}

// read configuration settings from config file
void kLinesSaver::readSettings(){
	QString str;

	KConfig *config=KApplication::getKApplication()->getConfig();
	config->setGroup("Settings");

	str=config->readEntry("Length");
	if(!str.isNull()) numLines=atoi(str);
	else numLines=10;
	str = config->readEntry("Speed");
	if(!str.isNull()) speed=100-atoi(str);
	else speed=50;
	if(numLines>MAXLENGTH) numLines=MAXLENGTH;
	else if(numLines<1) numLines = 1;

        str=config->readEntry("StartColor");
        if(!str.isNull()) colstart.setNamedColor(str);
        else colstart=white;
        str=config->readEntry("MidColor");
        if(!str.isNull()) colmid.setNamedColor(str);
        else colmid=blue;
        str=config->readEntry("EndColor");
        if(!str.isNull()) colend.setNamedColor(str);
        else colend=black;
}

// draw next lines and erase tail
void kLinesSaver::slotTimeout(){
	uint i;
	int x1,y1,x2,y2;
	int col=0;

	lines->reset();
	XSetForeground(qt_xdisplay(), gc, 0);
	for(i=0; i<numLines; i++){
		lines->getKoord(x1,y1,x2,y2);
                XDrawLine(qt_xdisplay(), d, gc, x1, y1, x2, y2);
		XSetForeground(qt_xdisplay(), gc, colors[col]);
		col=(int)(i*colscale);
		if(col>63) col=0;
	}
	lines->turn(width, height);
}

void kLinesSaver::blank(){
	XSetWindowBackground(qt_xdisplay(), d, black.pixel());
	XClearWindow(qt_xdisplay(), d);
}

// initialise the lines
void kLinesSaver::initialiseLines(){
	uint i;
	int x1,y1,x2,y2;
	if(lines) delete lines;
	lines=new Lines(numLines);
	lines->reset();
	x1=random()%width;
	y1=random()%height;
	x2=random()%width;
	y2=random()%height;
	for(i=0; i<numLines; i++){
		lines->setKoord(x1,y1,x2,y2);
		lines->next();
	}
}

// create a color table of 64 colors
void kLinesSaver::initialiseColor(){
	int i;
	double mr, mg, mb;
	double cr, cg, cb;
	QColor color;
        mr=(double)(colmid.red()-colstart.red())/32;
        mg=(double)(colmid.green()-colstart.green())/32;
        mb=(double)(colmid.blue()-colstart.blue())/32;
        cr=colstart.red();
        cg=colstart.green();
        cb=colstart.blue();
	for(i=0; i<32; i++){
		color.setRgb((int)(mr*i+cr), (int)(mg*i+cg), (int)(mb*i+cb));
		colors[63-i] = color.alloc();
	}
	mr=(double)(colend.red()-colmid.red())/32;
	mg=(double)(colend.green()-colmid.green())/32;
	mb=(double)(colend.blue()-colmid.blue())/32;
	cr=colmid.red();
	cg=colmid.green();
	cb=colmid.blue();
	for(i=0; i<32; i++){
		color.setRgb((int)(mr*i+cr), (int)(mg*i+cg), (int)(mb*i+cb));
		colors[31-i] = color.alloc();
	}
	colscale=64.0/(double)numLines;
}
