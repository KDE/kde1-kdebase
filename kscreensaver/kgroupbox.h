#ifndef __KGROUPBOX__H__
#define __KGROUPBOX__H__

#include <qgrpbox.h>
#include <qwidget.h>

class KGroupBox : public QGroupBox {
  Q_OBJECT
public:
  KGroupBox(QWidget *parent = 0, const char *name = 0);
  KGroupBox(const char *title, QWidget *parent = 0, const char *name=0);
  
  void setTitle(const char *);
  QWidget *inner();

  virtual QSize sizeHint() const;
  virtual void setFont(QFont &);
  virtual void resizeEvent(QResizeEvent *);

  void activate();

protected:
  virtual void updateRects();

  QWidget *contents;
  QSize sh;
};

#endif
