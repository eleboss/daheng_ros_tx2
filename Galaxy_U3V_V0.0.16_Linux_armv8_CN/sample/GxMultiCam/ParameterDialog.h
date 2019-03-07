//---------------------------------------------------------------------------------------
/**
\file          ParameterDialog.h
\brief       Declarations for CParameterDialog Class
\version   v1.0.1603.9011
\date        2016-03-01
*/
//---------------------------------------------------------------------------------------

#ifndef PARAMETERDIALOG_H
#define PARAMETERDIALOG_H

#include <QDialog>
#include <QSlider>
#include <QComboBox>
#include <QLabel>
#include "GxIAPI.h"

#define AB_TIME_OUT 1000
#define WHITE_BALANCE_RNTRY 3

class Ui_ParameterDialog;

class CParameterDialog: public QWidget
{
    Q_OBJECT

public:
    /// Constructor
    explicit CParameterDialog(QWidget* parent = 0);

    /// Destructor
    ~CParameterDialog(void);

private:
     Ui_ParameterDialog*   m_ui;
     GX_DEV_HANDLE       m_device_handle;                   ///< camera handle
     QTimer*                       m_timer;                                 ///< Timers
    bool                               m_auto_whitebalance;            ///< Whether support White Balance
    int64_t                           m_auto_whitebalance_index; ///< Auto white balance current option
    int64_t                           m_speed_level;                      ///< Acquisition speed level
    double	                          m_shutter_value;                   ///< Exposure time value
    double                           m_gain_value;                        ///< Gain value
    GX_FLOAT_RANGE       m_gain_float_range;               ///< Gain range
    GX_FLOAT_RANGE       m_shutter_float_range;           ///< Exposure time range
    GX_INT_RANGE            m_speedlevel_range;             ///< Acquisition speed range

public:

    /// Initialize the overall UI
    void InitUI(GX_DEV_HANDLE device_handle);

    /// Initialize the enumeration type UI
    void InitAutoWhiteBalanceUI(GX_DEV_HANDLE device_handle);

     /// Initialize the exposure time control
     void InitShutterUI(GX_DEV_HANDLE device_handle);

     /// Initialize the gain control
     void InitGainUI(GX_DEV_HANDLE device_handle);

     /// Initialize the acquisition speed level controls
     void InitSpeedLevel(GX_DEV_HANDLE device_handle);

     /// Error message
     void ShowErrorString(GX_STATUS error_status);

 public slots:

     /// The slot function connected to combox of the Auto White Balance option
     void on_WhiteBalanceComboBox_activated();

     /// The slot function that connect to the text box of exposure value when it being edit finish
     void on_ExposurelineEdit_editingFinished();

     /// The slot function that connect to the text box of gain value when it being edit finish
     void on_GainlineEdit_editingFinished();

     /// The slot function connected to horizontal slider of the acquisition speed level slider movement
     void on_SpeedLevelHorizontalSlider_sliderMoved(int position);

     /// The slot function connected to the acquisition speed level slider movement
     void on_SpeedLevellineEdit_editingFinished();

     /// The slot function connected to the auto white balance interface update
     void SlotAutoUpdateABbox();

};


#endif // PARAMETERDIALOG_H
