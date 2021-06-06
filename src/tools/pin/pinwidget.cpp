// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "pinwidget.h"
#include "src/utils/confighandler.h"
#include <QApplication>
#include <QLabel>
#include <QShortcut>
#include <QVBoxLayout>
#include <QWheelEvent>

PinWidget::PinWidget(const QPixmap& pixmap, QWidget* parent)
  : QWidget(parent)
  , m_pixmap(pixmap)
{
    setWindowIcon(QIcon(":img/app/flameshot.svg"));
    setWindowFlags(Qt::WindowStaysOnTopHint |  Qt::Tool | Qt::FramelessWindowHint);
    // set the bottom widget background transparent
    setAttribute(Qt::WA_TranslucentBackground);
    this->setMouseTracking(true);

    ConfigHandler conf;
    m_baseColor = conf.uiMainColorValue();
    m_hoverColor = conf.uiContrastColorValue();

    m_layout = new QVBoxLayout(this);
    const int margin = this->margin();
    m_layout->setContentsMargins(margin, margin, margin, margin);

    m_shadowEffect = new QGraphicsDropShadowEffect(this);
    m_shadowEffect->setColor(m_baseColor);
    m_shadowEffect->setBlurRadius(2 * margin);
    m_shadowEffect->setOffset(0, 0);
    setGraphicsEffect(m_shadowEffect);

    m_label = new QLabel();
    m_label->setPixmap(m_pixmap);
    m_layout->addWidget(m_label);

    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this, SLOT(close()));
    new QShortcut(Qt::Key_Escape, this, SLOT(close()));
}

int PinWidget::margin() const
{
    return 7;
}

void PinWidget::wheelEvent(QWheelEvent* e)
{
    int val = e->angleDelta().y() > 0 ? 15 : -15;
    int newWidth = qBound(50, m_label->width() + val, maximumWidth());
    int newHeight = qBound(50, m_label->height() + val, maximumHeight());

    QSize size(newWidth, newHeight);
    setScaledPixmap(size);
    adjustSize();

    e->accept();
}

void PinWidget::enterEvent(QEvent*)
{
    m_shadowEffect->setColor(m_hoverColor);
}
void PinWidget::leaveEvent(QEvent*)
{
    m_shadowEffect->setColor(m_baseColor);
}

void PinWidget::mouseDoubleClickEvent(QMouseEvent*)
{
    close();
}

// void PinWidget::mousePressEvent(QMouseEvent* e)
// {
//     m_dragStart = e->globalPos();
//     m_offsetX = e->localPos().x() / width();
//     m_offsetY = e->localPos().y() / height();
// }

// void PinWidget::mouseMoveEvent(QMouseEvent* e)
// {
//     const QPoint delta = e->globalPos() - m_dragStart;
//     int offsetW = width() * m_offsetX;
//     int offsetH = height() * m_offsetY;
//     move(m_dragStart.x() + delta.x() - offsetW,
//          m_dragStart.y() + delta.y() - offsetH);
// }

void PinWidget::setScaledPixmap(const QSize& size)
{
    const qreal scale = qApp->devicePixelRatio();
    QPixmap scaledPixmap = m_pixmap.scaled(
      size * scale, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    scaledPixmap.setDevicePixelRatio(scale);
    m_label->setPixmap(scaledPixmap);
}

void PinWidget::region(const QPoint &currentGlobalPoint)
{
    // 获取窗体在屏幕上的位置区域，topLeft为坐上角点，rightButton为右下角点
    QRect rect = this->rect();

    QPoint topLeft = this->mapToGlobal(rect.topLeft()); //将左上角的(0,0)转化为全局坐标
    QPoint rightButton = this->mapToGlobal(rect.bottomRight());

    int x = currentGlobalPoint.x(); //当前鼠标的坐标
    int y = currentGlobalPoint.y();

    if(((topLeft.x() + PADDING >= x) && (topLeft.x() <= x))
            && ((topLeft.y() + PADDING >= y) && (topLeft.y() <= y)))
    {
        // 左上角
        dir = LEFTTOP;
        this->setCursor(QCursor(Qt::SizeFDiagCursor));  // 设置光标形状
    }else if(((x >= rightButton.x() - PADDING) && (x <= rightButton.x()))
              && ((y >= rightButton.y() - PADDING) && (y <= rightButton.y())))
    {
        // 右下角
        dir = RIGHTBOTTOM;
        this->setCursor(QCursor(Qt::SizeFDiagCursor));
    }else if(((x <= topLeft.x() + PADDING) && (x >= topLeft.x()))
              && ((y >= rightButton.y() - PADDING) && (y <= rightButton.y())))
    {
        //左下角
        dir = LEFTBOTTOM;
        this->setCursor(QCursor(Qt::SizeBDiagCursor));
    }else if(((x <= rightButton.x()) && (x >= rightButton.x() - PADDING))
              && ((y >= topLeft.y()) && (y <= topLeft.y() + PADDING)))
    {
        // 右上角
        dir = RIGHTTOP;
        this->setCursor(QCursor(Qt::SizeBDiagCursor));
    }else if((x <= topLeft.x() + PADDING) && (x >= topLeft.x()))
    {
        // 左边
        dir = LEFT;
        this->setCursor(QCursor(Qt::SizeHorCursor));
    }else if((x <= rightButton.x()) && (x >= rightButton.x() - PADDING))
    {
        // 右边
        dir = RIGHT;
        this->setCursor(QCursor(Qt::SizeHorCursor));
    }else if((y >= topLeft.y()) && (y <= topLeft.y() + PADDING))
    {
        // 上边
        dir = UP;
        this->setCursor(QCursor(Qt::SizeVerCursor));
    }else if((y <= rightButton.y()) && (y >= rightButton.y() - PADDING))
    {
        // 下边
        dir = DOWN;
        this->setCursor(QCursor(Qt::SizeVerCursor));
    }else
    {
        // 默认
        dir = NONE;
        this->setCursor(QCursor(Qt::ArrowCursor));
    }
}


//三个鼠标事件的重写
//鼠标按下事件
void PinWidget::mousePressEvent(QMouseEvent *event)
{
    switch(event->button())
    {
        case Qt::LeftButton:
            isLeftPressDown = true;

            if(dir != NONE)
            {
                this->mouseGrabber(); //返回当前抓取鼠标输入的窗口
            }
            else
            {
                m_movePoint = event->globalPos() - this->frameGeometry().topLeft();
                //globalPos()鼠标位置，topLeft()窗口左上角的位置
            }
            break;
        case Qt::RightButton:
            this->setWindowState(Qt::WindowMinimized);
            break;
        default:
            QWidget::mousePressEvent(event);
    }
}



//鼠标移动事件
void PinWidget::mouseMoveEvent(QMouseEvent *event)
{
    QPoint globalPoint = event->globalPos();   //鼠标全局坐标

    QRect rect = this->rect();  //rect == QRect(0,0 1280x720)
    QPoint topLeft = mapToGlobal(rect.topLeft());
    QPoint bottomRight = mapToGlobal(rect.bottomRight());

    if (this->windowState() != Qt::WindowMaximized)
    {
        if(!isLeftPressDown)  //没有按下左键时
        {
            this->region(globalPoint); //窗口大小的改变——判断鼠标位置，改变光标形状
        }
        else
        {
            if(dir != NONE)
            {
                QRect newRect(topLeft, bottomRight); //定义一个矩形  拖动后最大1000*1618
                switch(dir)
                {
                    case LEFT:
                        if(bottomRight.x() - globalPoint.x() <= this->minimumWidth())
                        {
                            newRect.setLeft(topLeft.x());  //小于界面的最小宽度时，设置为左上角横坐标为窗口x
                            //只改变左边界
                        }
                        else
                        {
                            newRect.setLeft(globalPoint.x());
                        }
                        break;
                    case RIGHT:
                        newRect.setWidth(globalPoint.x() - topLeft.x());  //只能改变右边界
                        break;
                    case UP:
                        if(bottomRight.y() - globalPoint.y() <= this->minimumHeight())
                        {
                            newRect.setY(topLeft.y());
                        }
                        else
                        {
                            newRect.setY(globalPoint.y());
                        }
                        break;
                    case DOWN:
                        newRect.setHeight(globalPoint.y() - topLeft.y());
                        break;
                    case LEFTTOP:
                        if(bottomRight.x() - globalPoint.x() <= this->minimumWidth())
                        {
                            newRect.setX(topLeft.x());
                        }
                        else
                        {
                            newRect.setX(globalPoint.x());
                        }

                        if(bottomRight.y() - globalPoint.y() <= this->minimumHeight())
                        {
                            newRect.setY(topLeft.y());
                        }
                        else
                        {
                            newRect.setY(globalPoint.y());
                        }
                        break;
                     case RIGHTTOP:
                          if (globalPoint.x() - topLeft.x() >= this->minimumWidth())
                          {
                              newRect.setWidth(globalPoint.x() - topLeft.x());
                          }
                          else
                          {
                              newRect.setWidth(bottomRight.x() - topLeft.x());
                          }
                          if (bottomRight.y() - globalPoint.y() >= this->minimumHeight())
                          {
                              newRect.setY(globalPoint.y());
                          }
                          else
                          {
                              newRect.setY(topLeft.y());
                          }
                          break;
                     case LEFTBOTTOM:
                          if (bottomRight.x() - globalPoint.x() >= this->minimumWidth())
                          {
                              newRect.setX(globalPoint.x());
                          }
                          else
                          {
                              newRect.setX(topLeft.x());
                          }
                          if (globalPoint.y() - topLeft.y() >= this->minimumHeight())
                          {
                              newRect.setHeight(globalPoint.y() - topLeft.y());
                          }
                          else
                          {
                              newRect.setHeight(bottomRight.y() - topLeft.y());
                          }
                          break;
                      case RIGHTBOTTOM:
                          newRect.setWidth(globalPoint.x() - topLeft.x());
                          newRect.setHeight(globalPoint.y() - topLeft.y());
                          break;
                      default:
                          break;
                }

                QSize size(newRect.width(), newRect.height());
                setScaledPixmap(size);
                adjustSize();
                event->accept();
                newRect.setWidth(m_label->pixmap()->width());
                newRect.setHeight(m_label->pixmap()->height());
                this->setGeometry(newRect);
            }
            else
            {
                move(event->globalPos() - m_movePoint); //移动窗口
                event->accept();
            }
        }
    }
}


//鼠标释放事件
void PinWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        isLeftPressDown = false;
        if (dir != NONE)
        {
            this->releaseMouse(); //释放鼠标抓取
            this->setCursor(QCursor(Qt::ArrowCursor));
            dir = NONE;
        }
    }
}


