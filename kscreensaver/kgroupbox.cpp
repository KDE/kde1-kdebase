#include "kgroupbox.h"
#include <qtimer.h>

#define DEFAULT_BORDER  6

KGroupBox::KGroupBox(QWidget *parent, const char *name) : 
  QGroupBox(parent, name)
{
  contents = new QWidget(this);
}

KGroupBox::KGroupBox(const char *title, QWidget *parent, const char *name) :
  QGroupBox(title, parent, name)
{
  contents = new QWidget(this);
}

void KGroupBox::setTitle(const char *c) {
  QGroupBox::setTitle(c);
  updateRects();
}

QWidget *KGroupBox::inner() {
  return contents;
}

QSize KGroupBox::sizeHint() const {
  return sh;
}

void KGroupBox::setFont(QFont &f) {
  QGroupBox::setFont(f);
  updateRects();
}

void KGroupBox::resizeEvent(QResizeEvent *) {
  updateRects();
}

void KGroupBox::updateRects() {
  QRect r(rect());
  QRect r1;

  if(title() == 0) {
    r1.setCoords(r.left() + DEFAULT_BORDER, r.top() + DEFAULT_BORDER,
		 r.right() - DEFAULT_BORDER, r.bottom() - DEFAULT_BORDER);
    sh = contents->minimumSize();
    sh.setWidth(sh.width() + 2 * DEFAULT_BORDER);
    sh.setHeight(sh.height() + 2 * DEFAULT_BORDER);
  } else {
    r1.setCoords(r.left() + DEFAULT_BORDER, r.top() + fontMetrics().height() + 4,
		 r.right() - DEFAULT_BORDER, r.bottom() - DEFAULT_BORDER);
    sh = contents->minimumSize();
    sh.setWidth(sh.width() + 2 * DEFAULT_BORDER);
    sh.setHeight(sh.height() + DEFAULT_BORDER + fontMetrics().height() + 4);
  }

  contents->setGeometry(r1);
  setMinimumSize(sh);
}

void KGroupBox::activate() {
  updateRects();
}

#include "kgroupbox.moc"
