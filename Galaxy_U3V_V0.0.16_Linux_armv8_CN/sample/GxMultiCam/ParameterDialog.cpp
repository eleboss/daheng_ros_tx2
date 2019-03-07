//---------------------------------------------------------------------------------------
/**
\file          ParameterDialog.cpp
\brief       ParameterDialog Function
\version   v1.0.1603.9011
\date        2016-03-01
*/
//---------------------------------------------------------------------------------------

#include <QMessageBox>
#include <QTimer>
#include "ParameterDialog.h"
#include "ui_parameterdialog.h"
#include "UIDef.h"


//---------------------------------------------------------------------------------
/**
\brief   Constructor
\param   parent
\return  void
*/
//----------------------------------------------------------------------------------
CParameterDialog::CParameterDialog(QWidget* parent)
    :QWidget(parent)
     ,m_ui(new Ui_ParameterDialog)
    ,m_device_handle(NULL)
    ,m_timer(NULL)
    ,m_auto_whitebalance(false)
    ,m_auto_whitebalance_index(0)
    ,m_speed_level(0)
    ,m_shutter_value(0.0)
    ,m_gain_value(0.0)
{
    // Initialize parameters
    memset(&m_gain_float_range,0,sizeof(GX_FLOAT_RANGE));
    memset(&m_shutter_float_range,0,sizeof(GX_FLOAT_RANGE));
    memset(&m_speedlevel_range,0,sizeof(GX_INT_RANGE));

    m_ui->setupUi(this);
}

//---------------------------------------------------------------------------------
/**
\brief   Destructor
\param  void
\return  void
*/
//----------------------------------------------------------------------------------
CParameterDialog::~CParameterDialog(void)
{

}

//---------------------------------------------------------------------------------
/**
\brief   Initialize the interface of UI
\param   device_handle[in]       Device handle
\return  void
*/
//----------------------------------------------------------------------------------
void CParameterDialog::InitUI(GX_DEV_HANDLE device_handle)
{
    m_device_handle = device_handle;
    if (m_device_handle == NULL)
    {
        return;
    }

    // Initialize the auto white balance control
    InitAutoWhiteBalanceUI(m_device_handle);

     // Initialize the exposure control
     InitShutterUI(m_device_handle);

     // Initialize the gain control
     InitGainUI(m_device_handle);

     // Initialize the acquisition speed level controls
     InitSpeedLevel(m_device_handle);

     // Create a timer
     m_timer = new QTimer(this);

     // The timer timeout signal is connected to the slot function
     connect( m_timer, SIGNAL(timeout()), this, SLOT(SlotAutoUpdateABbox()) );

}


//---------------------------------------------------------------------------------
/**
\brief   Initialize the auto white balance control
\param   device_handle[in]       camera handle
\return  void
*/
//----------------------------------------------------------------------------------
void CParameterDialog::InitAutoWhiteBalanceUI(GX_DEV_HANDLE device_handle)
{
    GX_STATUS status = GX_STATUS_ERROR;
    GX_ENUM_DESCRIPTION *enum_description = NULL;
    size_t      buffer_size    = 0;
    int64_t     enum_value     = 0;
    bool        is_implemented   = false;
    uint32_t    entry_num      = 0;
    QString device_info = "";

    //Whether support auto white balance
    status = GXIsImplemented(device_handle, GX_ENUM_BALANCE_WHITE_AUTO, &is_implemented);
    GX_VERIFY(status);
    m_auto_whitebalance = is_implemented;

    if (!is_implemented)
    {
        // if camera does not support auto white balance,return directly.
        return;
    }

     // Get the enumerator for the function
    status = GXGetEnumEntryNums(device_handle, GX_ENUM_BALANCE_WHITE_AUTO, &entry_num);
    GX_VERIFY(status);

    //Apply for buffer
    buffer_size = entry_num * sizeof(GX_ENUM_DESCRIPTION);
    enum_description = new GX_ENUM_DESCRIPTION[entry_num];
    if (enum_description == NULL)
    {
        //QMessageBox("The allocation buffer failed !");
        return;
    }

    // Get the enumeration description of the function
    status = GXGetEnumDescription(device_handle, GX_ENUM_BALANCE_WHITE_AUTO, enum_description, &buffer_size);
    if (status != GX_STATUS_SUCCESS)
    {
        if (enum_description != NULL)
        {
            delete []enum_description;
            enum_description = NULL;
        }
    }
    GX_VERIFY(status);

    // Get the current value of the enumerated type and sets the value to the interface
    status = GXGetEnum(device_handle, GX_ENUM_BALANCE_WHITE_AUTO, &enum_value);
    if (status != GX_STATUS_SUCCESS)
    {
        if (enum_description != NULL)
        {
            delete []enum_description;
            enum_description = NULL;
        }
    }
    GX_VERIFY(status);
    m_auto_whitebalance_index = enum_value;

    // Initializes the options for the current control
    m_ui->WhiteBalanceComboBox->clear();
    for(uint32_t i = 0; i < entry_num; i++)
    {
        device_info.sprintf("%s", enum_description[i].szSymbolic);
        m_ui->WhiteBalanceComboBox->addItem(QString(device_info));
    }

    // Displays the camera's current settings
    m_ui->WhiteBalanceComboBox->setCurrentIndex(m_auto_whitebalance_index);

     // Release function resources
     if (enum_description != NULL)
     {
         delete []enum_description;
         enum_description = NULL;
     }
}

//---------------------------------------------------------------------------------
/**
\brief   Initialize the ExposureTime control
\param   device_handle[in]       camera handle
\return  void
*/
//----------------------------------------------------------------------------------
void CParameterDialog::InitShutterUI(GX_DEV_HANDLE device_handle)
{
    if ((m_ui->ExposurelineEdit == NULL) ||(m_ui->ExposureLabel == NULL) || (device_handle == NULL))
    {
        return;
    }

    GX_STATUS status = GX_STATUS_ERROR;
    QString   string_range = "";
    QString   string_float = "";
    double shutter_value = 0.0;

    //Restrict input
    QRegExp reg_shutter("[0-9]{1,7}");
    QRegExpValidator *shutter = new QRegExpValidator(reg_shutter,this);
    m_ui->ExposurelineEdit->setValidator(shutter);

    //Get the exposure range
    status = GXGetFloatRange(device_handle, GX_FLOAT_EXPOSURE_TIME, &m_shutter_float_range);
    GX_VERIFY(status);

    //Display range
    string_range.sprintf("ExposureTime(%.4f~%.4f)%s:", m_shutter_float_range.dMin, m_shutter_float_range.dMax, m_shutter_float_range.szUnit);
    m_ui->ExposureLabel->setText(string_range);

    // Get the current value and update to the interface
    status = GXGetFloat(device_handle, GX_FLOAT_EXPOSURE_TIME, &shutter_value);
    string_float.sprintf("%.0f", shutter_value);

    m_ui->ExposurelineEdit->setText(string_float);
    m_shutter_value = shutter_value;

    m_ui->ExposurelineEdit->setFocus();
}

//---------------------------------------------------------------------------------
/**
\brief   Initialize the gain control
\param   device_handle[in]       device handle
\return  void
*/
//----------------------------------------------------------------------------------
void CParameterDialog::InitGainUI(GX_DEV_HANDLE device_handle)
{
    if ((m_ui->GainlineEdit == NULL) ||(m_ui->GainLabel == NULL) || (device_handle == NULL))
    {
        return;
    }

    GX_STATUS status = GX_STATUS_ERROR;
    QString  string_range = "";
    QString  string_int = "";
    double gain_value = 0;

    //Restrict input
    QRegExp reg_gain("[0-9]{1,2}");
    QRegExpValidator *gain = new QRegExpValidator(reg_gain,this);
    m_ui->GainlineEdit->setValidator(gain);

    //Get the gain range
    status = GXGetFloatRange(device_handle, GX_FLOAT_GAIN, &m_gain_float_range);
    GX_VERIFY(status);

    //Display range
    string_range.sprintf("gain(%.4f~%.4f)dB:", m_gain_float_range.dMin, m_gain_float_range.dMax);
    m_ui->GainLabel->setText(string_range);

    //  Get the current value and update to the interface
    status = GXGetFloat(device_handle, GX_FLOAT_GAIN, &gain_value);
    string_int.sprintf("%.0f", gain_value);

    m_ui->GainlineEdit->setText(string_int);

    m_gain_value = gain_value;
}

//---------------------------------------------------------------------------------
/**
\brief   Initialize the acquisition speed level associated controls
\param   device_handle[in]       device handle
\return  void
*/
//----------------------------------------------------------------------------------
void CParameterDialog::InitSpeedLevel(GX_DEV_HANDLE device_handle)
{
    // Determine the validity of the control pointer
    if ((m_ui->SpeedLevelHorizontalSlider == NULL ||m_ui->SpeedLevellineEdit ==  NULL) || (device_handle == NULL))
    {
        return;
    }

    GX_STATUS status    = GX_STATUS_ERROR;
    int32_t   int_step       = 0;
    int64_t   speed_level = 0;
    bool        is_speed_level   = false;

    // whether the current camera supports the speed level
    status = GXIsImplemented(device_handle, GX_INT_ACQUISITION_SPEED_LEVEL, &is_speed_level);
    if (!is_speed_level)
    {
        m_ui->SpeedLevelHorizontalSlider->setDisabled(true);
        m_ui->SpeedLevellineEdit->setDisabled(true);
        return;
    }

    // Get the acquisition speed level range
    status = GXGetIntRange(device_handle, GX_INT_ACQUISITION_SPEED_LEVEL, &m_speedlevel_range);
    GX_VERIFY(status);
    m_ui->SpeedLevelHorizontalSlider->setRange(m_speedlevel_range.nMin, m_speedlevel_range.nMax);
    int_step = m_speedlevel_range.nInc;
    if (int_step == 0)
    {
        return;
    }

     // Get the current value and update to the interface
    status = GXGetInt(device_handle, GX_INT_ACQUISITION_SPEED_LEVEL, &speed_level);
    GX_VERIFY(status);
    m_ui->SpeedLevelHorizontalSlider->setSliderPosition((int32_t(speed_level / int_step)));
    m_ui->SpeedLevellineEdit->setText(QString::number(speed_level));
    m_speed_level = speed_level;
}

//----------------------------------------------------------------------------------
/**
\brief  The slot function that connect to the text box of Exposure Time value when it being edit finish.
\return void
*/
//----------------------------------------------------------------------------------
void CParameterDialog::on_ExposurelineEdit_editingFinished()
{
    GX_STATUS status = GX_STATUS_ERROR;
    double exposure_value;
    QString   string_float = "";

    exposure_value = m_ui->ExposurelineEdit->text().toFloat();

    //whether the input value is within the Exposure Time range
    //If the maximum value is greater than maximum value, the exposure value is set to the maximum value
    if (exposure_value > m_shutter_float_range.dMax)
    {
        exposure_value = m_shutter_float_range.dMax;
        //Update the displayed value
        string_float.sprintf("%.0f", exposure_value);
        m_ui->ExposurelineEdit->setText(string_float);
    }
    //If the value is less than the minimum, the Exposure Time value is set to the minimum value
    if (exposure_value < m_shutter_float_range.dMin)
    {
        exposure_value = m_shutter_float_range.dMin;
        //Update the displayed value
         string_float.sprintf("%.0f", exposure_value);
        m_ui->ExposurelineEdit->setText(string_float);
    }

    status = GXSetFloat(m_device_handle, GX_FLOAT_EXPOSURE_TIME, exposure_value);
    GX_VERIFY(status);
}

//----------------------------------------------------------------------------------
/**
\brief  The slot function that connect to the text box of gain value when it being edit finish.
\return void
*/
//----------------------------------------------------------------------------------
void CParameterDialog::on_GainlineEdit_editingFinished()
{
    GX_STATUS status = GX_STATUS_ERROR;
    double gain_value;
    QString string_float = "";

    gain_value = m_ui->GainlineEdit->text().toFloat();

    //Iwhether the input value is within the range of the gain value
    //If the maximum value is greater than maximum value, the gain value is set to the maximum value
    if (gain_value > m_gain_float_range.dMax)
    {
         gain_value = m_gain_float_range.dMax;
         //Update the displayed value
         string_float.sprintf("%.0f", gain_value);
         m_ui->GainlineEdit->setText(string_float);
     }

     //If the value is less than the minimum, the gain value is set to the minimum value
     if (gain_value < m_gain_float_range.dMin)
     {
        gain_value = m_gain_float_range.dMin;
        //Update the displayed value
        string_float.sprintf("%.0f", gain_value);
        m_ui->GainlineEdit->setText(string_float);
     }

     status = GXSetFloat(m_device_handle, GX_FLOAT_GAIN, gain_value);
     GX_VERIFY(status);
}

//----------------------------------------------------------------------------------
/**
\brief  /// The slot function connected to the combox of the Auto White Balance option
\return  void
*/
//------------------------------------------------------------------------------------
void CParameterDialog::on_WhiteBalanceComboBox_activated()
{
    GX_STATUS status = GX_STATUS_ERROR;
    int64_t   enum_value   = 0;
    bool flag = false;
    enum_value = (int64_t)m_ui->WhiteBalanceComboBox->currentIndex();
    m_auto_whitebalance_index = enum_value;

    status = GXIsImplemented(m_device_handle, GX_ENUM_BALANCE_WHITE_AUTO, &flag);
    if (flag == true)
    {
        //Set the auto white balance
        status = GXSetEnum(m_device_handle, GX_ENUM_BALANCE_WHITE_AUTO, enum_value);
        if (status != GX_STATUS_SUCCESS)
        {
            ShowErrorString(status);
            InitAutoWhiteBalanceUI(m_device_handle);
            return;
        }
    }
    m_timer->start(AB_TIME_OUT);
}

//----------------------------------------------------------------------------------
/**
\brief  The slot function connected to the horizontal slider acquisition of speed level slider movement
\param  position
\return  void
*/
//------------------------------------------------------------------------------------
void CParameterDialog::on_SpeedLevelHorizontalSlider_sliderMoved(int position)
{
    GX_STATUS status = GX_STATUS_ERROR;

    //Set the exposure time
    status = GXSetInt(m_device_handle, GX_INT_ACQUISITION_SPEED_LEVEL, position);

    if (status != GX_STATUS_SUCCESS)
    {
        //Get the current value
        status = GXGetInt(m_device_handle, GX_INT_ACQUISITION_SPEED_LEVEL, &m_speed_level);
        GX_VERIFY(status);
    }
    else
    {
        m_speed_level = position;
    }
}

//----------------------------------------------------------------------------------
/**
\brief  The slot function connected to the Acquisition speed level slider movement
\return void
*/
//----------------------------------------------------------------------------------
void CParameterDialog::on_SpeedLevellineEdit_editingFinished()
{
    m_speed_level = m_ui->SpeedLevellineEdit->text().toInt();
    m_ui->SpeedLevelHorizontalSlider->setSliderPosition(m_speed_level);
    on_SpeedLevelHorizontalSlider_sliderMoved(m_speed_level);
}

//----------------------------------------------------------------------------------
/**
\brief  The slot function connected to the automatic white balance interface update
\input
\output
\return  void
*/
//------------------------------------------------------------------------------------
void CParameterDialog::SlotAutoUpdateABbox()
{
    GX_STATUS status = GX_STATUS_SUCCESS;

    //If the auto white balance is Once, the actual white balance will automatically turn off when the setting is successful
    //The program updates the UI by periodically reading the current value of the internal white balance of the device
    if (m_auto_whitebalance_index == GX_BALANCE_WHITE_AUTO_ONCE)
    {
        int64_t    WB_enum_value = 0;

        //Get the auto white balance enumeration value
        status = GXGetEnum(m_device_handle, GX_ENUM_BALANCE_WHITE_AUTO, &WB_enum_value);
        if (status != GX_STATUS_SUCCESS)
        {
            return;
        }

        m_auto_whitebalance_index = WB_enum_value;

        //Determine whether the device auto white balance turns OFF
        if (m_auto_whitebalance_index == GX_BALANCE_WHITE_AUTO_OFF)
        {
           for (int i = 0; i < WHITE_BALANCE_RNTRY; i++)
           {
                if (m_ui->WhiteBalanceComboBox->itemText(i) == "Off")
                {
                    //Select the OFF item in the Auto Exposure control, which turns from ONCE to OFF
                    m_ui->WhiteBalanceComboBox->setCurrentIndex(i);
                    break;
                }
            }
        }
    }

    if(m_timer->isActive())
    {
        m_timer->stop();
    }
}

//----------------------------------------------------------------------------------
/**
\brief  Error message
\input  error_status    error code
\output
\return  void
*/
//------------------------------------------------------------------------------------
void CParameterDialog::ShowErrorString(GX_STATUS error_status)
{
    char*     error_info = NULL;
    size_t    size        = 0;
    GX_STATUS status     = GX_STATUS_ERROR;

    // Get the length of the error message and apply for memory space
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
        QMessageBox::about(NULL, "Error", " GXGetLastError Interface called failed ! ");
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
