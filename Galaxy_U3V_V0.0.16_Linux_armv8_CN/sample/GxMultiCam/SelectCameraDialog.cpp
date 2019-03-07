//---------------------------------------------------------------------------------------
/**
\file          SelectCameraDialog.cpp
\brief       SelectCameraDialog Function
\version   v1.0.1603.9011
\date        2016-03-01
*/
//---------------------------------------------------------------------------------------

#include "SelectCameraDialog.h"
#include "ui_selectcameradialog.h"
#include "UIDef.h"
#include <QMessageBox>


//----------------------------------------------------------------------------------
/**
\brief  Constructor
\input
\output
\return
*/
//------------------------------------------------------------------------------------
CSelectCameraDialog::CSelectCameraDialog(QWidget* parent)
    :QDialog(parent)
    ,m_select_ui(new Ui_SelectCameraDialog)
    ,m_main_frame(NULL)
    ,m_device_index(-1)
    ,m_device_number(0)
{
    m_select_ui->setupUi(this);
    m_main_frame = (MainWindow*)parentWidget();
}

//----------------------------------------------------------------------------------
/**
\brief  Destructor
\input
\output
\return
*/
//------------------------------------------------------------------------------------
CSelectCameraDialog::~CSelectCameraDialog(void)
{

}

//----------------------------------------------------------------------------------
/**
\brief  Initialize the enumeration of the camera window
\input
\output
\return  success: true  fail: false
*/
//------------------------------------------------------------------------------------
bool CSelectCameraDialog::InitDialog(void)
{
    uint32_t device_number = m_main_frame->m_camera_number;
    uint32_t min_device_num = 0;
    QString device_info = "";

    if(device_number > 0)
    {
        min_device_num = min(device_number, DEVICE_MAX_NUM);
        m_select_ui->comboBox->clear();
        for(unsigned int i = 0; i < min_device_num; i++)
        {
            //Display the device name in the comboBox
            device_info.sprintf("%s", m_main_frame->m_baseinfo[i].szDisplayName);
            m_select_ui->comboBox->addItem(QString(device_info));
        }
        //The last selected camera is displayed in the combobox
        m_select_ui->comboBox->setCurrentIndex(m_main_frame->m_operate_id);
    }
    else
    {
        //if the number of detection devices is 0, re-enumeration.
        UpdateDeviceList();
        if(AllocBufferForMainFrame())
        {
            m_select_ui->comboBox->clear();
            for(unsigned int i = 0; i < m_device_number; i++)
            {
                //Display the device name in the drop-down combox
                device_info.sprintf("%s", m_main_frame->m_baseinfo[i].szDisplayName);
                m_select_ui->comboBox->addItem(QString(device_info));
            }
            m_select_ui->comboBox->setCurrentIndex(0);
            m_main_frame->m_camera_number = m_device_number;
        }
    }
    return true;
}

//----------------------------------------------------------------------------------
/**
\brief  Update the device list
\input
\output
\return
*/
//------------------------------------------------------------------------------------
void CSelectCameraDialog::UpdateDeviceList(void)
{
    GX_STATUS status = GX_STATUS_SUCCESS;
    unsigned int device_number = 0;

    //Enumerate the list of cameras
    status = GXUpdateDeviceList(&device_number, UPDATE_TIME_OUT);
    if(status != GX_STATUS_SUCCESS)
    {
        ShowErrorString(status);
    }

    if (m_main_frame->m_baseinfo != NULL)
    {
        delete []m_main_frame->m_baseinfo;
        m_main_frame->m_baseinfo = NULL;
    }

    //if the number of cameras is greater than 0, allocate resources to the information of camera.
    if(device_number > 0)
    {
        //Allocating resources
        m_main_frame->m_baseinfo = new GX_DEVICE_BASE_INFO[device_number];
        if(m_main_frame->m_baseinfo == NULL)
        {
            QMessageBox::about(NULL, "Error", "Failed to get device information !");
            device_number = 0;
            return;
        }

        //Get all enumerated camera information
        size_t size = device_number * sizeof(GX_DEVICE_BASE_INFO);

        status = GXGetAllDeviceBaseInfo(m_main_frame->m_baseinfo, &size);
        if (status != GX_STATUS_SUCCESS)
        {
            ShowErrorString(status);
            delete []m_main_frame->m_baseinfo;
            m_main_frame->m_baseinfo = NULL;
            // Set the number of devices to zero
            device_number = 0;
            return;
         }
    }

    //Get the total number of drop-down combox which display the device
    m_device_number = min(device_number, DEVICE_MAX_NUM);
}


//----------------------------------------------------------------------------------
/**
\brief  The slot function connected to the button of Re-enumerate the device.
\input
\output
\return  Enumerates the number of cameras
*/
//------------------------------------------------------------------------------------
void CSelectCameraDialog::on_pushButtonupdate_clicked(void)
{
    GX_STATUS status = GX_STATUS_SUCCESS;
    QString device_info = "";

    m_main_frame->m_operate_id = -1;
    //First close the device if it has been opened
    status = CloseCamer();
    if(status != GX_STATUS_SUCCESS)
    {
        //Add codes to process error
    }
    m_select_ui->comboBox->clear();
    ClearBuffer();

    // Update Device List
    UpdateDeviceList();

    //if the number of camera is 0, return directly
    if(m_device_number <= 0)
    {
        m_main_frame->m_camera_number = m_device_number;
        return;
    }

    if(AllocBufferForMainFrame())
    {
        for(unsigned int i = 0; i < m_device_number; i++)
        {
            //add the camera name to the comboBox
            device_info.sprintf("%s", m_main_frame->m_baseinfo[i].szDisplayName);
            m_select_ui->comboBox->addItem(QString(device_info));
        }
        m_select_ui->comboBox->setCurrentIndex(0);
        m_main_frame->m_operate_id = 0;
        m_main_frame->m_camera_number = m_device_number;
    }
}

//----------------------------------------------------------------------------------
/**
\brief  The slot function connected to the button of OK.
\input
\output
\return  void
*/
//------------------------------------------------------------------------------------
void CSelectCameraDialog::on_pushButtonok_clicked(void)
{
    if(m_device_index == -1 && m_device_number > 0)
    {
        //Gets index of the selected device
        m_main_frame->m_operate_id = 0;
    }
    else
    {
        m_main_frame->m_operate_id = m_device_index;
    }

    this->close();
}

//----------------------------------------------------------------------------------
/**
\brief  The slot function connected to the button of cancel
\input
\output
\return  void
*/
//------------------------------------------------------------------------------------
void CSelectCameraDialog::on_pushButtoncancel_clicked(void)
{
    this->close();
}

//----------------------------------------------------------------------------------
/**
\brief  The slot function connected to the combox of selecting the list item.
\input
\output
\return  void
*/
//------------------------------------------------------------------------------------
void CSelectCameraDialog::on_comboBox_activated(void)
{
    //Gets the serial number of the selection
    m_device_index = (int64_t)m_select_ui->comboBox->currentIndex();
}

//----------------------------------------------------------------------------------
/**
\brief  Assign resources to the main window

\return  success:true  fail:false
*/
//----------------------------------------------------------------------------------
bool CSelectCameraDialog::AllocBufferForMainFrame()
{
    // New a camera handle
    m_main_frame->m_devices_handle = new GX_DEV_HANDLE[m_device_number];
    if (m_main_frame->m_devices_handle == NULL)
    {
        ClearBuffer();
        return false;
    }

    //Create a camera data structure
    m_main_frame->m_struct_camera=new CAMER_INFO[m_device_number];
    if (m_main_frame->m_struct_camera == NULL)
    {
        ClearBuffer();
        return false;
    }


        //---------------------Initialize resource information-----------------------------
        for (uint32_t i = 0; i < m_device_number; i++)
            {
                m_main_frame->m_devices_handle[i]  = NULL;

                m_main_frame->m_struct_camera[i].color_filter_flag = false;
                m_main_frame->m_struct_camera[i].open_flag        = false;
                m_main_frame->m_struct_camera[i].snap_flag        = false;
                m_main_frame->m_struct_camera[i].chose_flag        = false;
                m_main_frame->m_struct_camera[i].fps           = 0.0;
                m_main_frame->m_struct_camera[i].bayer_layout   = 0;
                m_main_frame->m_struct_camera[i].image_height   = 0;
                m_main_frame->m_struct_camera[i].image_width    = 0;
                m_main_frame->m_struct_camera[i].payload_size   = 0;
                //Creates a QImage object for displaying images
                m_main_frame->m_struct_camera[i].show_image = NULL;
                m_main_frame->m_struct_camera[i].result_image = NULL;
            }

        return true;
}

//----------------------------------------------------------------------------------
/**
\brief Release resources
\input
\output
\return
*/
//------------------------------------------------------------------------------------
void CSelectCameraDialog::ClearBuffer(void)
{
    MainWindow* main_frame = (MainWindow*)parentWidget();

    //Release resources
    for(uint32_t i = 0; i < main_frame->m_camera_number; i++)
    {
        if (main_frame->m_struct_camera[i].show_image != NULL)
       {
            delete main_frame->m_struct_camera[i].show_image;
            main_frame->m_struct_camera[i].show_image = NULL;
         }
         main_frame->m_struct_camera[i].result_image = NULL;
    }
    // Clear the camera handle
    if(main_frame->m_devices_handle != NULL)
    {
        delete[] main_frame->m_devices_handle;
        main_frame->m_devices_handle = NULL;
    }

    //Clear up camera information
    if(main_frame->m_baseinfo != NULL)
    {
        delete[] main_frame->m_baseinfo;
        main_frame->m_baseinfo = NULL;
    }

    // Release the memory of the pointer of the camera information
    if(main_frame->m_struct_camera != NULL)
    {
        delete[] main_frame->m_struct_camera;
        main_frame->m_struct_camera = NULL;
    }

}

//----------------------------------------------------------------------------------
/**
\brief  Traverse all the cameras to close them 
\input
\output
\return  status GX_STATUS_SUCCESS: success, other: fail
*/
//------------------------------------------------------------------------------------
GX_STATUS CSelectCameraDialog::CloseCamer()
{
    GX_STATUS status = GX_STATUS_SUCCESS;

    //If the camera is not closed, close the camera
    for(uint32_t i = 0; i < m_main_frame->m_camera_number; i++)
    {
        if(m_main_frame->m_struct_camera[i].snap_flag)
        {
            //Stop acquistion
            status = GXStreamOff(m_main_frame->m_devices_handle[i]);
            if(status != GX_STATUS_SUCCESS)
            {
                //Add codes to process error
            }

            //Unregister CallBack Function
            status = GXUnregisterCaptureCallback(m_main_frame->m_devices_handle[i]);
            if(status != GX_STATUS_SUCCESS)
            {
                //Add codes to process error
            }

            //Release the display space
            if (m_main_frame->m_struct_camera[i].show_image != NULL)
           {
                delete m_main_frame->m_struct_camera[i].show_image;
                m_main_frame->m_struct_camera[i].show_image = NULL;
            }
            //Stop the acquisition will trigger a drawing event, passing the pointer of QImage to the ShowImage function
            m_main_frame->m_child_window[i]->ShowImage(m_main_frame->m_struct_camera[i].show_image, NULL, NULL, false);
            m_main_frame->m_struct_camera[i].result_image = NULL;
            m_main_frame->m_struct_camera[i].snap_flag = false;
        }
        if(m_main_frame->m_struct_camera[i].open_flag)
        {
            //Close the camera
            status = GXCloseDevice(m_main_frame->m_devices_handle[i]);
            if(status != GX_STATUS_SUCCESS)
            {
                //Add codes to process error
            }
            m_main_frame->m_devices_handle[i] = NULL;
            m_main_frame->m_struct_camera[i].open_flag = false;
        }
    }
    return status;
}

//----------------------------------------------------------------------------------
/**
\brief  Show error message
\input  error_status
\output
\return  void
*/
//------------------------------------------------------------------------------------
void CSelectCameraDialog::ShowErrorString(GX_STATUS error_status)
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
            QMessageBox::about(NULL, "Error", " GXGetLastError Interface call failed!");
        }
        else
        {
            QMessageBox::about(NULL, "Error", tr("%1").arg(QString(QLatin1String(error_info))));
        }

        // Release the application's memory
        if (NULL != error_info)
        {
            delete[] error_info;
            error_info = NULL;
        }
}



