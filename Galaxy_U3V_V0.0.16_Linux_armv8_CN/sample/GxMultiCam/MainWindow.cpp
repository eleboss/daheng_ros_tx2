//--------------------------------------------------------------------------------
/**
\file          MainWindow.cpp
\brief       MainWindow Function
\version   v1.0.1603.9011
\date        2016-03-01
*/
//---------------------------------------------------------------------------------

#include <QMessageBox>
#include <QMenuBar>
#include <QSplitter>
#include <QTextEdit>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QGridLayout>
#include <QGraphicsDropShadowEffect>
#include <new>
#include "MainWindow.h"
#include "ParameterDialog.h"
#include "SelectCameraDialog.h"
#include "ChildWindow.h"
#include <qpainter.h>
#include "ui_mainwindow.h"
#include "UIDef.h"
#include "GxIAPI.h"
#include "DxImageProc.h"
#include <stdio.h>



//----------------------------------------------------------------------------------
/**
\brief Constructor
\input
\output
\return
*/
//------------------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
  ,ui(new Ui::MainWindow)
  ,m_mdiarea(NULL)
  ,m_parameter_dialog(NULL)
  ,m_about_dialog(NULL)
  ,m_baseinfo(NULL)
  ,m_frame_first(NULL)
  ,m_frame_second(NULL)
  ,m_frame_third(NULL)
  ,m_frame_fouth(NULL)
  ,m_camera(NULL)
  ,m_select_camera(NULL)
  ,m_action_open_device(NULL)
  ,m_action_close_device(NULL)
  ,m_action_start_acq(NULL)
  ,m_action_stop_acq(NULL)
  ,m_parameter(NULL)
  ,m_action_parameter_dialog(NULL)
  ,m_show_image_count(0.0)
  ,m_image_rate(0.0)
  ,m_devices_handle(NULL)
  ,m_operate_id(-1)
  ,m_timer(NULL)
  ,m_camera_number(0)
  ,m_struct_camera(NULL)
  ,m_view_flag(false)
{
    for(uint32_t i = 0; i < 4; i++)
    {
        m_mdiarea_sub_window[i] = NULL;
        m_child_window[i] = NULL;
        m_display_flag[i] = false;
    }

    bool result = false;

    ui->setupUi(this);

    //Initialize the camera library.
    GX_STATUS status = GX_STATUS_ERROR;
    status = GXInitLib();
    if (status != GX_STATUS_SUCCESS)
    {
        ShowErrorString(status);
        exit(0);
    }

    //initialization
    this->__SetSystemIcon();
    result = this->__Init();
    if(result == false)
    {
        QMessageBox::about(NULL, "Error", "Initialize fail !");
        exit(0);
    }
}

//----------------------------------------------------------------------------------
/**
\brief Destructor
\input
\output
\return
*/
//------------------------------------------------------------------------------------

MainWindow::~MainWindow()
{
    GX_STATUS status = GX_STATUS_ERROR;

    //In order to prevent errors when closing camera
    for(uint32_t i = 0; i < m_camera_number; i++)
    {
        if(m_struct_camera != NULL)
        {
              if (m_struct_camera[i].snap_flag)
             {
                //stop Acquisition
                status = GXStreamOff(m_devices_handle[i]);
                if (status != GX_STATUS_SUCCESS)
                {
                    //Add your code to process error
                }

                //Unregister Capture CallBack Function
                status = GXUnregisterCaptureCallback(m_devices_handle[i]);
                if (status != GX_STATUS_SUCCESS)
                {
                    //Add your code to process error
                }

                //Release the QImage resource
                if (m_struct_camera[i].show_image != NULL)
               {
                    delete m_struct_camera[i].show_image;
                    m_struct_camera[i].show_image = NULL;
                    //Stoping the acquisition will trigger a drawing event. Passing pShowImage to the showImage function
                    m_child_window[i]->ShowImage(m_struct_camera[i].show_image, NULL, NULL, false);
                    m_child_window[m_operate_id]->m_objFps.ResetTotalFrameCount();
                }
                m_struct_camera[i].result_image = NULL;

                m_struct_camera[i].snap_flag = false;
            }

            if (m_struct_camera[i].open_flag)
            {
                //close device
                status = GXCloseDevice(m_devices_handle[i]);
                if (status != GX_STATUS_SUCCESS)
                {
                    // Error handling
                }
                m_devices_handle[i] = NULL;
                m_struct_camera[i].open_flag = false;
            }
        }
    }

    //Stop the timer
    if ( m_timer->isActive() )
    {
        m_timer->stop();
    }

    //Release memory
    if(m_baseinfo != NULL)
    {
        delete[] m_baseinfo;
        m_baseinfo = NULL;
    }

    //Release the device handle
    if(m_devices_handle != NULL)
    {
        delete[] m_devices_handle;
        m_devices_handle = NULL;
    }

    //Release the camera information data structure
    if(m_struct_camera != NULL)
    {
        for(int i = 0; i < m_camera_number; i++)
        {
            if (m_struct_camera[i].show_image != NULL)
            {
                delete m_struct_camera[i].show_image;
                m_struct_camera[i].show_image = NULL;
            }
            m_struct_camera[i].result_image = NULL;
        }
        delete[] m_struct_camera;
        m_struct_camera = NULL;
    }

    //Close the library of GX SDK
    status = GXCloseLib();
    if (status != GX_STATUS_SUCCESS)
    {
        ShowErrorString(status);
        exit(0);
    }
    delete ui;
}

//----------------------------------------------------------------------------------
/**
\brief Initialize the function
\input
\output
\return success: true  fail: false
*/
//------------------------------------------------------------------------------------
bool MainWindow::__Init()
{
    bool result = false;

    do
    {
        //Create multiple document areas
        m_mdiarea = new QMdiArea(this);
        m_mdiarea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_mdiarea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_mdiarea->setViewMode(QMdiArea::SubWindowView);
        this->setCentralWidget(m_mdiarea);

        //Create action
        result = this->__CreateActions();
        UI_CHECK_BOOL_RESULT(result);

        //Create a menu bar
        result = this->__CreateMenu();
        UI_CHECK_BOOL_RESULT(result);

        //Interface layout
        this->__Layout();

        //Create a timer
        m_timer = new QTimer(this);

        //Start the timer
        startTimer(50);

        result = true;

    }while(0);

    return result;
}

//----------------------------------------------------------------------------------
/**
\brief Create action
\input
\output
\return success:true  fail:false
*/
//------------------------------------------------------------------------------------
bool MainWindow::__CreateActions()
{
    bool result = false;
    bool connect_flag = false;

    do
    {
        //
        m_select_camera = new(std::nothrow) QAction(tr("select camera"), this);
        UI_CHECK_NEW_MEMORY(m_select_camera);
        m_select_camera->setDisabled(false);
        connect_flag= connect(m_select_camera, SIGNAL( triggered() ), this, SLOT( SlotSelectDevice(void) ) );
        UI_CHECK_BOOL_RESULT(connect_flag);

        //
        m_action_open_device = new(std::nothrow) QAction(tr("open device"), this);
        UI_CHECK_NEW_MEMORY(m_action_open_device);
        m_action_open_device->setDisabled(true);
        connect_flag= connect(m_action_open_device, SIGNAL( triggered() ), this, SLOT( SlotOpenDevice(void) ) );
        UI_CHECK_BOOL_RESULT(connect_flag);

        //
        m_action_start_acq = new(std::nothrow) QAction(tr("start acquisition"), this);
        UI_CHECK_NEW_MEMORY(m_action_start_acq);
        m_action_start_acq->setDisabled(true);
        connect_flag= connect(m_action_start_acq, SIGNAL( triggered() ), this, SLOT( SlotStartAcquisition(void) ) );
        UI_CHECK_BOOL_RESULT(connect_flag);

        //
        m_action_stop_acq = new(std::nothrow) QAction(tr("stop acquisition"), this);
        UI_CHECK_NEW_MEMORY(m_action_stop_acq);
        m_action_stop_acq->setDisabled(true);
        connect_flag= connect(m_action_stop_acq, SIGNAL( triggered() ), this, SLOT( SlotStopAcquisition(void) ) );
        UI_CHECK_BOOL_RESULT(connect_flag);

        //
        m_action_close_device = new(std::nothrow) QAction(tr("close device"), this);
        UI_CHECK_NEW_MEMORY(m_action_close_device);
        m_action_close_device->setDisabled(true);
        connect_flag= connect(m_action_close_device, SIGNAL( triggered() ), this, SLOT( SlotCloseDevice(void) ) );
        UI_CHECK_BOOL_RESULT(connect_flag);

        //
        m_action_parameter_dialog = new(std::nothrow) QAction(tr("Setting parameters"), this);
        UI_CHECK_NEW_MEMORY(m_action_parameter_dialog);
        m_action_parameter_dialog->setDisabled(true);
        connect_flag= connect(m_action_parameter_dialog, SIGNAL( triggered() ), this, SLOT( SlotOnParameterDialog(void) ) );
        UI_CHECK_BOOL_RESULT(connect_flag);

        //
        m_action_about_info = new(std::nothrow) QAction(tr("About GxMultiCam(A)... "), this);
        UI_CHECK_NEW_MEMORY(m_action_about_info);
        m_action_about_info->setCheckable(false);
        connect_flag= connect(m_action_about_info, SIGNAL( triggered() ), this, SLOT( SlotAboutInfo(void) ) );
        UI_CHECK_BOOL_RESULT(connect_flag);

        result = true;

    }while(0);

    return result;
}

//----------------------------------------------------------------------------------
/**
\brief Create a menu bar
    if failed ,you do not have to manually delete the pointer,
    the main window will automatically destroy these menu items when it is destroyed.
\input
\output
\return success: true  fail :false
*/
//------------------------------------------------------------------------------------
bool MainWindow::__CreateMenu()
{
    bool result = false;

    do
    {
        //Get the menu bar
        QMenuBar* menu_bar = this->menuBar();
        UI_CHECK_NEW_MEMORY(menu_bar);

        //Create the "Camera" menu item
        m_camera = menu_bar->addMenu(tr("camera"));
        UI_CHECK_NEW_MEMORY(m_camera);
        m_camera->addAction(m_select_camera);
        m_camera->addSeparator();
        m_camera->addAction(m_action_open_device);
        m_camera->addAction(m_action_start_acq);
        m_camera->addAction(m_action_stop_acq);
        m_camera->addAction(m_action_close_device);

        //Create the "Parameters" menu item
        m_parameter = menu_bar->addMenu(tr("parameters"));
        UI_CHECK_NEW_MEMORY(m_parameter);
        m_parameter->addAction(m_action_parameter_dialog);

        //Create a "Help" menu item
        m_help = menu_bar->addMenu(tr("help"));
        UI_CHECK_NEW_MEMORY(m_help);
        m_help->addAction(m_action_about_info);

        result = true;

    }while(0);

    return result;
}

//-------------------------------------------------------------------------
/**
\brief  Set the icon for the sample program
\input
\output
\return void
*/
//-------------------------------------------------------------------------
void MainWindow::__SetSystemIcon(void)
{
    //Get the file path of this application
    QString string_path = QCoreApplication::applicationDirPath();

    //Get the path of icon
    QString string_con_path = string_path + QString("/logo.png");

    //Read the icon
    QPixmap object_icon_img(string_con_path);

    //If read the icon fail, use the default icon
    if(object_icon_img.isNull())
    {
        this->setWindowIcon(QIcon(":/images/DefaultLogo.png"));
    }
    else //use the loaded icon
    {
        this->setWindowIcon(QIcon(object_icon_img));
    }
}

//----------------------------------------------------------------------------------
/**
\brief Refresh the status of the menu bar
\input
\output
\return void
*/
//------------------------------------------------------------------------------------
void MainWindow::UpdateUI(void)
{
    if(m_operate_id < 0 || m_camera_number <= 0)
    {
        m_action_open_device->setDisabled(true);
        m_action_start_acq->setDisabled(true);
        m_action_stop_acq->setDisabled(true);
        m_action_close_device->setDisabled(true);
        m_action_parameter_dialog->setDisabled(true);
    }
    else
    {
        //Update the status of each menu bar
        m_action_open_device->setDisabled(m_struct_camera[m_operate_id].open_flag);
        m_action_start_acq->setDisabled(!(m_struct_camera[m_operate_id].open_flag &&(!m_struct_camera[m_operate_id].snap_flag)));
        m_action_stop_acq->setDisabled(!m_struct_camera[m_operate_id].snap_flag);
        m_action_close_device->setDisabled(!m_struct_camera[m_operate_id].open_flag);
        m_action_parameter_dialog->setDisabled((!m_struct_camera[m_operate_id].open_flag));
    }
}

//----------------------------------------------------------------------------------
/**
\brief The slot function connected to the action of selected the camera
\input
\output
\return void
*/
//------------------------------------------------------------------------------------
void MainWindow::SlotSelectDevice(void)
{
    //Instantiate the camera selection window
    CSelectCameraDialog select_device_dialog(this);

    //Set the dialog mode
    select_device_dialog.setWindowModality(Qt::ApplicationModal);
    //Set the selected camera window size can not be changed
    select_device_dialog.setFixedSize(select_device_dialog.width(),select_device_dialog.height());

    //Initialization
    select_device_dialog.InitDialog();

    //Display selecting device dialog
    select_device_dialog.exec();

    if(m_operate_id >= 0)
    {
        //Activate the window corresponded to the select the camera
        m_mdiarea->setActiveSubWindow(m_mdiarea_sub_window[m_operate_id]);
    }

    //Update UI
    UpdateUI();
}

//----------------------------------------------------------------------------------
/**
\brief The slot function connected to the action of setting the parameter
\input
\output
\return void
*/
//------------------------------------------------------------------------------------
void MainWindow::SlotOnParameterDialog(void)
{
    //Instantiate the parameter settings window
   m_parameter_dialog = new CParameterDialog;
   if(m_parameter_dialog == NULL)
   {
       return;
   }

   //Set window to fixed size
   m_parameter_dialog->setFixedSize(m_parameter_dialog->width(), m_parameter_dialog->height());
   //Set the window mode
   m_parameter_dialog->setWindowModality(Qt::ApplicationModal);

   //Display window
   m_parameter_dialog->InitUI(m_devices_handle[m_operate_id]);
   m_parameter_dialog->show();
   m_parameter_dialog->raise();
   m_parameter_dialog->activateWindow();
}

//----------------------------------------------------------------------------------
/**
\brief The slot function connected to the action of opening device
\input
\output
\return void
*/
//------------------------------------------------------------------------------------
void MainWindow::SlotOpenDevice(void)
{
    __OpenDevice();
}

//----------------------------------------------------------------------------------
/**
\brief The slot function connected to the action of starting acquisition
\input
\output
\return void
*/
//------------------------------------------------------------------------------------
void MainWindow::SlotStartAcquisition(void)
{
    __StartAcquisition();
}

//----------------------------------------------------------------------------------
/**
\brief The slot function connected to the action of stoping acquisition
\input
\output
\return void
*/
//------------------------------------------------------------------------------------
void MainWindow::SlotStopAcquisition(void)
{
    __StopAcquisition();
}

//----------------------------------------------------------------------------------
/**
\brief The slot function connected to the action of closing device
\input
\output
\return void
*/
//------------------------------------------------------------------------------------
void MainWindow::SlotCloseDevice(void)
{
    __CloseDevice();
}

//----------------------------------------------------------------------------------
/**
\brief  Timer events
\param  Reference the Qt help documentation
*/
//----------------------------------------------------------------------------------
void MainWindow::timerEvent(QTimerEvent *event)
{
    uint32_t	    i     = 0;                                      //Loop variable
    static int		count      = 0;                             //Accumulated value

    count++;
    if((count % 20) == 0)
    {
         for(i = 0; i < m_camera_number; i++)
         {
             m_child_window[i]->m_objFps.UpdateFps();
             m_struct_camera[i].fps = m_child_window[i]->m_objFps.GetFps();
         }
         m_object_time.restart();
    }
}

//----------------------------------------------------------------------------------
/**
\brief The slot function connected to the action of displaying the program information
\input
\output
\return void
*/
//------------------------------------------------------------------------------------
void MainWindow::SlotAboutInfo(void)
{
    //Instantiate the parameter settings window
   m_about_dialog = new CAboutDialog;
   if(m_about_dialog == NULL)
   {
       return;
   }

   //Set window to fixed size
   m_about_dialog ->setFixedSize(m_about_dialog->width(), m_about_dialog->height());
   //Set the window mode
   m_about_dialog->setWindowModality(Qt::ApplicationModal);

   //Display window
   m_about_dialog->show();
   m_about_dialog->raise();
   m_about_dialog->activateWindow();
}

//----------------------------------------------------------------------------------
/**
\brief The function for opening the camera
\input
\output
\return  void
*/
//------------------------------------------------------------------------------------
void MainWindow::__OpenDevice()
{
    GX_STATUS status = GX_STATUS_ERROR;
    char      index[10]    = {0};
    int64_t   value   = 0;
    bool      is_implemented = false;

    // This struct is designed for the open device interface
    GX_OPEN_PARAM open_param;
    open_param.accessMode = GX_ACCESS_EXCLUSIVE;
    open_param.openMode   = GX_OPEN_INDEX;
    open_param.pszContent = "";

    //The first camera index is 1
    sprintf(index, "%d", (m_operate_id+1));
    //Close the cameras if they has been opened.
    if (m_devices_handle[m_operate_id] != NULL)
    {
        status = GXCloseDevice(m_devices_handle[m_operate_id]);
        GX_VERIFY(status);
        m_devices_handle[m_operate_id] = NULL;
    }
    open_param.pszContent = index;

    //Open camera
    status = GXOpenDevice(&open_param, &m_devices_handle[m_operate_id]);
    GX_VERIFY(status);

    // Whether it is a Gige camera
    if(m_baseinfo[m_operate_id].deviceClass == GX_DEVICE_CLASS_GEV)
    {
        //High-resolution Gige cameras require a large timeout time,
        //Otherwise you can't acquire complete images.
        status = GXSetInt(m_devices_handle[m_operate_id],  GX_DS_INT_BLOCK_TIMEOUT,  1000);
        GX_VERIFY(status);
    }

    //The flag of opening camera
    m_struct_camera[m_operate_id].open_flag = true;

    //Get the type of Bayer conversion. whether is a color camera.
    status = GXIsImplemented(m_devices_handle[m_operate_id], GX_ENUM_PIXEL_COLOR_FILTER, &is_implemented);
    GX_VERIFY(status);
    m_struct_camera[m_operate_id].color_filter_flag = is_implemented;
    if (is_implemented)
    {
        status = GXGetEnum(m_devices_handle[m_operate_id], GX_ENUM_PIXEL_COLOR_FILTER, &value);
        GX_VERIFY(status);
        m_struct_camera[m_operate_id].bayer_layout = value;
    }

    // Get the image width from camera
    status = GXGetInt(m_devices_handle[m_operate_id], GX_INT_WIDTH, &value);
    GX_VERIFY(status);
    m_struct_camera[m_operate_id].image_width = value;

    //Get the image height from camera
    status = GXGetInt(m_devices_handle[m_operate_id], GX_INT_HEIGHT, &value);
    GX_VERIFY(status);
    m_struct_camera[m_operate_id].image_height = value;

    //When you are not sure of the current camera's Pixel format,
    //you can call the following function to set the image data format to 8Bit
    status = SetPixelFormat8bit(m_devices_handle[m_operate_id]);
    GX_VERIFY(status);

    // Get PayloadSize of image from camera
    status = GXGetInt(m_devices_handle[m_operate_id], GX_INT_PAYLOAD_SIZE, &value);
    GX_VERIFY(status);
    m_struct_camera[m_operate_id].payload_size = value;

    //Set the acquisition mode to  continuous acquisition
    status = GXSetEnum(m_devices_handle[m_operate_id], GX_ENUM_ACQUISITION_MODE, GX_ACQ_MODE_CONTINUOUS);
    GX_VERIFY(status);

    //Set the trigger mode to OFF
    status = GXSetEnum(m_devices_handle[m_operate_id], GX_ENUM_TRIGGER_MODE, GX_TRIGGER_MODE_OFF);
    GX_VERIFY(status);

    UpdateUI();
}

//----------------------------------------------------------------------------------
/**
\brief The function for closing the device
\input
\output
\return  success:true  fail:false
*/
//------------------------------------------------------------------------------------
void MainWindow::__CloseDevice()
{
    GX_STATUS status = GX_STATUS_SUCCESS;

    if (m_struct_camera[m_operate_id].snap_flag)
    {
        //stop acquisition
        status = GXStreamOff(m_devices_handle[m_operate_id]);
        if (status != GX_STATUS_SUCCESS)
        {
            //Add codes to process error
        }

        //unregister CallBackFunction
        status = GXUnregisterCaptureCallback(m_devices_handle[m_operate_id]);
        if (status != GX_STATUS_SUCCESS)
        {
            // Add codes to process error
        }

        //Release the display space
        if (m_struct_camera[m_operate_id].show_image != NULL)
       {
            delete m_struct_camera[m_operate_id].show_image;
            m_struct_camera[m_operate_id].show_image = NULL;
            //Stop the acquisition will trigger a drawing event,
            //Passing the pointer of pShowImage to the ShowImage function
            m_child_window[m_operate_id]->ShowImage(m_struct_camera[m_operate_id].show_image, NULL, NULL, false);
            m_child_window[m_operate_id]->m_objFps.ResetTotalFrameCount();
        }
        m_struct_camera[m_operate_id].result_image = NULL;

        m_struct_camera[m_operate_id].snap_flag = false;
    }

    //Close device
    status = GXCloseDevice(m_devices_handle[m_operate_id]);
    if (status != GX_STATUS_SUCCESS)
    {
        // Add codes to process error
    }
    m_devices_handle[m_operate_id] = NULL;
    m_struct_camera[m_operate_id].open_flag = false;

    UpdateUI();
}

//----------------------------------------------------------------------------------
/**
\brief The function for starting acquisition
\input
\output
\return  success: true  fail: false
*/
//------------------------------------------------------------------------------------
void MainWindow::__StartAcquisition()
{
    GX_STATUS status       = GX_STATUS_ERROR;
    bool      is_color_filter = m_struct_camera[m_operate_id].color_filter_flag;

    // If it is a color camera, apply resources for it
    if(is_color_filter)
    {
        m_struct_camera[m_operate_id].show_image = new QImage(m_struct_camera[m_operate_id].image_width, m_struct_camera[m_operate_id].image_height, QImage::Format_RGB888);
        m_struct_camera[m_operate_id].result_image = m_struct_camera[m_operate_id].show_image->bits();
    }

    // If it is a mono camera, apply resources for it
    else
    {
        m_struct_camera[m_operate_id].show_image = new QImage(m_struct_camera[m_operate_id].image_width, m_struct_camera[m_operate_id].image_height, QImage::Format_RGB888);
        m_struct_camera[m_operate_id].result_image = m_struct_camera[m_operate_id].show_image->bits();
    }

    //Register CallBackFunction
    RegisterCallback(m_operate_id);

    //Start Acquisition
    status = GXStreamOn(m_devices_handle[m_operate_id]);
    GX_VERIFY(status);

    m_struct_camera[m_operate_id].snap_flag = true;
    m_display_flag[m_operate_id] = true;

    UpdateUI();
}

//----------------------------------------------------------------------------------
/**
\brief The function for stoping acquisition
\input
\output
\return  success: true  fail: false
*/
//------------------------------------------------------------------------------------
void MainWindow::__StopAcquisition()
{
    GX_STATUS status = GX_STATUS_ERROR;

    //Stop Acquisition
    status = GXStreamOff(m_devices_handle[m_operate_id]);
    GX_VERIFY(status);

    //Unregister CallBackFunction
    status = GXUnregisterCaptureCallback(m_devices_handle[m_operate_id]);
    GX_VERIFY(status);

    //Release the QImage object
    if (m_struct_camera[m_operate_id].show_image != NULL)
    {
        delete m_struct_camera[m_operate_id].show_image;
        m_struct_camera[m_operate_id].show_image = NULL;
        //Stops the acquisition will trigger a drawing event,
        //passing the pointer of pShowImage to the drawing function
        m_child_window[m_operate_id]->ShowImage(m_struct_camera[m_operate_id].show_image, NULL, NULL, false);
        m_child_window[m_operate_id]->m_objFps.ResetTotalFrameCount();
    }

    m_struct_camera[m_operate_id].result_image = NULL;

    m_struct_camera[m_operate_id].snap_flag = false;

    UpdateUI();
}

//----------------------------------------------------------------------------------
/**
\brief The function for seting the Form layout
\input
\output
\return  void
*/
//------------------------------------------------------------------------------------
void MainWindow::__Layout()
{
    //Loop to create four sub-windows
    for(int i = 0; i < DEVICE_MAX_NUM; i++)
    {
        //Create a window object
        m_child_window[i] = new CChildWindow(m_mdiarea);
        //Add the sub-window to the multi-document area
        m_mdiarea_sub_window[i] = m_mdiarea->addSubWindow(m_child_window[i]);

        //The buttion of removing the sub-window
        //m_mdiarea_sub_window[i]->setWindowFlags(Qt::CustomizeWindowHint|Qt::WindowTitleHint | Qt::FramelessWindowHint);
        m_mdiarea_sub_window[i]->setWindowFlags(Qt::WindowTitleHint | Qt::CustomizeWindowHint);

        //Remove the child window title bar and add shadows
        QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(m_mdiarea_sub_window[i]);
        shadow_effect->setOffset(1, 1);
        shadow_effect->setColor(Qt::gray);
        shadow_effect->setBlurRadius(5);
        m_mdiarea_sub_window[i]->setGraphicsEffect(shadow_effect);

        //Show sub-window
        m_mdiarea_sub_window[i]->show();
        if(NULL == m_mdiarea_sub_window[i])
        {
             break;
          }
    }

    //Set the layout of  sub-window
    QGridLayout *image_view = new QGridLayout(m_mdiarea);
    image_view->addWidget(m_mdiarea_sub_window[0],0,0);
    image_view->addWidget(m_mdiarea_sub_window[1],0,1);
    image_view->addWidget(m_mdiarea_sub_window[2],1,0);
    image_view->addWidget(m_mdiarea_sub_window[3],1,1);
    image_view->setHorizontalSpacing(2);              //Horizontal interval is 2
    image_view->setVerticalSpacing(2);                  //The vertical interval is 2
    image_view->setContentsMargins(0, 0, 0, 0);    //Remove the blanks around
    m_mdiarea->setLayout(image_view);

    //Make sure that the first window is the current active window
    m_mdiarea->setActiveSubWindow(m_mdiarea_sub_window[0]);
}

// ---------------------------------------------------------------------------------
/**
\brief   Set the camera's data format to 8bit
\input   device_handle      device handle
\output
\return  status GX_STATUS_SUCCESS:Set up successfully, other:Set up failed
*/
// ----------------------------------------------------------------------------------
GX_STATUS MainWindow::SetPixelFormat8bit(GX_DEV_HANDLE device_handle)
{
    GX_STATUS status    = GX_STATUS_SUCCESS;
    int64_t   pixel_size  = 0;
    uint32_t  enmu_entry  = 0;
    size_t    buffer_size = 0;

    GX_ENUM_DESCRIPTION  *enum_description = NULL;

    // Get the pixel size
    status = GXGetEnum(device_handle, GX_ENUM_PIXEL_SIZE, &pixel_size);
    if (status != GX_STATUS_SUCCESS)
    {
        return status;
    }

    // When the pixel size is 8bit,return directly.If not, set the camera pixel size to 8bit
    if (pixel_size == GX_PIXEL_SIZE_BPP8)
    {
        return GX_STATUS_SUCCESS;
    }
    else
    {
        // Gets the number of pixel formats enumerated by the device
        status = GXGetEnumEntryNums(device_handle, GX_ENUM_PIXEL_FORMAT, &enmu_entry);
        if (status != GX_STATUS_SUCCESS)
        {
            return status;
        }

        // Prepare resources for the pixel format enumeration values that are supported by the device
        buffer_size      = enmu_entry * sizeof(GX_ENUM_DESCRIPTION);
        enum_description = new GX_ENUM_DESCRIPTION[enmu_entry];

        // Gets the supported enumerated values
        status = GXGetEnumDescription(device_handle, GX_ENUM_PIXEL_FORMAT, enum_description, &buffer_size);
        if (status != GX_STATUS_SUCCESS)
        {
            if (enum_description != NULL)
            {
                delete []enum_description;
                enum_description = NULL;
            }
            return status;
        }

        // Traversing the pixel format supported by the device, set the camera pixel format to 8bit,
        // If the device supports the pixel format Mono10 and Mono8 ,set the camera pixel format  to Mono8.
        for (uint32_t i = 0; i < enmu_entry; i++)
        {
            if ((enum_description[i].nValue & GX_PIXEL_8BIT) == GX_PIXEL_8BIT)
            {
                status = GXSetEnum(device_handle, GX_ENUM_PIXEL_FORMAT, enum_description[i].nValue);
                break;
            }
        }

        // Release resources
        if (enum_description != NULL)
        {
            delete []enum_description;
            enum_description = NULL;
        }
    }

    return status;
}

//---------------------------------------------------------------------------------
/**
\brief	Register callback function according to the camera ID.
        If more than 4 cameras been found on computer,it only can register four callback function
\param	CamID   [in]   camera ID

\return void
*/
//---------------------------------------------------------------------------------
void MainWindow::RegisterCallback(int camera_id)
{
    GX_STATUS status = GX_STATUS_ERROR;
    m_object_time.start();

    //Register CallBackFunction
    switch(camera_id)
    {
    case 0:
        status = GXRegisterCaptureCallback(m_devices_handle[camera_id], this,   (GXCaptureCallBack)OnFrameCallbackFun1);
        GX_VERIFY(status);
        break;

    case 1:
        status = GXRegisterCaptureCallback(m_devices_handle[camera_id], this, (GXCaptureCallBack)OnFrameCallbackFun2);
        GX_VERIFY(status);
        break;

    case 2:
        status = GXRegisterCaptureCallback(m_devices_handle[camera_id], this, (GXCaptureCallBack)OnFrameCallbackFun3);
        GX_VERIFY(status);
        break;

    case 3:
        status = GXRegisterCaptureCallback(m_devices_handle[camera_id], this, (GXCaptureCallBack)OnFrameCallbackFun4);
        GX_VERIFY(status);
        break;

    default:
        break;
    }

}

//---------------------------------------------------------------------------------
/**
\brief  The callback function of first camera
\param  frame is the parameters of callback function

\return void
*/
//---------------------------------------------------------------------------------
void  MainWindow::OnFrameCallbackFun1(GX_FRAME_CALLBACK_PARAM* frame)
{
    if (frame->status != 0)
    {
        return;
    }

    MainWindow *pf1 = (MainWindow*)(frame->pUserParam);
    pf1->m_frame_first = frame;

    int		id = 0;                                //ID number
    int		image_width    = 0;           //image width
    int                image_height    = 0;          //image height
    int64_t	        bayer_layout  = 0;              //Bayer format
    void             *result_image = NULL;      //the image pointer for display
    QImage       *show_image = NULL;
    QString       device_name = "";              //camera
    QString       display_fps = "";                 //frame rate

    image_width    = (int)(pf1->m_struct_camera[id].image_width);
    image_height   = (int)(pf1->m_struct_camera[id].image_height);
    bayer_layout = pf1->m_struct_camera[id].bayer_layout;
    result_image = pf1->m_struct_camera[id].result_image;
    show_image = pf1->m_struct_camera[id].show_image;


    //If it is a color camera, converts the images format from Raw8 to RGB
    if (pf1->m_struct_camera[id].color_filter_flag)
    {
        //Converts the images format from Raw8 to RGB for display
        DxRaw8toRGB24((char*)frame->pImgBuf, result_image, image_width, image_height, RAW2RGB_NEIGHBOUR,
            DX_PIXEL_COLOR_FILTER(bayer_layout), false);
    }
    else
    {
        DxRaw8toRGB24((char*)frame->pImgBuf, result_image, image_width, image_height,RAW2RGB_NEIGHBOUR,
            DX_PIXEL_COLOR_FILTER(NONE),false);
    }

    device_name.sprintf("camera: %s", pf1->m_baseinfo[id].szDisplayName);

    display_fps.sprintf("serial number: %s frame rate: %.2f FPS", pf1->m_baseinfo[id].szSN, pf1->m_struct_camera[id].fps);

    pf1->m_child_window[id]->ShowImage(show_image, device_name, display_fps, pf1->m_view_flag);
    if(pf1->m_display_flag[id] == true)
    {
        pf1->m_child_window[id]->setWindowTitle(device_name);
        pf1->m_child_window[id]->update();
        pf1->m_display_flag[id] = false;
    }
}

//---------------------------------------------------------------------------------
/**
\brief  The callback function of  second camera
\param  frame is the parameters of callback function

\return void
*/
//---------------------------------------------------------------------------------
void  MainWindow::OnFrameCallbackFun2(GX_FRAME_CALLBACK_PARAM* frame)
{
    if (frame->status != 0)
    {
        return;
    }
    MainWindow *pf2 = (MainWindow*)(frame->pUserParam);
    pf2->m_frame_second = frame;

    int             id = 1;                                    //ID number
    int             image_width     = 0;               //image width
    int             image_height    = 0;               //image height
    int64_t	     bayer_layout  = 0;                  //Bayer format
    void          *result_image = NULL;          //the image pointer for display
    QImage    *show_image = NULL;
    QString    device_name = "";                   //camera
    QString    display_fps = "";                     //frame rate

    image_width    = (int)(pf2->m_struct_camera[id].image_width);
    image_height   = (int)(pf2->m_struct_camera[id].image_height);
    bayer_layout = pf2->m_struct_camera[id].bayer_layout;
    result_image = pf2->m_struct_camera[id].result_image;
    show_image = pf2->m_struct_camera[id].show_image;

    //If it is a color camera, converts the images format from Raw8 to RGB
    if (pf2->m_struct_camera[id].color_filter_flag)
    {
        //Converts the images format from Raw8 to RGB for display
        DxRaw8toRGB24((char*)frame->pImgBuf, result_image, image_width, image_height, RAW2RGB_NEIGHBOUR,
            DX_PIXEL_COLOR_FILTER(bayer_layout), false);
    }
    else
    {
        DxRaw8toRGB24((char*)frame->pImgBuf, result_image, image_width, image_height,RAW2RGB_NEIGHBOUR,
            DX_PIXEL_COLOR_FILTER(NONE),false);
    }

    device_name.sprintf("camera: %s", pf2->m_baseinfo[id].szDisplayName);
    display_fps.sprintf("serial number: %s frame rate: %.2f FPS", pf2->m_baseinfo[id].szSN, pf2->m_struct_camera[id].fps);

    pf2->m_child_window[id]->ShowImage(show_image, device_name, display_fps, pf2->m_view_flag);
    if(pf2->m_display_flag[id] == true)
    {
        pf2->m_child_window[id]->setWindowTitle(device_name);
        pf2->m_child_window[id]->update();
        pf2->m_display_flag[id] = false;
    }
}

//---------------------------------------------------------------------------------
/**
\brief  The callback function of third camera
\param  frame is the parameters of callback function

\return void
*/
//---------------------------------------------------------------------------------
void  MainWindow::OnFrameCallbackFun3(GX_FRAME_CALLBACK_PARAM* frame)
{
    if (frame->status != 0)
    {
        return;
    }

    MainWindow *pf3 = (MainWindow*)(frame->pUserParam);
    pf3->m_frame_first = frame;

    int	    id = 2;                                    //ID number
    int	    image_width     = 0;               //image widht
    int            image_height    = 0;               //image height
    int64_t	    bayer_layout  = 0;                  //Bayer format
    void         *result_image = NULL;          //the image pointer for display
    QImage   *show_image = NULL;
    QString   device_name = "";                   //camera
    QString   display_fps = "";                     //frame rate

    image_width    = (int)(pf3->m_struct_camera[id].image_width);
    image_height   = (int)(pf3->m_struct_camera[id].image_height);
    bayer_layout = pf3->m_struct_camera[id].bayer_layout;
    result_image = pf3->m_struct_camera[id].result_image;
    show_image = pf3->m_struct_camera[id].show_image;

    //If it is a color camera, converts the images format from Raw8 to RGB
    if (pf3->m_struct_camera[id].color_filter_flag)
    {
        //Converts the images format from Raw8 to RGB for display
        DxRaw8toRGB24((char*)frame->pImgBuf, result_image, image_width, image_height, RAW2RGB_NEIGHBOUR,
            DX_PIXEL_COLOR_FILTER(bayer_layout), false);
    }
    else
    {
        DxRaw8toRGB24((char*)frame->pImgBuf, result_image, image_width, image_height,RAW2RGB_NEIGHBOUR,
            DX_PIXEL_COLOR_FILTER(NONE),false);
    }

    device_name.sprintf("camera: %s", pf3->m_baseinfo[id].szDisplayName);
    display_fps.sprintf("serial number: %s frame rate: %.2f FPS", pf3->m_baseinfo[id].szSN, pf3->m_struct_camera[id].fps);

    pf3->m_child_window[id]->ShowImage(show_image, device_name, display_fps, pf3->m_view_flag);
    if(pf3->m_display_flag[id] == true)
    {
        pf3->m_child_window[id]->setWindowTitle(device_name);
        pf3->m_child_window[id]->update();
        pf3->m_display_flag[id] = false;
    }
}

//---------------------------------------------------------------------------------
/**
\brief   The callback function of fourth camera
\param   frame is the parameters of callback function

\return void
*/
//---------------------------------------------------------------------------------
void  MainWindow::OnFrameCallbackFun4(GX_FRAME_CALLBACK_PARAM* frame)
{
    if (frame->status != 0)
    {
        return;
    }

    MainWindow *pf4 = (MainWindow*)(frame->pUserParam);
    pf4->m_frame_first = frame;

    int	    id = 3;                                     //ID number
    int	    image_width     = 0;                //image width
    int            image_height    = 0;               //image height
    int64_t	    bayer_layout  = 0;                   //Bayer format
    void         *result_image = NULL;           //the image pointer for display
    QImage   *show_image = NULL;
    QString   device_name = "";                   //camera
    QString   display_fps = "";                      //frame rate

    image_width    = (int)(pf4->m_struct_camera[id].image_width);
    image_height   = (int)(pf4->m_struct_camera[id].image_height);
    bayer_layout = pf4->m_struct_camera[id].bayer_layout;
    result_image = pf4->m_struct_camera[id].result_image;
    show_image = pf4->m_struct_camera[id].show_image;

    //If it is a color camera, converts the images format from Raw8 to RGB
    if (pf4->m_struct_camera[id].color_filter_flag)
    {
        //Converts the images format from Raw8 to RGB for display
        DxRaw8toRGB24((char*)frame->pImgBuf, result_image, image_width, image_height, RAW2RGB_NEIGHBOUR,
            DX_PIXEL_COLOR_FILTER(bayer_layout), false);
    }
    else
    {
        DxRaw8toRGB24((char*)frame->pImgBuf, result_image, image_width, image_height,RAW2RGB_NEIGHBOUR,
            DX_PIXEL_COLOR_FILTER(NONE),false);
    }

    device_name.sprintf("camera: %s", pf4->m_baseinfo[id].szDisplayName);
    display_fps.sprintf("serial: %s frame rate: %.2f FPS", pf4->m_baseinfo[id].szSN, pf4->m_struct_camera[id].fps);

    pf4->m_child_window[id]->ShowImage(show_image, device_name, display_fps, pf4->m_view_flag);

    if(pf4->m_display_flag[id] == true)
    {
        pf4->m_child_window[id]->setWindowTitle(device_name);
        pf4->m_child_window[id]->update();
        pf4->m_display_flag[id] = false;
    }
}

//----------------------------------------------------------------------------------
/**
\brief  Show Error Message
\input  error_status
\output
\return  void
*/
//------------------------------------------------------------------------------------
void MainWindow::ShowErrorString(GX_STATUS error_status)
{
        char*     error_info = NULL;
        size_t    size        = 0;
        GX_STATUS status     = GX_STATUS_ERROR;

        // Get the length of the error message and apply for memory
        status = GXGetLastError(&error_status, NULL, &size);
        error_info = new char[size];
        if (NULL == error_info)
        {
            return;
        }

        // Get the error message and display it
        status = GXGetLastError (&error_status, error_info, &size);
        if (status != GX_STATUS_SUCCESS)
        {
            QMessageBox::about(NULL, "Error", " GXGetLastError Interface call failed ! ");
        }
        else
        {
            QMessageBox::about(NULL, "Error", tr("%1").arg(QString(QLatin1String(error_info))));
        }

        // Release the application's memory space
        if (NULL != error_info)
        {
            delete[] error_info;
            error_info = NULL;
        }
}
