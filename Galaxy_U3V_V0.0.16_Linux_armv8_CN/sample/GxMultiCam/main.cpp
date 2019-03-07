//---------------------------------------------------------------------------------------
/**
\file          main.cpp
\brief       Main Function
\version   v1.0.1603.9011
\date        2016-03-01
*/
//---------------------------------------------------------------------------------------

#include <QApplication>
#include <QMessageBox>
#include "MainWindow.h"
#include <QTextCodec>


int main(int argc, char *argv[])
{
    QApplication object_app(argc, argv);

    //Set the Qt application style
    //QApplication::setStyle("windows");   // or windows etc.

    MainWindow object_dialog;
    //Set the form mode
    object_dialog.setWindowModality(Qt::ApplicationModal);
    //Fixed form size
    object_dialog.setFixedSize(object_dialog.width(), object_dialog.height());

    object_dialog.show();

    return object_app.exec();
}

