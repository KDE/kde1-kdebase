#include "kmsgwin.moc"

KMsgWin::KMsgWin(QWidget *parent, const char *caption, const char *message, int type,
                 const char *b1text, const char *b2text, const char *b3text,
                 const char *b4text) : QWidget ( parent )

{
    int icon;
    
    static int icons_initialized = 0;
    static KPixmap icons[4];

    if( !icons_initialized )
    {
        QString ipath( getenv("KDEDIR") );
	if ( ipath.isNull() )
	{
	    printf("ERROR: You did not set $KDEDIR\n");
	    exit(2);
	}
	ipath.detach();
	ipath += "/lib/pics/";
        icons[0].load(ipath + "info.xpm");
        icons[1].load(ipath + "exclamation.xpm");
        icons[2].load(ipath + "stopsign.xpm");
        icons[3].load(ipath + "question.xpm");
        icons_initialized = 1;

	printf("ICON_PATH=%s\n",ipath.data());
    }

    int icon_index = type & 0x0000000f;   // mask the lower 4 bits (icon style)
    if(icon_index <= 4)
        icon = icon_index >> 1;
    else
        icon = 3;
    initMe(caption, message, b1text, b2text, b3text, b4text, icons[icon]);

    /*
     * check, if we have a default button, if not, set the left button to default
     */
    
    if( !( type & 0x000000f0 ) )
        type |= DB_FIRST;
    
    if(b1)
        b1->setDefault(type & DB_FIRST);
    if(b2)
        b2->setDefault(type & DB_SECOND);
    if(b3)
        b3->setDefault(type & DB_THIRD);
    if(b4)
        b4->setDefault(type & DB_FOURTH);
    
    setGeometry( x(), y(), w + 20, h);
}

void KMsgWin::initMe(const char *caption, const char *message,
                     const char *b1text, const char *b2text, const char *b3text, const char *b4text,
                    const KPixmap & icon)
{
    setCaption(caption);
    
    nr_buttons = 0;
    
    b1 = b2 = b3 = b4 = 0;

    if(b1text) {
        b1 = new QPushButton(b1text, this, "_b1");
        b1->resize(80, 25);
        connect(b1, SIGNAL(clicked()), this, SLOT(b1Pressed()));
        nr_buttons++;
    }

    if(b2text) {
        b2 = new QPushButton(b2text, this, "_b2");
        b2->resize(80, 25);
        connect(b2, SIGNAL(clicked()), this, SLOT(b2Pressed()));
        nr_buttons++;
    }

    if(b3text) {
        b3 = new QPushButton(b3text, this, "_b3");
        b3->resize(80, 25);
        connect(b3, SIGNAL(clicked()), this, SLOT(b3Pressed()));
        nr_buttons++;
    }

    if(b4text) {
        b4 = new QPushButton(b4text, this, "_b4");
        b4->resize(80, 25);
        connect(b4, SIGNAL(clicked()), this, SLOT(b4Pressed()));
        nr_buttons++;
    }
    
    msg = new QLabel(message, this, "_msg");
    msg->setAlignment(AlignCenter);
    msg->adjustSize();
    picture = new QLabel(this, "_pict");
    picture->setAutoResize(TRUE);
    picture->setPixmap(icon);
    calcOptimalSize();
    setMinimumSize(w + 20, h);
    f1 = new QFrame(this);
    f1->setLineWidth(1);
    f1->setFrameStyle(QFrame::HLine | QFrame::Sunken);
}


KMsgWin::~KMsgWin()
{
}

void KMsgWin::resizeEvent( QResizeEvent * _ev )
{
    /*
     * align buttons
     */
    int interval = B_WIDTH + B_SPACING;
    
    calcOptimalSize();

    f1->setGeometry(0, height() - 45, width(), 2);
    
    picture->move(15, 10 + (h1 - picture->height()) / 2);
    msg->move(text_offset + ((width() - text_offset) - msg->width()) / 2, 10 + (h1 - msg->height()) / 2);
    int left_offset = (width() - ((nr_buttons * 80) + (nr_buttons - 1) * 10)) / 2;
    int hw = height() - 30;

    if( b1 )
    {
        b1->move(left_offset, hw);
        left_offset += interval;
    }
    if( b2 )
    {
        b2->move(left_offset, hw);
        left_offset += interval;
    }
    if( b3 )
    {
        b3->move(left_offset, hw);
        left_offset += interval;
    }
    if( b4 )
        b4->move(left_offset, hw);

}

void KMsgWin::calcOptimalSize()
{
    int text_width = picture->width() + 10 + msg->width() + 30;  // label width + margins

    int button_width = (nr_buttons * 80) + 20 + (nr_buttons - 1) * B_SPACING;

    w = button_width > text_width ? button_width : text_width;
    
    h1 = msg->height() > picture->height() ? msg->height() : picture->height();

    h = h1 + 25 + 40;

    text_offset = 15 + picture->width() + 10;
}

void KMsgWin::b1Pressed() { emit result( this, 1 ); }
void KMsgWin::b2Pressed() { emit result( this, 2 ); }
void KMsgWin::b3Pressed() { emit result( this, 3 ); }
void KMsgWin::b4Pressed() { emit result( this, 4 ); }


