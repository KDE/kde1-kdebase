/* 	Main Widget for showing system-dependent information.
	
	** main.cpp includes this file ! **

	This source-file includes another system-dependet sourcefile called
	info_<systemname>.cpp
	which should define one or more of the following defines to
	indicate, that this information is really available.
	
        #define INFO_CPU_AVAILABLE
	#define INFO_IRQ_AVAILABLE
        #define INFO_DMA_AVAILABLE
	#define INFO_PCI_AVAILABLE
        #define INFO_IOPORTS_AVAILABLE
	#define INFO_SOUND_AVAILABLE
        #define INFO_DEVICES_AVAILABLE
	#define INFO_SCSI_AVAILABLE
        #define INFO_PARTITIONS_AVAILABLE
	#define INFO_XSERVER_AVAILABLE
	
	right now, there is the problem, that also the .kdelnk-files should depend
	on the systemname, so that only available .kdelnk-files will be copied to
	kde/applnk/Settings/Information !!
*/


#include <qtabbar.h>

#include <kapp.h>
#include <kcharsets.h>
#include "info.h"
#include "info.moc"


#include <X11/Xlib.h>

void XServer_fill_screen_info( KTabListBox *lBox, Display *dpy, int scr )
{
    int i;
    double xres, yres;
    int ndepths = 0, *depths = NULL;
    QString str,str2;
    
    xres = ((((double) DisplayWidth(dpy,scr)) * 25.4) / 
	    ((double) DisplayWidthMM(dpy,scr)));
    yres = ((((double) DisplayHeight(dpy,scr)) * 25.4) / 
	    ((double) DisplayHeightMM(dpy,scr)));

    lBox->insertItem("");
    str.sprintf(i18n("screen #%d"),(int)scr);
    lBox->insertItem(str);
    str.sprintf(i18n("  dimensions          : %dx%d pixels (%dx%d mm)\n"),
	    DisplayWidth (dpy, scr), DisplayHeight (dpy, scr),
	    DisplayWidthMM(dpy, scr), DisplayHeightMM (dpy, scr));
    lBox->insertItem(str);
    str.sprintf(i18n("  resolution          : %dx%d dots per inch\n"), 
		    (int) (xres + 0.5), (int) (yres + 0.5));
    lBox->insertItem(str);
    depths = XListDepths (dpy, scr, &ndepths);
    if (!depths) ndepths = 0;
    str.sprintf(i18n("  depths (%d)          : "), ndepths);
    for (i = 0; i < ndepths; i++) 
    {	str2.sprintf("%d", depths[i]);
	str = str+str2;
	if (i < ndepths - 1)
	    str = str + ", ";
    }
    lBox->insertItem(str);
    if (depths) XFree((char *) depths);
}



bool GetInfo_XServer_Generic( KTabListBox *lBox )
{
    /* Parts of this source is taken from the X11-program "xdpyinfo" */
    
    Display *dpy;
    int i;
    QString str;

    dpy = XOpenDisplay(NULL);
    if (!dpy)  return FALSE;

    str.sprintf(i18n("name of display       : %s"),DisplayString(dpy));
    lBox->insertItem(str);
    str.sprintf(i18n("version number        : %i.%i"),(int)ProtocolVersion(dpy),(int)ProtocolRevision(dpy));
    lBox->insertItem(str);
    str.sprintf(i18n("vendor string         : %s"), ServerVendor(dpy));
    lBox->insertItem(str);
    str.sprintf(i18n("vendor release number : %i"),(int)VendorRelease(dpy));
    lBox->insertItem(str);
    str.sprintf(i18n("default screen number : %i"),(int)DefaultScreen(dpy));
    lBox->insertItem(str);
    str.sprintf(i18n("number of screens     : %i"),(int)ScreenCount(dpy));

    for (i = 0; i < ScreenCount (dpy); i++)
	XServer_fill_screen_info (lBox,dpy, i);

    XCloseDisplay (dpy);
    return TRUE;
}


/*
***************************************************************************
**  Include system-specific code					 **
***************************************************************************
*/

#ifdef linux
#include "info_linux.cpp"
#elif sgi || sun
#include "info_sgi.cpp"
#elif __FreeBSD__
#include "info_fbsd.cpp"
#elif hpux
#include "info_hpux.cpp"
#elif __svr4__
#include "info_svr4.cpp"
#else
#include "info_generic.cpp"	/* Default for unsupportet systems.... */
#endif


/*
***************************************************************************
***************************************************************************
***************************************************************************
*/






#define SCREEN_XY_OFFSET 20

void KInfoListWidget::defaultSettings()
{  
    bool ok = FALSE;
    
    if (lBox)	delete lBox;
    lBox 	= new KTabListBox(this);

    if (lBox)
    {   lBox->clear();
	lBox->setMinimumSize( 200,2*SCREEN_XY_OFFSET );
	lBox->setGeometry(SCREEN_XY_OFFSET,SCREEN_XY_OFFSET,
                     width() -2*SCREEN_XY_OFFSET,
		     height()-2*SCREEN_XY_OFFSET);
	lBox->setTableFont(kapp->fixedFont);
	lBox->enableKey();
	lBox->setAutoUpdate(TRUE);

        if (getlistbox)
	    ok = (*getlistbox)(lBox);

	if (lBox->numCols()<=1)  // if ONLY ONE COLUMN (!), then set title and calculate necessary column-width
	{   QFontMetrics fm(lBox->tableFont());
	    int row, cw, colwidth = 0;
	    row = lBox->numRows();
	    while (row>=0)			// loop through all rows in this single column
	    {  cw = fm.width(lBox->text(row));	// calculate the necessary width
	       if (cw>colwidth) colwidth=cw;
	       --row;
	    }
	    colwidth += 5;
	    if (localname) 
   	        lBox->setColumn(0,localname,colwidth); // set title and width
	    else
   	        lBox->setDefaultColumnWidth(colwidth); // only set width
	}

	if (ok) lBox->show();
    }

    if (!ok)
    {	if (lBox) { delete lBox; lBox = 0; }	    
	if (!NoInfoText)
	    NoInfoText = new QLabel(i18n("No Information available\n or\nthis system is not yet supported :-("),this);
	NoInfoText->setAutoResize(TRUE);
	NoInfoText->setAlignment(AlignCenter | WordBreak);
	NoInfoText->move( width()/2-120,height()/2-30 );
    }
}


KInfoListWidget::KInfoListWidget(QWidget *parent, const char *name, 
		const char *_localname, bool _getlistbox(KTabListBox *lbox))
  : KConfigWidget(parent, name)
{   char 	*p;
    getlistbox 	= _getlistbox;
    lBox 	= 0;
    NoInfoText  = 0;
    strncpy(&localname[0],_localname,sizeof(localname)-1);
    p = localname;
    while (*p) { if (*p=='&') strcpy(&p[0],&p[1]); else ++p; } /* delete the key-accelerator ! */
    setMinimumSize( 200,6*SCREEN_XY_OFFSET );
    defaultSettings();
}


void KInfoListWidget::resizeEvent( QResizeEvent *re )
{   QSize size = re->size();
    if (lBox)
        lBox->setGeometry(SCREEN_XY_OFFSET,SCREEN_XY_OFFSET,
                    size.width() -2*SCREEN_XY_OFFSET,
		    size.height()-2*SCREEN_XY_OFFSET);
    if (NoInfoText)
	NoInfoText->move( size.width()/2-120,size.height()/2-30 );
}


