// -*- c++ -*-

#ifndef KBOOKMARKDLG_H
#define KBOOKMARKDLG_H

#include <qdialog.h>
#include <qlistbox.h>
#include <klined.h>
#include "bookmark.h"

class KBookmarkDlgRenameDlg : public QDialog
{
  Q_OBJECT

public:
  KBookmarkDlgRenameDlg(const char *, const char *);
  virtual ~KBookmarkDlgRenameDlg();
  const char *text() const;

private:
  KLined *ed;
};

/**
 * A simple bookmark editing dialog for kfm, kdehelp, and kfiledialog.
 * @author rich@kde.org
 * @version $Id$
 */
class KBookmarkDlg : public QDialog
{
  Q_OBJECT

public:
  /**
   * Popup the dialog on the contents of the specified bookmark manager.
   */
  KBookmarkDlg(KBookmarkManager *, QWidget *parent= 0, const char *name= 0);
  virtual ~KBookmarkDlg();

signals:
  void bookmarksChanged();
  void rereadBookmarks();

public slots:
  void refreshBookmarks();
  void applyChanges();
  void cancelChanges();

protected slots:
  void moveUp();
  void moveDown();
  void rename();
  void del();

private:
  QListBox *myList;
  bool myHaveSelection;
  QPushButton *myUpButton;
  QPushButton *myDownButton;
  QPushButton *myDelButton;
  QPushButton *myRenameButton;
  KBookmarkManager *myManager;
};

#endif // KBOOKMARKDLG_H
