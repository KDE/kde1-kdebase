/* This is the default widget for kcc
   Author: Markus Wuebben
	   <markus.wuebben@kde.org>
   Date:   September '97         */
   
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <kiconloader.h>
#include <kcharsets.h>

#include "mainwidget.moc"
#include "mainwidget.h"

mainWidget::mainWidget(QWidget *parent , const char *name)
  : QWidget(parent, name)
{
  KIconLoader iconLoader;

  QLabel *heading = new QLabel(klocale->translate("KDE Control Center"),this);
  QFont font("times",18,QFont::Bold);
  KApplication::getKApplication()->getCharsets()->setQFont(font);
  pmap = iconLoader.loadIcon("kdekcc.xpm");
  heading->setFont(font);
  heading->adjustSize();
  heading->move(120,10);

  uname(&info);
}


void mainWidget::paintEvent(QPaintEvent *)
{
  QString str;
  char buf[512];
  QPainter p(this);
  
  QFont normalFont("times",12,QFont::Normal);
  KApplication::getKApplication()->getCharsets()->setQFont(normalFont);
  QFont boldFont("times",12,QFont::Bold);
  KApplication::getKApplication()->getCharsets()->setQFont(boldFont);

  // center the pixmap horizontally
  p.drawPixmap( (width() - pmap.width())/2, 250, pmap);
  str = i18n("KDE Version: ");
  p.setFont(boldFont);
  p.drawText(60,70,str);
  str.sprintf("%s",KDE_VERSION_STRING);
  p.setFont(normalFont);
  p.drawText(180,70,str);

  p.setFont(boldFont);
  str= i18n("User: ");
  p.drawText(60,90,str);
  str.sprintf("%s",getlogin());
  p.setFont(normalFont);
  p.drawText(180,90,str);

  str = i18n("Hostname: ");
  p.setFont(boldFont);
  p.drawText(60,110,str);
  gethostname(buf,511);
  str.sprintf("%s",buf);
  p.setFont(normalFont);
  p.drawText(180,110,str);

  str = i18n("System: ");
  p.setFont(boldFont);
  p.drawText(60,130,str);
  str.sprintf("%s",info.sysname);
  p.setFont(normalFont);
  p.drawText(180,130,str);
   
  str = i18n("Release: ");
  p.setFont(boldFont);
  p.drawText(60,150,str);
  str.sprintf("%s",info.release);
  p.setFont(normalFont);
  p.drawText(180,150,str);


  str = i18n("Version: ");
  p.setFont(boldFont);
  p.drawText(60,170,str);
  str.sprintf("%s",info.version);
  p.setFont(normalFont);
  p.drawText(180,170,str);

  str = i18n("Machine: ");
  p.setFont(boldFont);
  p.drawText(60,190,str);
  str.sprintf("%s",info.machine);
  p.setFont(normalFont);
  p.drawText(180,190,str);

  p.end();

}


void mainWidget::resizeEvent(QResizeEvent *event)
{
  QWidget::resizeEvent(event);

  emit resized();
}
