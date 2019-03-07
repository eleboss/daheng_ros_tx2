//--------------------------------------------------------------------------------
/**
\file          ChildWindow.cpp
\brief       ChildWindow Function
\version   v1.0.1603.9011
\date        2016-03-01
*/
//---------------------------------------------------------------------------------

#include "ChildWindow.h"
#include <QPainter>
#include <qpainter.h>
#include <QPushButton>
#include <QLabel>
#include "UIDef.h"

#define TIMER_INTERVAL  4000

//----------------------------------------------------------------------------------
/**
\brief Constructor
\input
\output
\return
*/
//------------------------------------------------------------------------------------
CChildWindow::CChildWindow(QWidget* parent)
    :QWidget(parent)
    ,label_view(NULL)
    ,m_view_flag(false)
    ,m_label_object(NULL)
    ,m_image(NULL)
    ,m_device_name("")
    ,m_fps_info("")
    ,title_timer(NULL)
{
    //whether the Label has been created
    if(m_label_object == NULL)
    {
         m_label_object = new QLabel(this);
    }

    title_timer = new QTimer(this);
    connect(title_timer, SIGNAL(timeout()), this, SLOT(DisTitleInfo()));
    title_timer->start(TIMER_INTERVAL);

    //The displayed image size is automatically adjusted to the label size
    m_label_object->setScaledContents(true);
    m_display_width = this->parentWidget()->size().width();
    m_display_height = this->parentWidget()->size().height();

    m_objFps.Reset();
    m_objFps.ResetFPS();
}

//----------------------------------------------------------------------------------
/**
\brief Destructor
\input
\output
\return
*/
//------------------------------------------------------------------------------------
CChildWindow::~CChildWindow()
{
        if(m_label_object != NULL)
        {
            m_label_object->deleteLater();
            m_label_object = NULL;
        }

        if(label_view != NULL)
        {
            label_view->deleteLater();
            label_view = NULL;
        }

        if(title_timer->isActive())
        {
            title_timer->stop();
            title_timer->deleteLater();
            title_timer = NULL;
        }
}

//----------------------------------------------------------------------------------
/**
\brief  The function demonstrates how to display image. 
\input
\output
\return void
*/
//------------------------------------------------------------------------------------
void CChildWindow::ShowImage(QImage *image, QString device_name, QString display_fps, bool view_id)
{
    m_objFps.IncreaseFrameNum();
    m_image = image;
    m_device_name = device_name;
    m_fps_info = display_fps;
    m_view_flag = view_id;
}


void CChildWindow::paintEvent(QPaintEvent *event)
{
    this->_PaintImage(event);
}


void CChildWindow::_PaintImage(QPaintEvent* /*event*/)
{

    //whether the label has been created
     if(m_label_object == NULL)
     {
         m_label_object = new QLabel(this);
     }

     //Set the label size to the widget size
     if((this->parentWidget()->size().height() != m_display_height) ||
             (this->parentWidget()->size().width() != m_display_width))
     {
         m_label_object->resize(this->parentWidget()->size());
         m_display_height = this->parentWidget()->size().height();
         m_display_width = this->parentWidget()->size().width();
     }

     if(m_image != NULL)
     {
         //Display the image
         m_label_object->setPixmap(QPixmap::fromImage(*m_image));
         m_label_object->show();
     }
}

//----------------------------------------------------------------------------------
/**
\brief The slot function demonstrates how to display the snap information.
\input
\output
\return void
*/
//------------------------------------------------------------------------------------
void CChildWindow::DisTitleInfo()
{
        if(m_objFps.GetTotalFrameCount() >= 1)
        {
            printf("%s, %s\n", m_device_name.toStdString().c_str() ,  m_fps_info.toStdString().c_str());
        }
}
