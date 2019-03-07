//--------------------------------------------------------------------------------
/**
\file          AboutDialog.cpp
\brief       AboutDialog Function
\version   v1.0.1808.9141
\date        2018-08-14
*/
//---------------------------------------------------------------------------------

#include "AboutDialog.h"
#include "ui_aboutdialog.h"
#include "UIDef.h"

//----------------------------------------------------------------------------------
/**
\brief  Constructor
\input
\output
\return
*/
//------------------------------------------------------------------------------------
CAboutDialog::CAboutDialog(QWidget* parent)
    :QWidget(parent)
    ,m_ui(new Ui_AboutDialog)
{
    m_ui->setupUi(this);
    this->InitDialog();
}

//----------------------------------------------------------------------------------
/**
\brief  Destructor
\input
\output
\return
*/
//------------------------------------------------------------------------------------
CAboutDialog::~CAboutDialog(void)
{

}

//----------------------------------------------------------------------------------
/**
\brief  Initialize 
\input
\output
\return success: true,  fail: false
*/
//------------------------------------------------------------------------------------
bool CAboutDialog::InitDialog(void)
{
    QFont ft;
    ft.setPointSize(10);
    m_ui->VersionLabel->setFont(ft);
    m_ui->VersionLabel->setText("version:1.0.1808.9141");
    m_ui->CopyrightLabel->setFont(ft);
    m_ui->CopyrightLabel->setText("copyright (C)  2015-2018");
}

//----------------------------------------------------------------------------------
/**
\brief  The slot function connected to the buttion of ok
\input
\output
\return  void
*/
//------------------------------------------------------------------------------------
void CAboutDialog::on_pushButtonok_clicked(void)
{
    this->close();
}
