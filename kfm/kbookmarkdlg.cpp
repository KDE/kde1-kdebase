#include <kapp.h>
#include <qpushbt.h>
#include <qlayout.h>
#include <qframe.h>
#include <qlabel.h>
#include <qstring.h>
#include "kbookmarkdlg.h"

KBookmarkDlg::KBookmarkDlg(KBookmarkManager *manager,
			   QWidget *parent, const char *name)
  : QDialog(parent, name, true)
{
  QPushButton *okButton;
  QPushButton *cancelButton;
  QLabel *dialogLabel;
  QFrame *frame;

  myManager= manager;
  setCaption(klocale->translate("Edit Bookmarks"));

  frame= new QFrame(this);
  frame->setFrameStyle(QFrame::Sunken | QFrame::Panel);

  dialogLabel= new QLabel(klocale->translate("Edit Bookmarks"),
			  frame, "title");
  dialogLabel->setFixedSize(dialogLabel->sizeHint());
  dialogLabel->setAlignment(AlignCenter);
  myList= new QListBox(frame);
  refreshBookmarks();
  connect( myManager, SIGNAL( changed() ), 
	   this, SLOT( refreshBookmarks() ) );

  // Create the action buttons
  myDelButton= new QPushButton(klocale->translate("Delete"), frame);
  myRenameButton= new QPushButton(klocale->translate("Rename"), frame);
  myUpButton= new QPushButton(klocale->translate("Move up"), frame);
  myDownButton= new QPushButton(klocale->translate("Move down"), frame);

  myUpButton->setFixedSize(myUpButton->sizeHint());
  myDownButton->setFixedSize(myDownButton->sizeHint());
  myRenameButton->setFixedSize(myRenameButton->sizeHint());
  myDelButton->setFixedSize(myDelButton->sizeHint());

  connect(myUpButton, SIGNAL(clicked()), SLOT(moveUp()) );
  connect(myDownButton, SIGNAL(clicked()), SLOT(moveDown()) );
  connect(myRenameButton, SIGNAL(clicked()), SLOT(rename()) );
  connect(myDelButton, SIGNAL(clicked()), SLOT(del()) );

  // And now arrange them
  QVBoxLayout *frameLayout= new QVBoxLayout(frame, 4);
  QHBoxLayout *actionLayout= new QHBoxLayout();

  frameLayout->addWidget(dialogLabel, 0);
  frameLayout->addWidget(myList, 1);
  frameLayout->addLayout(actionLayout, 0);
  actionLayout->addStretch();
  actionLayout->addWidget(myDelButton, 0, AlignCenter);
  actionLayout->addWidget(myRenameButton, 0, AlignCenter);
  actionLayout->addWidget(myUpButton, 0, AlignCenter);
  actionLayout->addWidget(myDownButton, 0, AlignCenter);
  actionLayout->addStretch();
  frameLayout->activate();

  // Create the ok/cancel push buttons
  okButton= new QPushButton(klocale->translate("Ok"), this);
  cancelButton= new QPushButton(klocale->translate("Cancel"), this);

  okButton->setFixedSize(okButton->sizeHint());
  cancelButton->setFixedSize(cancelButton->sizeHint());

  connect(okButton, SIGNAL(clicked()), SLOT(applyChanges()) );
  connect(okButton, SIGNAL(clicked()), SLOT(accept()) );
  connect(cancelButton, SIGNAL(clicked()), SLOT(cancelChanges()) );
  connect(cancelButton, SIGNAL(clicked()), SLOT(reject()) );

  // Now sort out the geometry
  QVBoxLayout *topLayout= new QVBoxLayout(this, 4);
  QHBoxLayout *buttonLayout= new QHBoxLayout();

  topLayout->addWidget(frame, 1);
  topLayout->addLayout(buttonLayout, 0);
  buttonLayout->addStretch();
  buttonLayout->addWidget(okButton, 0, AlignRight);
  buttonLayout->addWidget(cancelButton, 0, AlignRight);
  topLayout->activate();
}

KBookmarkDlg::~KBookmarkDlg()
{

}

void KBookmarkDlg::applyChanges() // SLOT
{
  myManager->write();
}

void KBookmarkDlg::cancelChanges() // SLOT
{
  myManager->reread();
}

void KBookmarkDlg::refreshBookmarks() // SLOT
{
  QString s;
  // Insert the bookmarks
  KBookmark *bm;

  myList->clear();
    
  for ( bm = myManager->getRoot()->getChildren().first(); bm != 0;
	bm = myManager->getRoot()->getChildren().next() )
    {
      if ( bm->getType() == KBookmark::URL )
	{
	  s.sprintf("%s", bm->getText());
	  myList->insertItem( (const char *) s);
	}
	//	else
	//	{
	//	    QPopupMenu *subMenu = new QPopupMenu;
	//	    menu->insertItem( bm->getText(), subMenu );
	//	    fillBookmarkMenu( bm, subMenu, id );
	//	}
    }
}


void KBookmarkDlg::moveUp() // SLOT
{
  int selected= myList->currentItem();

  if(myManager->moveUp(selected))
    myList->setCurrentItem(selected-1);
}

void KBookmarkDlg::moveDown() // SLOT
{
  uint selected= myList->currentItem();
  
  if (myManager->moveDown(selected))
    myList->setCurrentItem(selected+1);
}

void KBookmarkDlg::del() // SLOT
{
  int i= myList->currentItem();

  if(myManager->remove(i) && ((i-1) >= 0))
    myList->setCurrentItem(i-1);
}

void KBookmarkDlg::rename() // SLOT
{
  KBookmarkDlgRenameDlg *dlg;
  QString s;

  int i= myList->currentItem();

  if (i != -1) {
    s= myManager->getRoot()->getChildren().at(i)->getURL();
     dlg= new KBookmarkDlgRenameDlg(myList->text(i), s);
     if (dlg->exec()) {
       myManager->rename(i, dlg->text());
     }
  }
}

KBookmarkDlgRenameDlg::KBookmarkDlgRenameDlg(const char *oldName,
					     const char *url)
  : QDialog(0, "kbookmarkdlgrenamedlg", true)
{
  QPushButton *okButton;
  QPushButton *cancelButton;
  QLabel *dialogLabel;
  QString s;

  setCaption(klocale->translate("Rename Bookmark"));
  dialogLabel= new QLabel(this, "rename");
  s.sprintf("%s %s", klocale->translate("Rename Bookmark:"), url);
  dialogLabel->setText(s);
  dialogLabel->setFixedHeight(dialogLabel->sizeHint().height());
  dialogLabel->setMinimumWidth(dialogLabel->sizeHint().width());
  dialogLabel->setAlignment(AlignLeft);

  ed= new KLined(this, "edit");
  ed->setText(oldName);
  ed->setFixedHeight(ed->sizeHint().height());

  // Create the ok/cancel push buttons
  okButton= new QPushButton(klocale->translate("Ok"), this);
  cancelButton= new QPushButton(klocale->translate("Cancel"), this);

  okButton->setFixedSize(okButton->sizeHint());
  cancelButton->setFixedSize(cancelButton->sizeHint());

  connect(okButton, SIGNAL(clicked()), SLOT(accept()) );
  connect(cancelButton, SIGNAL(clicked()), SLOT(reject()) );

  // Now sort out the geometry
  QVBoxLayout *topLayout= new QVBoxLayout(this, 4);
  QHBoxLayout *buttonLayout= new QHBoxLayout();

  topLayout->addWidget(dialogLabel, 0);
  topLayout->addWidget(ed, 0);
  topLayout->addLayout(buttonLayout, 0);
  buttonLayout->addStretch();
  buttonLayout->addWidget(okButton, 0, AlignRight);
  buttonLayout->addWidget(cancelButton, 0, AlignRight);
  topLayout->activate();

  resize(350, 90);
}

KBookmarkDlgRenameDlg::~KBookmarkDlgRenameDlg()
{

}

const char *KBookmarkDlgRenameDlg::text() const
{
  return ed->text();
}

#include "kbookmarkdlg.moc"

