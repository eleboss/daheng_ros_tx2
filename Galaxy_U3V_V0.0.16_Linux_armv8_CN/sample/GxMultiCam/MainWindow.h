//---------------------------------------------------------------------------------------
/**
\file          MainWindow.h
\brief       Declarations for MainWindow Class
\version   v1.0.1603.9011
\date        2016-03-01
*/
//---------------------------------------------------------------------------------------

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QTimer>
#include <QTime>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMutex>
#include "unistd.h"
#include "ChildWindow.h"
#include "ParameterDialog.h"
#include "SelectCameraDialog.h"
#include "AboutDialog.h"
#include "GxIAPI.h"

namespace Ui {
class MainWindow;
}

class CSelectCameraDialog;

/// Camera parameter structure
typedef struct CAMER_INFO
{
    QImage               *show_image;                 ///< the pointer of image object for display
    void                     *result_image;                ///< the pointer of image data for display
    int64_t                 payload_size;                 ///< image size
    int64_t		        image_width;	              ///< image width
    int64_t	                image_height;	              ///< image height
    int64_t			bayer_layout;	              ///< Bayer format
    bool			color_filter_flag;	      ///< Whether Color image
    bool			open_flag;		      ///< Whether camera being opened
    bool			snap_flag;		              ///< whether the camera is acquisiting
    bool                     chose_flag;                     ///< Whether the camera has been selected
    float			fps;			              ///< frame rate
}CAMER_INFO;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    /// Constructor
    explicit MainWindow(QWidget *parent = 0);

    /// Destructor
    ~MainWindow();
    
    /// Error message
    void ShowErrorString(GX_STATUS error_status);

    /// Set the camera's data format to 8bit
    GX_STATUS SetPixelFormat8bit(GX_DEV_HANDLE device_handle);

    ///  Register the callback function
    void RegisterCallback(int camera_id);

    /// First camera callback function
    static void  OnFrameCallbackFun1(GX_FRAME_CALLBACK_PARAM* frame);

    /// Second camera callback function
    static void OnFrameCallbackFun2(GX_FRAME_CALLBACK_PARAM* framee);

    /// Third camera callback function
    static void  OnFrameCallbackFun3(GX_FRAME_CALLBACK_PARAM* frame);

    /// Fourth camera callback function
    static void  OnFrameCallbackFun4(GX_FRAME_CALLBACK_PARAM* frame);

    /// Refresh the status of the menu bar
    void UpdateUI(void);

private:
    /// Initialize the function
    bool __Init();

    /// Create action
    bool __CreateActions();

    /// Create a menu
    bool __CreateMenu();

    /// Set the icon for the sample program
    void __SetSystemIcon(void);

    /// Window layout function
    void __Layout();

    /// Open the device function
    void __OpenDevice();

    /// Close the device function
    void __CloseDevice();

    /// Start acquisition functions
    void __StartAcquisition();

    /// Start stop functions
    void __StopAcquisition();

public slots:
    /// The slot function connected to the action of selected the camera
    void SlotSelectDevice(void);

    /// The slot fuction connected to the action of opening device
    void SlotOpenDevice(void);

    /// The slot fuction connected to the action of closing device
    void SlotCloseDevice(void);

    /// The slot fuction connected to the action of starting acquisition
    void SlotStartAcquisition(void);

    /// The slot fuction connected to the action of stoping acquisition
    void SlotStopAcquisition(void);

    /// The slot function connected to the action of setting the parameter
    void SlotOnParameterDialog(void);

    /// The slot function connected to the action of dispalaying the program information
    void SlotAboutInfo(void);

protected:
    /// Timed events
    void timerEvent(QTimerEvent *event);

public:
    Ui::MainWindow *               ui;
    QMdiArea*                          m_mdiarea;                                                               ///< Sub-window manager
    QMdiSubWindow *              m_mdiarea_sub_window[DEVICE_MAX_NUM];       ///< sub-window pointer
    CChildWindow *                  m_child_window[DEVICE_MAX_NUM];                   ///< Sub-window object pointer
    CParameterDialog*              m_parameter_dialog;                                               ///< Configuration parameter window
    CAboutDialog*                     m_about_dialog;                                                      ///< Program information dialog box
    GX_DEVICE_BASE_INFO *  m_baseinfo;                                                             ///< Device information structure pointer

    GX_FRAME_CALLBACK_PARAM* m_frame_first;                                                ///< The parameter pointer of first camera callback function
    GX_FRAME_CALLBACK_PARAM* m_frame_second;                                           ///< The parameter pointer of second camera callback function
    GX_FRAME_CALLBACK_PARAM* m_frame_third;                                              ///< The parameter pointer of third camera callback function
    GX_FRAME_CALLBACK_PARAM* m_frame_fouth;                                             ///< The parameter pointer of fourth camera callback function

    QMenu*                                m_camera;                                                             ///< Camera menu
    QAction*                               m_select_camera;                                                  ///< The action connected to select camera
    QAction*                               m_action_open_device;                                          ///< The action connected to open device
    QAction*                               m_action_close_device;                                         ///< The action connected to close device
    QAction*                               m_action_start_acq;                                               ///< The action connected to start acquisition
    QAction*                               m_action_stop_acq;                                               ///< The action connected to stop acquisition

    QMenu*                                m_parameter;                                                        ///< The action connected to parameter menu
    QAction*                               m_action_parameter_dialog;                                  ///< The action connected to parameter settings

    QMenu*                                m_help;                                                                  ///< The action connected to help menu
    QAction*                               m_action_about_info;                                             ///< The action connected to about program information

     float                                     m_show_image_count;                                           ///< Display frame statistics
     float                                     m_image_rate;                                                       ///< Display frame rate

    GX_DEV_HANDLE*	         m_devices_handle;	                                               ///< camera handle pointer
    int			                 m_operate_id;		                                               ///< camera ID

    QTimer*                                m_timer;                                                                ///< Timers
    QTime                                   m_object_time;                                                      ///< Timer

    uint32_t		                 m_camera_number;			                        ///< Number of cameras
    CAMER_INFO*	                 m_struct_camera;			                                ///< Camera data structure
    bool			                 m_view_flag;			                                        ///< Whether the camera is displaying
    bool                                      m_display_flag[DEVICE_MAX_NUM];                    ///< The flag for display image
};

#endif // MAINWINDOW_H

