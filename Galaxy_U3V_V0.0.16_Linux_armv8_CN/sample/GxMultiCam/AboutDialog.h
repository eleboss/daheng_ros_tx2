//---------------------------------------------------------------------------------------
/**
\file          AboutDialog.h
\brief       Declarations for CAboutDialog Class
\version   v1.0.1603.9011
\date        2016-03-01
*/
//---------------------------------------------------------------------------------------

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QLabel>

class Ui_AboutDialog;

class CAboutDialog: public QWidget
{
    Q_OBJECT

public:
    /// Constructor
    explicit CAboutDialog(QWidget* parent = 0);

    /// Destructor
    ~CAboutDialog(void);

    /// The Initialize function
    bool InitDialog(void);

private:
    Ui_AboutDialog*   m_ui;

public slots:
    /// The slot function connected to the buttion of ok
    void on_pushButtonok_clicked(void);

};

#endif // ABOUTDIALOG_H
