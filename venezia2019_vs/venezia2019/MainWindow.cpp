#include "MainWindow.h"

#include "PDiskPage.h"
#include "FoldingFacesPage.h"
#include "CircleLimit3Page.h"
#include "TestPage.h"
#include "H3GridPage.h"
#include "PolydronPage.h"
#include "CubeGridPage.h"


#include <QKeyEvent>
#include <QGraphicsView>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsItem>
#include <QDebug>

#include <map>

class Slide : public QGraphicsView
{
  QGraphicsScene m_scene;
  int m_status;
  std::map<int, std::pair<int, int> > m_acts;
  std::vector<QGraphicsItem*> m_items;

public:
  Slide(QString imageName = "") : QGraphicsView(new QGraphicsScene()), m_status(0)
  {
    setRenderHints(
      QPainter::HighQualityAntialiasing
      | QPainter::SmoothPixmapTransform
      | QPainter::TextAntialiasing);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    scene()->setBackgroundBrush(Qt::black);
    if(imageName != "")
    {
      addImage(imageName,0,0);
    }
    setFocusPolicy(Qt::ClickFocus);
    setFrameShape(QFrame::NoFrame);
  }

  void showEvent(QShowEvent*) {setFocus();}

  void addItem(QGraphicsItem*item) {
    m_items.push_back(item);
  }

  void setAct(int id, int s0, int s1) {
    m_acts[id] = std::make_pair(s0,s1);
    if(0<s0) m_items[id]->hide();
  }
  void setStatus(int status) {
    if(status<0) status=0;
    if(m_status == status) return;
    m_status = status;
    std::map<int, std::pair<int, int> >::iterator it;
    for(it = m_acts.begin(); it != m_acts.end(); ++it)
    {
      if(it->second.first<=m_status && m_status<=it->second.second)
        m_items[it->first]->show();
      else
        m_items[it->first]->hide();
    }
    update();
  }

  QGraphicsSimpleTextItem *addText(QString text, const QFont &font, QColor color, double x, double y) {
    QGraphicsSimpleTextItem *item = scene()->addSimpleText(text, font);
    item->setPos(x,y);
    item->setBrush(color);
    addItem(item);
    return item;
  }
  QGraphicsPixmapItem *addImage(QString fn, double x, double y, bool fit = false) {
    QPixmap pixmap(fn);
    // titleImagePixmap = titleImagePixmap.scaledToHeight(300,Qt::SmoothTransformation);
    QGraphicsPixmapItem *item = scene()->addPixmap(pixmap);
    if(fit)
    {
        double sx = 1024.0/(double)pixmap.width();
        double sy = 768.0/(double)pixmap.height();
        double sc = sx<sy?sx:sy;
        item->setScale(sc);
    }
    item->setPos(x,y);
    addItem(item);
    return item;
  }

  void keyPressEvent ( QKeyEvent *e) {
    if(e->key() == Qt::Key_Left) setStatus(m_status-1);
    else if(e->key() == Qt::Key_Right) setStatus(m_status+1);
    else
      e->ignore();
  }
};


void addConference(MainWindow *w)
{
  /*
  w->addWidget(new PDiskPage());
  w->addWidget(new CircleLimit3Page());

  return;
  */

  QFont h1("Arial", 40, QFont::Bold);
  QFont h2("Arial", 20);

  
  Slide *slide;
  w->addWidget(new CubeGridPage());
  w->addWidget(new PolydronPage());
  /*
  w->addWidget(new TestPage());
  */
 
  // w->addWidget(new PDiskPage());
  // w->addWidget(new FoldingFacesPage());
  // w->addWidget(new CircleLimit3Page());
  w->addWidget(new H3GridPage());
 
}

MainWindow::MainWindow()
: QStackedWidget(0)
{
  // setWindowFlags(windowFlags() | Qt::X11BypassWindowManagerHint);
  setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

  QGLFormat format;
  format.setSampleBuffers(true);
  format.setDefaultFormat(format);
  
  // QGraphicsItem *item;
  setFrameShape(QFrame::NoFrame);
  

  addConference(this);

  resize(sizeHint());
}

MainWindow::~MainWindow()
{
}


QSize MainWindow::sizeHint () const
{
  return QSize(1024,768);
//  return QSize(800,800);
//  return QSize(1000,1000);
}

int timerId = -1;

void MainWindow::keyPressEvent(QKeyEvent *e)
{
  if(e->key() == Qt::Key_PageDown)
  {
    if(currentIndex()+1<count()) setCurrentIndex(currentIndex()+1);
  }
  else if(e->key() == Qt::Key_PageUp)
  {
    if(currentIndex()>0) setCurrentIndex(currentIndex()-1);
  }
  else if(e->key() == Qt::Key_Escape)
  {
    close();
  }
  else if(e->key() == Qt::Key_F11)
  {
    if(isFullScreen())
      setWindowState(windowState() & ~Qt::WindowFullScreen);
    else
      setWindowState(windowState() | Qt::WindowFullScreen);
    setFocus();
  }
  else
    QStackedWidget::keyPressEvent(e);
}


void MainWindow::timerEvent(QTimerEvent *)
{
  if(timerId>=0)
  {
    killTimer(timerId);
    timerId = -1;
  }
  showNormal();
}

