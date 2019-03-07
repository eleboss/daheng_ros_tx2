//---------------------------------------------------------------------------------------
/**
\file          SelectCameraDialog.h
\brief       Declarations for CSelectCameraDialog Class
\version   v1.0.1603.9011
\date        2016-03-01
*/
//---------------------------------------------------------------------------------------

#ifndef SELECTCAMERADIALOG_H
#define SELECTCAMERADIALOG_H

#include <QDialog>
#include "GxIAPI.h"
#include "MainWindow.h"

#define UPDATE_TIME_OUT  1000       ///<  Update the timeout of the device list
#define DEVICE_MAX_NUM 4              ///< Maximum number of cameras
#define min(a,b) ((a)<(b))?(a):(b);

class Ui_SelectCameraDialog;
class MainWindow;

class CSelectCameraDialog: public QDialog
{
    Q_OBJECT

public:
    /// Constructor
    explicit CSelectCameraDialog(QWidget* parent = 0);

    /// Destructor
    ~CSelectCameraDialog(void);

    /// Initialize Dialog
    bool InitDialog(void);

    /// Update the device list
    void UpdateDeviceList(void);

    /// Show Error message
    void ShowErrorString(GX_STATUS error_status);

    /// Assign resources to the main window
    bool AllocBufferForMainFrame(void);

    /// Clear Buffer
    void ClearBuffer(void);

    /// Traverse all the cameras to close
    GX_STATUS CloseCamer();

private:
    Ui_SelectCameraDialog* m_select_ui;
    MainWindow *                 m_main_frame;                  ///< Main window pointer
    int                                  m_device_index;                ///<  The index of current selected device  
    uint32_t                           m_device_number;            ///<  the number of devices

public slots:
    /// The slot function connected to the button of Re-enumerate the device.
    void on_pushButtonupdate_clicked(void);

    /// The slot function connected to  the button of OK.
    void on_pushButtonok_clicked(void);

    /// The slot function connected to the button of cancel.
    void on_pushButtoncancel_clicked(void);

    /// The slot function connected to the comboBox of selecting the list item.
    void on_comboBox_activated(void);
};

#endif // SELECTCAMERADIALOG_H
