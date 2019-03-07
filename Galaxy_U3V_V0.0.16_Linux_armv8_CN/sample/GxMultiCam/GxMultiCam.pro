QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

LIBS += -lgxiapi \

INCLUDEPATH += ./include/

FORMS += \
    parameterdialog.ui \
    selectcameradialog.ui \
    mainwindow.ui \
    aboutdialog.ui

HEADERS += $$files(./include/*.h)

HEADERS += \
    UIDef.h \
    MainWindow.h \
    ParameterDialog.h \
    ChildWindow.h \
    SelectCameraDialog.h \
    AboutDialog.h \
    Fps.h

SOURCES += \
    main.cpp \
    MainWindow.cpp \
    ParameterDialog.cpp \
    ChildWindow.cpp \
    SelectCameraDialog.cpp \
    AboutDialog.cpp \
    Fps.cpp

QMAKE_CXXFLAGS  +=  -w
