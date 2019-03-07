//---------------------------------------------------------------------------------------
/**
\file          ChildWindow.h
\brief       Declarations for CChildWindow Class
\version   v1.0.1603.9011
\date        2016-03-01
*/
//---------------------------------------------------------------------------------------

#ifndef CHILDWINDOW_H
#define CHILDWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QTime>
#include <QTimer>
#include <QGridLayout>
#include "Fps.h"

#define DEVICE_MAX_NUM 4

class CChildWindow: public QWidget
{
    Q_OBJECT

public:
    /// Constructor
    explicit CChildWindow(QWidget* parent);

    /// Destructor
    virtual ~CChildWindow();

    /// Show Image
    void ShowImage(QImage *image, QString device_name, QString display_fps, bool view_id);
protected:

    /// The function of drawing image event
    void paintEvent(QPaintEvent *event);

    /// Draw image
    void _PaintImage(QPaintEvent *event);

public slots:
    /// The slot function connected to displaying the information
    void DisTitleInfo();

private:
    QGridLayout *label_view;                  ///< Display camera frame rate flag
    bool              m_view_flag;                 ///< Whether the frame rate is displayed
    QLabel          *m_label_object;           ///< Used to display the image controls
    QLabel          *m_label_title;              ///< Used to display information controls
    QImage         *m_image;                    ///< Display the image object pointer
    QString         m_device_name;           ///< Camera type
    QString         m_fps_info;                   ///< Camera serial number and frame rate
    int                 m_display_width;          ///< display the image width 
    int                 m_display_height;         ///< display the image height
    int                 m_title_count;               ///< Show title count
    QTimer         *title_timer;                   ///< Timers

public:
     // int               m_fps_count;                 ///<  Display frame rate count
    CFps                  m_objFps;

};

#endif // CHILDWINDOW_H
