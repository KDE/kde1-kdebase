/* This file is part of the KDE libraries
    Copyright (C) 1996, 1997, 1998 Martin R. Jones <mjones@kde.org>
      
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
//-----------------------------------------------------------------------------
//
// KDE HTML Bookmarks
//
// (c) Martin R. Jones 1996
//

#ifndef __BOOKMARK_H__
#define __BOOKMARK_H__

#include <qlist.h>
#include <qstring.h>
#include <qpixmap.h>

#include <ksimpleconfig.h>

class KBookmarkManager;

class KBookmark
{
  friend KBookmarkManager;
  
public:
  enum { URL, Folder };

  /**
   * Creates a real bookmark ( type = Folder ) and saves the bookmark on the disk.
   */
  KBookmark( KBookmarkManager *, KBookmark *_parent, const char *_text, const char *_url );
  
  const char *text() { return m_text; }
  const char *url() { return m_url; }
  int type() { return m_type; }
  int id() { return m_id; }
  const char* file() { return m_file; }
  QPixmap* pixmap();
  QPixmap* miniPixmap();
  
  void append( KBookmark *_bm ) { m_lstChildren.append( _bm ); }
  
  QList<KBookmark> *children() { return &m_lstChildren; }
  
  KBookmark* findBookmark( int _id );
  KBookmark* findBookmark( const char *_url );
 
  static QString encode( const char* );
  static QString decode( const char* );
  
protected:
  /**
   * Creates a folder.
   */
  KBookmark( KBookmarkManager *, KBookmark *_parent, const char *_text );
  /**
   * Creates a bookmark from a file.
   */
  KBookmark( KBookmarkManager *, KBookmark *_parent, const char *_text,
             KSimpleConfig& _cfg, const char * _group );

  void clear();
  
  QString m_text;
  QString m_url;
  QString m_file;
  
  QPixmap* m_pPixmap;
  QPixmap* m_pMiniPixmap;
  
  int m_type;
  int m_id;
  
  QList<KBookmark> m_lstChildren;

  KBookmarkManager *m_pManager;
};

class KBookmarkManager : public QObject
{
  friend KBookmark;
  
  Q_OBJECT
public:
  KBookmarkManager();
  ~KBookmarkManager() {};

  void scan( const char *filename );

  KBookmark* root() { return &m_Root; }
  KBookmark* findBookmark( int _id ) { return m_Root.findBookmark( _id ); }
  KBookmark* findBookmark(const char *_url) {return m_Root.findBookmark( _url ); }
  void emitChanged();
  
signals:
  void changed();

public slots:
  void slotNotify( const char* _url );
  
protected:
  void scanIntern( KBookmark*, const char *filename );

  void disableNotify() { m_bNotify = false; }
  void enableNotify() { m_bNotify = true; }
    
  KBookmark m_Root;
  bool m_bAllowSignalChanged;
  bool m_bNotify;

  /**
   * This list is to prevent infinite looping while
   * scanning directories with ugly symbolic links
   */
  QList<QString> m_lstParsedDirs;
};

#endif	// __BOOKMARK_H__

