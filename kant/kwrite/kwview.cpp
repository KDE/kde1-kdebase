#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <qstring.h>
#include <qwidget.h>
#include <qfont.h>
#include <qpainter.h>
#include <qkeycode.h>
#include <qmsgbox.h>
#include <qpixmap.h>
#include <qfileinf.h>
#include <qfile.h>
#include <qdir.h>

#include <kapp.h>
#include <kfiledialog.h>
#include <kdir.h>

//#include <X11/Xlib.h>

#include "kwview.h"
#include "kwdoc.h"

struct BufferInfo {
  void *user;
  int w;
  int h;
};

QList<BufferInfo> bufferInfoList;
QPixmap *buffer = 0;

QPixmap *getBuffer(void *user) {
  BufferInfo *info;

  if (!buffer) buffer = new QPixmap;
  info = new BufferInfo;
  info->user = user;
  info->w = 0;
  info->h = 0;
  bufferInfoList.append(info);
  return buffer;
}

void resizeBuffer(void *user, int w, int h) {
  int z;
  BufferInfo *info;
  int maxW, maxH;

  maxW = w;
  maxH = h;
  for (z = 0; z < (int) bufferInfoList.count(); z++) {
    info = bufferInfoList.at(z);
    if (info->user == user) {
      info->w = w;
      info->h = h;
    } else {
      if (info->w > maxW) maxW = info->w;
      if (info->h > maxH) maxH = info->h;
    }
  }
  if (maxW != buffer->width() || maxH != buffer->height()) {
    buffer->resize(maxW,maxH);
  }
}

void releaseBuffer(void *user) {
  int z;
  BufferInfo *info;

  for (z = (int) bufferInfoList.count() -1; z >= 0 ; z--) {
    info = bufferInfoList.at(z);
    if (info->user == user) bufferInfoList.remove(z);
  }
  resizeBuffer(0,0,0);
}


KWriteView::KWriteView(KWrite *write, KWriteDoc *doc) : QWidget(write) {
  kWrite = write;
  kWriteDoc = doc;

  QWidget::setCursor(ibeamCursor);
  setBackgroundMode(NoBackground);
  setFocusPolicy(StrongFocus);
  move(2,2);

  xScroll = new QScrollBar(QScrollBar::Horizontal,write);
  yScroll = new QScrollBar(QScrollBar::Vertical,write);
  connect(xScroll,SIGNAL(valueChanged(int)),SLOT(changeXPos(int)));
  connect(yScroll,SIGNAL(valueChanged(int)),SLOT(changeYPos(int)));

  xPos = 0;
  yPos = 0;

  scrollTimer = 0;

  cursor.x = 0;
  cursor.y = 0;
  cursorOn = false;
  cursorTimer = 0;
  cXPos = 0;
  cOldXPos = 0;
  cursorMoved = false;

  startLine = 0;
  endLine = 0;
  updateState = 0;

  drawBuffer = getBuffer(this);

  doc->registerView(this);
}

KWriteView::~KWriteView() {
  kWriteDoc->removeView(this);
  releaseBuffer(this);
}

void KWriteView::cursorLeft(VConfig &c) {

  cursor.x--;
  if (c.flags & cfWrapCursor && cursor.x < 0 && cursor.y > 0) {
    cursor.y--;
    cursor.x = kWriteDoc->textLength(cursor.y);
  }
  cOldXPos = cXPos = kWriteDoc->textWidth(cursor);
  update(c);
}

void KWriteView::cursorRight(VConfig &c) {

  cursor.x++;
  if (c.flags & cfWrapCursor && cursor.x > kWriteDoc->textLength(cursor.y)) {
    if (cursor.y < kWriteDoc->numLines() -1) {
      cursor.y++;
      cursor.x = 0;
    } else {
      cursor.x = kWriteDoc->textLength(cursor.y);
    }
  }
  cOldXPos = cXPos = kWriteDoc->textWidth(cursor);
  update(c);
}


void KWriteView::cursorUp(VConfig &c) {

  cursor.y--;
  cXPos = kWriteDoc->textWidth(c.flags & cfWrapCursor,cursor,cOldXPos);
  update(c);
}

void KWriteView::cursorDown(VConfig &c) {

  cursor.y++;
  cXPos = kWriteDoc->textWidth(c.flags & cfWrapCursor,cursor,cOldXPos);
  update(c);
}

void KWriteView::home(VConfig &c) {

  cursor.x = 0;
  cOldXPos = cXPos = 0;
  update(c);
}

void KWriteView::end(VConfig &c) {

  cursor.x = kWriteDoc->textLength(cursor.y);
  cOldXPos = cXPos = kWriteDoc->textWidth(cursor);
  update(c);
}

void KWriteView::pageUp(VConfig &c) {

  cursor.y -= endLine - startLine;
//  int h = kWriteDoc->fontHeight;
//  if ((height() % h)*2 < h) cursor.y++;
  cXPos = kWriteDoc->textWidth(c.flags & cfWrapCursor,cursor,cOldXPos);
  update(c);
}

void KWriteView::pageDown(VConfig &c) {

  cursor.y += endLine - startLine;
//  int h = kWriteDoc->fontHeight;
//  if (height() % h < h/2) cursor.y--;
  cXPos = kWriteDoc->textWidth(c.flags & cfWrapCursor,cursor,cOldXPos);
  update(c);
}

void KWriteView::top(VConfig &c) {

  cursor.x = 0;
  cursor.y = 0;
  cOldXPos = cXPos = 0;
  update(c);
}

void KWriteView::bottom(VConfig &c) {

  cursor.x = 0;
  cursor.y = kWriteDoc->numLines() -1;//c.maxY;
  cOldXPos = cXPos = 0;
  update(c);
}


void KWriteView::changeXPos(int p) {
  int dx;

  dx = xPos - p;
  xPos = p;
  if (QABS(dx) < width()) scroll(dx,0); else QWidget::update();
}

void KWriteView::changeYPos(int p) {
  int dy;

  dy = yPos - p;
  yPos = p;
  startLine = yPos / kWriteDoc->fontHeight;
  endLine = (yPos + height() -1) / kWriteDoc->fontHeight;
  if (QABS(dy) < height()) scroll(0,dy); else QWidget::update();
}


void KWriteView::getVConfig(VConfig &c) {

  c.cursor = cursor;
  c.startCursor = cursor;
//  c.maxY = (int) kWriteDoc->contents.count() -1;
  c.flags = kWrite->configFlags;
  c.wrapAt = kWrite->wrapAt;

  kWriteDoc->unmarkFound();
}



void KWriteView::update(VConfig &c) {

  if (cursor.x == c.startCursor.x && cursor.y == c.startCursor.y) return;
  cursorMoved = true;

  if (cursorOn) {
    tagLines(c.startCursor.y,c.startCursor.y);
    cursorOn = false;
  }

  if (c.flags & cfMark) {
    kWriteDoc->selectTo(c.startCursor,cursor,c.flags);
  } else {
    if (!(c.flags & cfPersistent)) kWriteDoc->deselectAll();
  }
}


void KWriteView::updateCursor(PointStruc &start, PointStruc &end, bool insert) {
  int dy;
  PointStruc oldCursor;

  oldCursor = cursor;
  dy = end.y - start.y;

  if (cursor.y > start.y) {
    cursor.y += dy;
  } else if (cursor.y == start.y && cursor.x >= start.x) {
    cursor.y += dy;
    cursor.x += end.x - start.x;
  } else if (cursor.y > end.y || (cursor.y == end.y && cursor.x > end.x)) {
    cursor.y = end.y;
    cursor.x = end.x;
  }

  if (cursor.x != oldCursor.x || cursor.y != oldCursor.y) {
    cursorMoved = true;
    cOldXPos = cXPos = kWriteDoc->textWidth(cursor);
  }

  if (insert) {
    if (start.y < startLine && end.y < startLine) {
      startLine += dy;
      endLine += dy;
      yPos += dy*kWriteDoc->fontHeight;
    } else if (start.y <= endLine || end.y <= endLine) {
      if (dy == 0) {
        if (start.y == cursor.y) cursorOn = false;
        tagLines(start.y,start.y);
      } else tagAll();
    }
  } else {
    if (dy >= 0) tagLines(start.y,end.y); else tagLines(end.y,start.y);
  }
}


void KWriteView::updateCursor(PointStruc &newCursor) {
  cursorMoved = true;

  if (cursorOn) {
    tagLines(cursor.y,cursor.y);
    cursorOn = false;
  }
  cursor = newCursor;
  cOldXPos = cXPos = kWriteDoc->textWidth(cursor);
}

void KWriteView::updateView(int flags) {
  int fontHeight;
  int oldXPos, oldYPos;
  int w, h;
  int z;
  bool b;
  int xMax, yMax;
  int cYPos;
  int cXPosMin, cXPosMax, cYPosMin, cYPosMax;
  int dx, dy;

  if (cursorMoved) emit kWrite->newCurPos();

  if (!cursorMoved && updateState == 0) return;
  cursorMoved = false;

  if (cursorTimer) {
    killTimer(cursorTimer);
    cursorTimer = startTimer(500);
    cursorOn = true;
  } else {
//    if (updateState == 0) return;
//    updatePos = false;
  }



  fontHeight = kWriteDoc->fontHeight;
  oldXPos = xPos;
  oldYPos = yPos;

  cYPos = cursor.y*fontHeight;

  z = 0;
  do {
    w = kWrite->width() - 4;
    h = kWrite->height() - 4;

    xMax = kWriteDoc->textWidth() - w;
    b = (xPos > 0 || xMax > 0);
    if (b) h -= 16;
    yMax = kWriteDoc->textHeight() - h;
    if (yPos > 0 || yMax > 0) {
      w -= 16;
      xMax += 16;
      if (!b && xMax > 0) {
        h -= 16;
        yMax += 16;
      }
    }

    if (flags & ufNoScroll) break;

    if (flags & ufCenter) {
      cXPosMin = xPos + w/3;
      cXPosMax = xPos + (w*2)/3;
      cYPosMin = yPos + h/3;
      cYPosMax = yPos + ((h - fontHeight)*2)/3;
    } else {
      cXPosMin = xPos + 4;
      cXPosMax = xPos + w - 8;
      cYPosMin = yPos;
      cYPosMax = yPos + (h - fontHeight);
    }

    if (cXPos < cXPosMin) {
      xPos -= cXPosMin - cXPos;
      if (xPos < 0) xPos = 0;
    }
    if (cXPos > cXPosMax) {
      xPos += cXPos - cXPosMax;
    }
    if (cYPos < cYPosMin) {
      yPos -= cYPosMin - cYPos;
      if (yPos < 0) yPos = 0;
    }
    if (cYPos > cYPosMax) {
      yPos += cYPos - cYPosMax;
    }

/*
    if (cXPos < xPos +2) {
      xPos = cXPos -8;
      if (xPos < 0) xPos = 0;
    }
    if (cXPos > xPos + w -4) {
      xPos = cXPos - w +8;
    }

    if (cYPos < yPos) {
      yPos = cYPos;
      if (yPos < 0) yPos = 0;
    }
    if (cYPos > yPos + (h-fontHeight)) {
      yPos = cYPos - (h-fontHeight);
    }
*/
    z++;
  } while (z < 2);

  if (xMax < xPos) xMax = xPos;
  if (yMax < yPos) yMax = yPos;

  if (xMax > 0) {
    xScroll->blockSignals(true);
    xScroll->setGeometry(2,h + 2,w,16);
    xScroll->setRange(0,xMax);
    xScroll->setValue(xPos);
    xScroll->setSteps(fontHeight,w - 4 - 16);
    xScroll->blockSignals(false);
    xScroll->show();
  } else xScroll->hide();

  if (yMax > 0) {
    yScroll->blockSignals(true);
    yScroll->setGeometry(w + 2,2,16,h);
    yScroll->setRange(0,yMax);
    yScroll->setValue(yPos);
    yScroll->setSteps(fontHeight,h - 4 - 16);
    yScroll->blockSignals(false);
    yScroll->show();
  } else yScroll->hide();

  startLine = yPos / fontHeight;
  endLine = (yPos + h -1) / fontHeight;

  if (w != width() || h != height()) {
    resize(w,h);
  } else {
    dx = oldXPos - xPos;
    dy = oldYPos - yPos;

    b = updateState == 3;
    if (flags & ufUpdateOnScroll) {
      b |= dx || dy;
    } else {
      b |= QABS(dx)*3 > w*2 || QABS(dy)*3 > h*2;
    }

    if (b) {
      QWidget::update();
    } else {

      if (updateState > 0) paintTextLines(oldXPos,oldYPos);
      if (cursorOn && !(dx | dy)) paintCursor();

      if (dx | dy) {
        scroll(dx,dy);
//        kapp->syncX();
//        scroll2(dx - dx/2,dy - dy/2);
      }
    }
  }
  updateState = 0;
}

/*
void KWriteView::scroll2( int dx, int dy ) {
  int w, h, x, y;
  Display *dpy;
  int winid;

  if ( dx == 0 && dy == 0 ) return;
  dpy = qt_xdisplay();
  winid = winId();
  w = width();
  h = height();

  XCopyArea( dpy, winid, winid, qt_xget_readonly_gc(),-dx,-dy,w,h,0,0);
//XSync(qt_xdisplay(),false);
usleep(100000);
*/
/*
  if ( dx ) {
    x = (dx < 0) ? w + dx : 0;
    dx = QABS(dx);
    XClearArea( dpy, winid,x,0,dx/2,h, TRUE);
    XClearArea( dpy, winid,x + dx/2,0,dx - dx/2,h, TRUE);
  }
  if ( dy ) {
    y = (dy < 0) ? h + dy : 0;
    XClearArea( dpy, winid,0,y,w,QABS(dy), TRUE);
  }
*/
//}


void KWriteView::tagLines(int start, int end) {
  int line, z;

  if (updateState < 3) {
    if (start < startLine) start = startLine;
    if (end > endLine) end = endLine;

    if (end - start > 1) {
      updateState = 3;
    } else {
      for (line = start; line <= end; line++) {
        for (z = 0; z < updateState && updateLines[z] != line; z++);
        if (z == updateState) {
          updateState++;
          if (updateState > 2) break;
          updateLines[z] = line;
        }
      }
    }
  }
}

void KWriteView::tagAll() {
  updateState = 3;
}

void KWriteView::paintTextLines(int xPos, int yPos) {
  int xStart, xEnd;
  int line, z;
  int h;

  QPainter paint;
  paint.begin(drawBuffer);

  xStart = xPos-2;
  xEnd = xStart + width();
  h = kWriteDoc->fontHeight;
  for (z = 0; z < updateState; z++) {
    line = updateLines[z];
    kWriteDoc->paintTextLine(paint,line,xStart,xEnd);
    bitBlt(this,0,line*h - yPos,drawBuffer,0,0,width(),h);
  }
  paint.end();
}

void KWriteView::paintCursor() {
  int h, y, x;

  h = kWriteDoc->fontHeight;
  y = h*cursor.y - yPos;
  x = cXPos - (xPos-2);

  QPainter paint;
  if (cursorOn) {
    paint.begin(this);
    paint.setPen(kWriteDoc->cursorCol(cursor.x,cursor.y));

    h += y - 1;
    paint.drawLine(x,y,x,h);
    paint.drawLine(x-2,y,x+2,y);
    paint.drawLine(x-2,h,x+2,h);
  } else {
    paint.begin(drawBuffer);
    kWriteDoc->paintTextLine(paint,cursor.y,cXPos - 2,cXPos + 3);
    bitBlt(this,x - 2,y,drawBuffer,0,0,5,h);
  }
  paint.end();
}

void KWriteView::placeCursor(int x, int y, int flags) {
  VConfig c;

  getVConfig(c);
  c.flags |= flags;
  cursor.y = (yPos + y)/kWriteDoc->fontHeight;
  cXPos = cOldXPos = kWriteDoc->textWidth(c.flags & cfWrapCursor, cursor,xPos-2 + x);
  update(c);
}

void KWriteView::focusInEvent(QFocusEvent *) {
//  printf("got focus %d\n",cursorTimer);

  if (!cursorTimer) {
    cursorTimer = startTimer(500);
    cursorOn = true;
    paintCursor();
  }
}

void KWriteView::focusOutEvent(QFocusEvent *) {
//  printf("lost focus\n");

  if (cursorTimer) {
    killTimer(cursorTimer);
    cursorTimer = 0;
  }

  if (cursorOn) {
    cursorOn = false;
    paintCursor();
  }
}

void KWriteView::keyPressEvent(QKeyEvent *e) {
  VConfig c;
  bool t;
//  printf("ascii %i, key %i, state %i\n",e->ascii(), e->key(), e->state());

/*
CTRL+
A, <-  : home
B      : <-
C      : copy
D      : del
E, ->  : end
F      : ->
H      : backspace
K      : kill line
N      : down
P      : up
V      : paste
X      : cut
*/

  getVConfig(c);

  if ((e->ascii() >= 32 || e->ascii() == '\t')
    && e->key() != Key_Delete && e->key() != Key_Backspace) {
//    printf("input %d\n",e->ascii());
    if (c.flags & cfDelOnInput) {
      kWriteDoc->cut(c.flags);
      getVConfig(c);
    }
    kWriteDoc->insertChar(c,e->ascii());
  } else {
    if (e->state() & ShiftButton) c.flags |= cfMark;

    t = false;
    if ( e->state() & ControlButton ) {
      t = true;
      switch ( e->key() ) {
/*
        case Key_A:
        case Key_Left:
            home(c);
            break;
        case Key_B:
            cursorLeft(c);
            break; */
        case Key_Insert:
//        case Key_C:
            kWriteDoc->copy(c.flags);
            break;
/*        case Key_D:
            kWriteDoc->del(c);
            break;
        case Key_E:
        case Key_Right:
            end(c);
            break;
        case Key_F:
            cursorRight(c);
            break;
        case Key_H:
            kWriteDoc->backspace(c);
            break;   */
        case Key_K:
            kWriteDoc->killLine(c);
            break;
/*        case Key_N:
            cursorDown(c);
            break;
        case Key_P:
            cursorUp(c);
            break;
        case Key_V:
            kWriteDoc->paste(c);
            break;   */
        case Key_Delete:
//        case Key_X:
            kWriteDoc->cut(c.flags);
            break;
//      case Key_Left:
//          cursorLeft(c);
//          break;
//      case Key_Right:
//          cursorRight(c);
//          break;
        case Key_Next:
            bottom(c);
            break;
        case Key_Prior:
            top(c);
            break;
        default:
            t = false;
      }
    }
    if (e->state() & ControlButton) c.flags |= cfMark | cfKeepSelection;
    if (!t) {
      switch ( e->key() ) {
        case Key_Left:
            cursorLeft(c);
            break;
        case Key_Right:
            cursorRight(c);
            break;
        case Key_Up:
            cursorUp(c);
            break;
        case Key_Down:
            cursorDown(c);
            break;
        case Key_Backspace:
            kWriteDoc->backspace(c);
            break;
        case Key_Home:
            home(c);
            break;
        case Key_End:
            end(c);
            break;
        case Key_Delete:
            if (c.flags & cfDelOnInput || e->state() & ShiftButton) kWriteDoc->cut(c.flags); else kWriteDoc->del(c);
            break;
        case Key_Next:
            pageDown(c);
            break;
        case Key_Prior:
            pageUp(c);
            break;
        case Key_Enter:
        case Key_Return:
            kWriteDoc->newLine(c);
            //emit returnPressed();
            e->ignore();
            break;
        case Key_Insert:
            if (e->state() & ShiftButton) kWriteDoc->paste(c);
              else kWrite->toggleOverwriteMode();
      }
    }
  }
  kWriteDoc->updateViews();
  e->accept();
}

void KWriteView::mousePressEvent(QMouseEvent *e) {
  int flags;

//printf("mousepress\n");
  if (e->button() == MidButton) {
    placeCursor(e->x(),e->y(),0);
    kWrite->paste();
  } else {
    flags = 0;
    if (e->state() & ShiftButton) {
      flags |= cfMark;
      if (e->state() & ControlButton) flags |= cfMark | cfKeepSelection;
    }
    placeCursor(e->x(),e->y(),flags);
    scrollX = 0;
    scrollY = 0;
    if (!scrollTimer) scrollTimer = startTimer(50);
    kWriteDoc->updateViews();
  }
}

void KWriteView::mouseReleaseEvent(QMouseEvent *e) {

//printf("mouserelease\n");
  if (e->button() == MidButton) return;
  kWrite->copy();
  killTimer(scrollTimer);
  scrollTimer = 0;
}

void KWriteView::mouseMoveEvent(QMouseEvent *e) {
  int flags;
  int d;

//printf("mousemove\n");
//if (e->button() == MidButton) return;
  mouseX = e->x();
  mouseY = e->y();
// printf("mouse moved %d %d\n",mx,my);
  scrollX = 0;
  scrollY = 0;
  d = kWriteDoc->fontHeight;
  if (mouseX < 0) {
    mouseX = 0;
    scrollX = -d;
  }
  if (mouseX > width()) {
    mouseX = width();
    scrollX = d;
  }
  if (mouseY < 0) {
    mouseY = 0;
    scrollY = -d;
  }
  if (mouseY > height()) {
    mouseY = height();
    scrollY = d;
  }

  flags = cfMark;
  if (e->state() & ControlButton) flags |= cfKeepSelection;
  placeCursor(mouseX,mouseY,flags);
  kWriteDoc->updateViews(ufNoScroll);
}

void KWriteView::paintEvent(QPaintEvent *e) {
  int xStart, xEnd;
  int h;
  int line, y, yEnd;

  QRect updateR = e->rect();
//  printf("update rect  = ( %i, %i, %i, %i )\n",
//    updateR.x(),updateR.y(), updateR.width(), updateR.height() );

  QPainter paint;
  paint.begin(drawBuffer);

  xStart = xPos-2 + updateR.x();
  xEnd = xStart + updateR.width();

  h = kWriteDoc->fontHeight;
  line = (yPos + updateR.y()) / h;
  y = line*h - yPos;
  yEnd = updateR.y() + updateR.height();

  while (y < yEnd) {
    kWriteDoc->paintTextLine(paint,line,xStart,xEnd);
//    if (cursorOn && line == cursor.y) paintCursor(paint,cXPos - xStart,h);
    bitBlt(this,updateR.x(),y,drawBuffer,0,0,updateR.width(),h);

    line++;
    y += h;
  }
  paint.end();
  if (cursorOn) paintCursor();
}

void KWriteView::resizeEvent(QResizeEvent *) {
  printf("KWriteView::resize\n");
  resizeBuffer(this,width(),kWriteDoc->fontHeight);
  QWidget::update();
}

void KWriteView::timerEvent(QTimerEvent *e) {
  if (e->timerId() == cursorTimer) {
    cursorOn = !cursorOn;
    paintCursor();

//    QPainter paint;
//    paint.begin(drawBuffer);

//    if (cursorOn = !cursorOn) {
//      paintCursor(paint);
//    } else {
//      kWriteDoc->paintTextLine(paint,cursor.y,xPos-2,cXPos - 2, cXPos + 3,yPos);
//    }

//    kWriteDoc->paintTextLine(paint,cursor.y,xPos-2,cXPos - 2, cXPos + 3);
//    if (cursorOn) paintCursor(paint);
//    bitBlt(this,cXPos - 2 - (xPos-2),line*fontHeight - yPos,drawBuffer,cXPos - 2 - (xPos-2),0,5,fontHeight);

//    paint.end();
  }
  if (e->timerId() == scrollTimer && (scrollX | scrollY)) {
    xScroll->setValue(xPos + scrollX);
    yScroll->setValue(yPos + scrollY);

    placeCursor(mouseX,mouseY,cfMark);
    kWriteDoc->updateViews(ufNoScroll);
  }
}


KWrite::KWrite(KWriteDoc *doc, QWidget *parent) : QWidget(parent) {
  kWriteDoc = doc;
  kWriteView = new KWriteView(this,doc);

  kfm = 0L;
  configFlags = cfRemoveSpaces | cfPersistent;// | cfReplaceTabs;
  searchFlags = 0;

  kWriteView->setFocus();
}

KWrite::~KWrite() {
  delete kWriteView;
}


int KWrite::currentLine() {
  return kWriteView->cursor.y;
}

int KWrite::currentColumn() {
  return kWriteDoc->currentColumn(kWriteView->cursor);
}

void KWrite::toggleOverwriteMode() {
  configFlags = configFlags ^ cfOvr;
  emit newStatus();
}

bool KWrite::isOverwriteMode() {
  return (configFlags & cfOvr);
}

void KWrite::setModified(bool m) {
  kWriteDoc->setModified(m);
}

bool KWrite::isModified() {
  return kWriteDoc->isModified();
}

bool KWrite::isLastView() {
  return kWriteDoc->isLastView(1);
}

KWriteDoc *KWrite::doc() {
  return kWriteDoc;
}

void KWrite::loadFile(QIODevice &dev, bool insert) {
  VConfig c;
  if (!insert) kWriteDoc->clear();
  kWriteView->getVConfig(c);
  kWriteDoc->insertFile(c,dev);
  if (!insert) setModified(false);
  kWriteDoc->updateViews();
}

bool KWrite::loadFile(const char *name, bool insert) {
printf("load file %s\n",name);
  QFileInfo info(name);
  if (!info.exists()) {
    QMessageBox::warning(this,
      i18n("Sorry"),
      i18n("The specified File does not exist"),
      i18n("OK"),
      "",
      "",
      0,0);
    return false;
  }
  if (info.isDir()) {
    QMessageBox::warning(this,
      i18n("Sorry"),
      i18n("You have specified a directory"),
      i18n("OK"),
      "",
      "",
      0,0);
    return false;
  }
  if (!info.isReadable()) {
    QMessageBox::warning(this,
      i18n("Sorry"),
      i18n("You do not have read permission to this file"),
      i18n("OK"),
      "",
      "",
      0,0);
    return false;
  }

  QFile f(name);
  if (f.open(IO_ReadOnly)) {
    loadFile(f,insert);
    f.close();
    return true;//kWriteDoc->setFileName(name);
  }
  QMessageBox::warning(this,
    i18n("Sorry"),
    i18n("An Error occured while trying to open this Document"),
    i18n("OK"),
    "",
    "",
    0,0);
  return false;
}

void KWrite::loadURL(const char *url, bool insert) {
  KURL u(url);

  if (u.isMalformed()) {
    QString s;
    if (url) {
      s = "file:";
      if (*url != '/') {
        s += QDir::currentDirPath();
        s += '/';
      }
      s += url;
      u.parse(s);
    }
    if (u.isMalformed()) {
      s.sprintf("%s\n%s",i18n("Malformed URL"),url);
      QMessageBox::warning(this,
        i18n("Sorry"),
        s,
        i18n("OK"),
        "",
        "",
        0,0);
      return;
    }
  }
  if (u.isLocalFile()) {
    // usual local file
    QString name(u.path());
    KURL::decodeURL(name);

    if (loadFile(name,insert)) kWriteDoc->setFileName(u.url());
  } else {
    // url
    if (kfm != 0L) {
      QMessageBox::information(this,
        i18n("Sorry"),
        i18n("KWrite is already waiting\nfor an internet job to finish\n"\
             "Please wait until it has finished\nAlternatively stop the running one."),
        i18n("Ok"),
        "",
        "",
        0,0);
      return;
    }

//    setGeneralStatusField(klocale->translate("Calling KFM"));

    kfm = new KFM;
//    setGeneralStatusField(klocale->translate("Done"));
    if (!kfm->isOK()) {
      QMessageBox::warning(this,
        i18n("Sorry"),
        i18n("Could not start or find KFM"),
        i18n("Ok"),
        "",
        "",
        0,0);
      delete kfm;
      kfm = 0L;
      return;
    }

//    setGeneralStatusField(klocale->translate("Starting Job"));
    kfmURL = u.url();
    kfmFile.sprintf("/tmp/kwrite%i",time(0L));
    kfmAction = KWrite::GET;
    kfmInsert = insert;

    connect(kfm,SIGNAL(finished()),this,SLOT(kfmFinished()));
    connect(kfm,SIGNAL(error(int, const char *)),this,SLOT(kfmError(int, const char *)));
//  setGeneralStatusField(klocale->translate("Connected"));
    kfm->copy(url,kfmFile);
//  setGeneralStatusField(klocale->translate("Waiting..."));
  }

}

void KWrite::writeFile(QIODevice &dev) {
  kWriteDoc->writeFile(dev);
  kWriteDoc->updateViews();
}

bool KWrite::writeFile(const char *name) {

  QFileInfo info(name);
  if(info.exists() && !info.isWritable()) {
    QMessageBox::warning(this,
      i18n("Sorry"),
      i18n("You do not have write permission to this file"),
      i18n("OK"),
      "",
      "",
      0,0);
    return false;
  }

  QFile f(name);
  if (f.open(IO_WriteOnly | IO_Truncate)) {
    writeFile(f);
    f.close();
    return true;//kWriteDoc->setFileName(name);
  }
  QMessageBox::warning(this,
    i18n("Sorry"),
    i18n("An Error occured while trying to open this Document"),
    i18n("OK"),
    "",
    "",
    0,0);
  return false;
}

void KWrite::writeURL(const char *url) {
  KURL u(url);

  if (u.isLocalFile()) {
    // usual local file
    QString name(u.path());
    KURL::decodeURL(name);


    if (writeFile(name)) {
      kWriteDoc->setFileName(url);
      setModified(false);
    }
  } else {
    // url
    if (kfm != 0L) {
      QMessageBox::information(this,
        i18n("Sorry"),
        i18n("KWrite is already waiting\nfor an internet job to finish\n"\
             "Please wait until it has finished\nAlternatively stop the running one."),
        i18n("Ok"),
        "",
        "",
        0,0);
      return;
    }

//    setGeneralStatusField(klocale->translate("Calling KFM"));

    kfmURL = url;
    kfmFile.sprintf("/tmp/kwrite%i",time(0L));
    kfmAction = KWrite::PUT;
    if (!writeFile(kfmFile)) return;

    kfm = new KFM;
//    setGeneralStatusField(klocale->translate("Done"));
    if (!kfm->isOK()) {
      QMessageBox::warning(this,
        i18n("Sorry"),
        i18n("Could not start or find KFM"),
        i18n("Ok"),
        "",
        "",
        0,0);
      delete kfm;
      kfm = 0L;
      return;
    }

//    setGeneralStatusField(klocale->translate("Starting Job"));

    connect(kfm,SIGNAL(finished()),this,SLOT(kfmFinished()));
    connect(kfm,SIGNAL(error(int, const char *)),this,SLOT(kfmError(int, const char *)));
//  setGeneralStatusField(klocale->translate("Connected"));
    kfm->copy(kfmFile,url);
//  setGeneralStatusField(klocale->translate("Waiting..."));
  }
}



const char *KWrite::fileName() {
  return kWriteDoc->fileName();//fPath;
}


bool KWrite::canDiscard() {
  int query;

  if (isModified()) {
    query = QMessageBox::warning(this,
      i18n("Warning"),
      i18n("The current Document has been modified.\nWould you like to save it?"),
      i18n("Yes"),
      i18n("No"),
      i18n("Cancel"),
      0,2);
    switch (query) {
      case 0: //yes
        save();
        if (isModified()) {
            query = QMessageBox::warning(this,
            i18n("Sorry"),
            i18n("Could not save the document.\nOpen a new document anyways?"),
            i18n("Yes"),
            i18n("No"),
            "",
            0,1);
          if (query == 1) return false; //no
        }
        break;
      case 2: //cancel
        return false;
    }
  }
  return true;
}

void KWrite::newDoc() {

  if (canDiscard()) clear();
}

void KWrite::open() {
  QString url;

  if (!canDiscard()) return;
//  if (kWriteDoc->hasFileName()) s = QFileInfo(kWriteDoc->fileName()).dirPath();
//    else s = QDir::currentDirPath();

  url = KFileDialog::getOpenFileURL(kWriteDoc->fileName(),"*",topLevelWidget());
  if (url.isEmpty()) return;
  loadURL(url);
}

void KWrite::insertFile() {
  QString url;

  url = KFileDialog::getOpenFileURL(kWriteDoc->fileName(),"*",topLevelWidget());
  if (url.isEmpty()) return;
  loadURL(url,true);
}

void KWrite::save() {
  if (isModified()) {
    if (kWriteDoc->hasFileName()) writeURL(kWriteDoc->fileName()); else saveAs();
  }
}

void KWrite::saveAs() {
  QString url;
  int query;

  do {
    query = 0;
    url = KFileDialog::getSaveFileURL(kWriteDoc->fileName(),"*",this);
    if (url.isEmpty()) return;

    KURL u(url);
    if (u.isLocalFile()) {
      QFileInfo info;
      QString name(u.path());
      KURL::decodeURL(name);
      info.setFile(name);
      if (info.exists()) {
        query = QMessageBox::warning(this,
          i18n("Warning"),
          i18n("A Document with this Name already exists.\nDo you want to overwrite it?"),
          i18n("Yes"),
          i18n("No"),
          "",0,1);
      }
    }
  } while (query == 1);

  writeURL(url);
}


void KWrite::kfmFinished() {
//  QString string;
//  string.sprintf(klocale->translate("Finished '%s'"),tmpFile.data());
//  setGeneralStatusField(string);

  if (kfmAction == GET ) {
//    KURL u(kfmFile);
  //  if (!kfm->isOK()) printf("kfm not ok!!!\n");
    if (loadFile(kfmFile,kfmInsert)) kWriteDoc->setFileName(kfmURL);

    //clean up
    unlink(kfmFile);
  }
  if (kfmAction == PUT) {
//    ->toggleModified( FALSE );
    kWriteDoc->setFileName(kfmURL);
    setModified(false);
    //clean up
    unlink(kfmFile);
  }
  delete kfm;
  kfm = 0L;
}

void KWrite::kfmError(int e, const char *s) {
  printf("error %d = %s\n",e,s);
}

void KWrite::clear() {
  kWriteDoc->clear();
  kWriteDoc->setFileName(0);
  kWriteDoc->updateViews();
}

void KWrite::cut() {
  kWriteDoc->cut(configFlags);
  kWriteDoc->updateViews();
}

void KWrite::copy() {
  kWriteDoc->copy(configFlags);
}

void KWrite::paste() {
  VConfig c;
  kWriteView->getVConfig(c);
  kWriteDoc->paste(c);
  kWriteDoc->updateViews();
}

void KWrite::selectAll() {
  kWriteDoc->selectAll();
  kWriteDoc->updateViews();
}

void KWrite::deselectAll() {
  kWriteDoc->deselectAll();
  kWriteDoc->updateViews();
}

void KWrite::invertSelection() {
  kWriteDoc->invertSelection();
  kWriteDoc->updateViews();
}


void KWrite::search() {
  SearchDialog *searchDialog;

  searchDialog = new SearchDialog(searchFor,replaceWith,
    searchFlags & ~sfReplace,topLevelWidget());

  kWriteView->focusOutEvent(0);// QT bug ?
  if (searchDialog->exec() == QDialog::Accepted) {
    searchFor = searchDialog->getSearchFor();
    searchFlags = searchDialog->getFlags() | (searchFlags & sfPrompt);
    searchAgain(searchFlags);
  }
  delete searchDialog;
}

void KWrite::replace() {
  SearchDialog *searchDialog;

  searchDialog = new SearchDialog(searchFor,replaceWith,
    searchFlags | sfReplace,topLevelWidget());

  kWriteView->focusOutEvent(0);// QT bug ?
  if (searchDialog->exec() == QDialog::Accepted) {
    searchFor = searchDialog->getSearchFor();
    replaceWith = searchDialog->getReplaceWith();
    searchFlags = searchDialog->getFlags();
    replaceAgain(searchFlags);
  }
  delete searchDialog;
}

//usleep(50000);
//XSync(qt_xdisplay(),true);
//kapp->syncX();
//printf("xpending %d\n",XPending(qt_xdisplay()));
//kapp->processEvents();
//    kWriteView->tagAll();
//    searchAgain();

void KWrite::searchAgain() {
  int flags;

  flags = searchFlags | sfFromCursor | sfPrompt | sfAgain;
  if (flags & sfReplace) replaceAgain(flags); else searchAgain(flags);
}

void KWrite::searchAgain(int flags) {
  int result, query;
  PointStruc cursor;
  QString s;

  cursor = kWriteView->cursor;
  do {
    query = 1;
    result = kWriteDoc->doSearch(searchFor,flags,cursor);
    if (result) {
      kWriteView->updateCursor(cursor);
      kWriteDoc->updateViews(((flags & sfAgain) ? 0 : ufUpdateOnScroll) | ufCenter);
    } else {
      if (flags & sfFromCursor) {
        // from cursor
        if (!(flags & sfBackward)) {
          // forward search
          s.sprintf("%s.\n%s?",
            i18n("End of document reached"),
            i18n("Continue from the beginning"));
          query = QMessageBox::information(this,
            i18n("Find"),
            s,
            i18n("Yes"),
            i18n("No"),
            "",0,1);
        } else {
          // backward search
          s.sprintf("%s.\n%s?",
            i18n("Beginning of document reached"),
            i18n("Continue from the end"));
          query = QMessageBox::information(this,
            i18n("Find"),
            s,
            i18n("Yes"),
            i18n("No"),
            "",0,1);
        }
        flags &= ~sfFromCursor & ~sfAgain;
      } else {
        // entire scope
        QMessageBox::information(this,
          i18n("Find"),
          i18n("Search string not found!"),
          i18n("OK"),
          "",
          "",0,0);
      }
    }
  } while (query == 0);
}

void qt_enter_modal(QWidget *);

void KWrite::replaceAgain(int flags) {
  int result, query;
  PointStruc cursor;
  int replaces;
  QString s;
  ReplacePrompt *prompt;
  bool fromCursor;

  fromCursor = flags & sfFromCursor;
  cursor = kWriteView->cursor;
  replaces = 0;
  prompt = 0;
  do {
    query = 1;
    result = kWriteDoc->doSearch(searchFor,flags,cursor);
    if (result) {
      query = 0;
      if (flags & sfPrompt) {
        kWriteView->updateCursor(cursor);
        kWriteDoc->updateViews(((flags & sfAgain) ? 0 : ufUpdateOnScroll) | ufCenter);
        if (!prompt) {
          prompt = new ReplacePrompt(topLevelWidget());
          result = prompt->exec();
        } else {
          qt_enter_modal(prompt);
          kapp->enter_loop();
          result = prompt->result();
        }
        //delete prompt;
//XSetTransientForHint(qt_xdisplay(), prompt->winId(), topLevelWidget()->winId());
//  result = 0;
        switch (result) {
          case 0: //cancel
            delete prompt;
            return;
          case 3: //all
            flags &= ~sfPrompt;
          case 1: //yes
            kWriteDoc->replace(cursor,searchFor.length(),replaceWith,flags & sfBackward);
            replaces++;
            break;
          case 2: //no
            break;
        }
      } else {
        kWriteDoc->replace(cursor,searchFor.length(),replaceWith,flags);
        replaces++;
      }
      flags |= sfFromCursor;
    } else {
      delete prompt;
      prompt = 0;
      kWriteDoc->updateViews();
      if (fromCursor) {
        // from cursor
        if (!(flags & sfBackward)) {
          // forward search
          s.sprintf("%d %s.\n%s.\n%s?",
            replaces,i18n("replace(s) made"),
            i18n("End of document reached"),
            i18n("Continue from the beginning"));
          query = QMessageBox::information(this,
            i18n("Replace"),
            s,
            i18n("Yes"),
            i18n("No"),
            "",0,1);
        } else {
          // backward search
          s.sprintf("%d %s.\n%s.\n%s?",
            replaces,i18n("replace(s) made"),
            i18n("Beginning of document reached"),
            i18n("Continue from the end"));
          query = QMessageBox::information(this,
            i18n("Replace"),
            s,
            i18n("Yes"),
            i18n("No"),
            "",0,1);
        }
      } else {
        // entire scope
        s.sprintf("%d %s.",
          replaces,i18n("replace(s) made"));
        QMessageBox::information(this,
          i18n("Replace"),
          s,
          i18n("OK"),
          "",
          "",0,0);
      }
      flags &= ~sfFromCursor & ~sfAgain;
      fromCursor = false;
      replaces = 0;
    }
  } while (query == 0);
}

void KWrite::gotoLine() {
  GotoLineDialog *dlg;
  PointStruc cursor;

  dlg = new GotoLineDialog(kWriteView->cursor.y + 1,topLevelWidget());
  if (dlg->exec() == QDialog::Accepted) {
    cursor.x = 0;
    cursor.y = dlg->getLine() - 1;
    kWriteView->updateCursor(cursor);
    kWriteView->updateView(ufUpdateOnScroll);
  }
  delete dlg;
}


void KWrite::readConfig(KConfig *config) {
  int flags;

  config->setGroup("Search Options");
  flags = 0;
  if (config->readNumEntry("CaseSensitive")) flags |= sfCaseSensitive;
  if (config->readNumEntry("WholeWordsOnly")) flags |= sfWholeWords;
  if (config->readNumEntry("FromCursor")) flags |= sfFromCursor;
  if (config->readNumEntry("FindBackwards")) flags |= sfBackward;
  if (config->readNumEntry("SelectedText")) flags |= sfSelected;
  if (config->readNumEntry("PromptOnReplace",1)) flags |= sfPrompt;
  searchFlags = flags;

  config->setGroup("Settings");
  flags = 0;
  if (config->readNumEntry("AutoIndent")) flags |= cfAutoIndent;
  if (config->readNumEntry("BackspaceIndent")) flags |= cfBackspaceIndent;
  if (config->readNumEntry("WordWrap")) flags |= cfWordWrap;
  if (config->readNumEntry("ReplaceTabs")) flags |= cfReplaceTabs;
  if (config->readNumEntry("RemoveTrailingSpaces",1)) flags |= cfRemoveSpaces;
  if (config->readNumEntry("WrapCursor")) flags |= cfWrapCursor;
  if (config->readNumEntry("AutoBrackets")) flags |= cfAutoBrackets;
  if (config->readNumEntry("PersistentSelections",1)) flags |= cfPersistent;
  if (config->readNumEntry("MultipleSelections")) flags |= cfKeepSelection;
  if (config->readNumEntry("VerticalSelections")) flags |= cfVerticalSelect;
  if (config->readNumEntry("DeleteOnInput")) flags |= cfDelOnInput;
  if (config->readNumEntry("ToggleOld")) flags |= cfXorSelect;
  configFlags = flags;

  wrapAt = config->readNumEntry("WrapAt",78);
  kWriteDoc->setTabWidth(config->readNumEntry("TabWidth",8));
}

void KWrite::writeConfig(KConfig *config) {
  int flags;

  config->setGroup("Search Options");
  flags = searchFlags;
  config->writeEntry("CaseSensitive",(flags & sfCaseSensitive) != 0);
  config->writeEntry("WholeWordsOnly",(flags & sfWholeWords) != 0);
  config->writeEntry("FromCursor",(flags & sfFromCursor) != 0);
  config->writeEntry("FindBackwards",(flags & sfBackward) != 0);
  config->writeEntry("SelectedText",(flags & sfSelected) != 0);
  config->writeEntry("PromptOnReplace",(flags & sfPrompt) != 0);

  config->setGroup("Settings");
  flags = configFlags;
  config->writeEntry("AutoIndent",(flags & cfAutoIndent) != 0);
  config->writeEntry("BackspaceIndent",(flags & cfBackspaceIndent) != 0);
  config->writeEntry("WordWrap",(flags & cfWordWrap) != 0);
  config->writeEntry("ReplaceTabs",(flags & cfReplaceTabs) != 0);
  config->writeEntry("RemoveTrailingSpaces",(flags & cfRemoveSpaces) != 0);
  config->writeEntry("WrapCursor",(flags & cfWrapCursor) != 0);
  config->writeEntry("AutoBrackets",(flags & cfAutoBrackets) != 0);
  config->writeEntry("PersistentSelections",(flags & cfPersistent) != 0);
  config->writeEntry("MultipleSelections",(flags & cfKeepSelection) != 0);
  config->writeEntry("VerticalSelections",(flags & cfVerticalSelect) != 0);
  config->writeEntry("DeleteOnInput",(flags & cfDelOnInput) != 0);
  config->writeEntry("ToggleOld",(flags & cfXorSelect) != 0);

  config->writeEntry("WrapAt",wrapAt);
  config->writeEntry("TabWidth",kWriteDoc->tabChars);
}

void KWrite::readSessionConfig(KConfig *config) {
  searchFlags = config->readNumEntry("SearchFlags",sfPrompt);
  configFlags = config->readNumEntry("ConfigFlags");
  wrapAt = config->readNumEntry("WrapAt",78);
}
void KWrite::writeSessionConfig(KConfig *config) {
  config->writeEntry("SearchFlags",searchFlags);
  config->writeEntry("ConfigFlags",configFlags);
  config->writeEntry("WrapAt",wrapAt);
}


void KWrite::setHighlight(Highlight *hl) {
  if (hl) {
    kWriteDoc->setHighlight(hl);
//    kWriteDoc->tagAll();
    kWriteDoc->updateViews();
  }
}

void KWrite::settings() {
  SettingsDialog *dlg;

  dlg = new SettingsDialog(configFlags,wrapAt,kWriteDoc->tabChars,topLevelWidget());
  if (dlg->exec() == QDialog::Accepted) {
    configFlags = dlg->getFlags() | (configFlags & cfOvr);
    wrapAt = dlg->getWrapAt();
    kWriteDoc->setTabWidth(dlg->getTabWidth());
  }
  delete dlg;
}


void KWrite::paintEvent(QPaintEvent *e) {
  int x, y;

  QRect updateR = e->rect();                    // update rectangle
//  printf("Update rect = ( %i, %i, %i, %i )\n",
//    updateR.x(),updateR.y(), updateR.width(), updateR.height() );

  int ux1 = updateR.x();
  int uy1 = updateR.y();
  int ux2 = ux1 + updateR.width();
  int uy2 = uy1 + updateR.height();

  QPainter paint;
  paint.begin(this);

  QColorGroup g = colorGroup();
  x = width();
  y = height();

  paint.setPen(g.dark());
  if (uy1 <= 0) paint.drawLine(0,0,x-2,0);
  if (ux1 <= 0) paint.drawLine(0,1,0,y-2);

  paint.setPen(black);
  if (uy1 <= 1) paint.drawLine(1,1,x-3,1);
  if (ux1 <= 1) paint.drawLine(1,2,1,y-3);

  paint.setPen(g.midlight());
  if (uy2 >= y-1) paint.drawLine(1,y-2,x-3,y-2);
  if (ux2 >= x-1) paint.drawLine(x-2,1,x-2,y-2);

  paint.setPen(g.light());
  if (uy2 >= y) paint.drawLine(0,y-1,x-2,y-1);
  if (ux2 >= x) paint.drawLine(x-1,0,x-1,y-1);

  x -= 2 + 16;
  y -= 2 + 16;
  if (ux2 > x && uy2 > y) {
    paint.fillRect(x,y,16,16,g.background());
  }
  paint.end();
}

void KWrite::resizeEvent(QResizeEvent *e) {

  printf("Resize %d, %d\n",e->size().width(),e->size().height());

  kWriteView->tagAll();//updateState = 3;
  kWriteView->updateView(ufNoScroll);
}



