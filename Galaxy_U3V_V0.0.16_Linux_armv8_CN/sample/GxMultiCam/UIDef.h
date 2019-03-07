//---------------------------------------------------------------------------------------
/**
\file          UIDef.h
\brief       Some Definition Used in Other Files
\version   v1.0.1603.9011
\date        2016-03-01
*/
//---------------------------------------------------------------------------------------

#ifndef UIDEF_H
#define UIDEF_H

#include <QDebug>


/// If the result is false, the loop is bounced, otherwise it is not processed
#define UI_CHECK_BOOL_RESULT(result) \
if(!result)\
{\
   break;\
}

/// If the result is NULL, the loop is bounced, otherwise it is not processed
#define UI_CHECK_NEW_MEMORY(new_memory)\
if(NULL == new_memory)\
{\
   break;\
}

/// Print error
#define UI_DEBUG_PRINT \
    qDebug()<<"Error ocurred at "<<__FILE__<<" "\
    <<__LINE__ <<" " <<__FUNCTION__

/// Print warning
#define UI_DEBUG_WARNING(msg)  \
    qDebug()<<__FILE__<<"("<<__LINE__ <<")"<<__FUNCTION__<<" warning:"<<msg;

/// Error message of function macro definition
#define  GX_VERIFY(status) \
         if(status != GX_STATUS_SUCCESS) \
         { \
            ShowErrorString(status); \
            return ;\
         }

#define DH_TRACE(...) \
do \
{ \
    fprintf(stderr, " TRACE  (%s|%s|%d): ", __FUNCTION__, __func__, __LINE__); \
    fprintf(stderr, __VA_ARGS__); \
} \
while(0)

#endif // UIDEF_H
