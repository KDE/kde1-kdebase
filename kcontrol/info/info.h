#ifndef _INFO_H_
#define _INFO_H_


#include <qwidget.h>
#include <qframe.h>
#include <qlabel.h>
#include <qtabdlg.h>
#include <qpushbt.h>
#include <qtimer.h>
#include <qlistview.h>
#include <qfile.h>
#include <qevent.h>

#include <kmsgbox.h>
#include <kcontrol.h>
#include <ktablistbox.h>


/* function call-back-prototypes... */

bool GetInfo_CPU( KTabListBox *lBox );
bool GetInfo_IRQ( KTabListBox *lBox );
bool GetInfo_DMA( KTabListBox *lBox );
bool GetInfo_PCI( KTabListBox *lBox );
bool GetInfo_IO_Ports( KTabListBox *lBox );
bool GetInfo_Sound( KTabListBox *lBox );
bool GetInfo_Devices( KTabListBox *lBox );
bool GetInfo_SCSI( KTabListBox *lBox );
bool GetInfo_Partitions( KTabListBox *lBox );
bool GetInfo_XServer_and_Video( KTabListBox *lBox );



class KInfoListWidget : public KConfigWidget
{
  Q_OBJECT

public:

  KInfoListWidget(QWidget *parent, const char *name=0, const char *_localname=0, bool _getlistbox (KTabListBox *)=0);

  void applySettings() {};
  void loadSettings() {};
  virtual void defaultSettings();
  
private:
  KTabListBox 	*lBox;
  bool 		(*getlistbox) (KTabListBox *);
  char		localname[31];
  
  QLabel	*NoInfoText;

protected:
  virtual void resizeEvent( QResizeEvent * );
};


#endif
