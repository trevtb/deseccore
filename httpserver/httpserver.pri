INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

QT += network

HEADERS += $$PWD/httplistener.h \
           $$PWD/httpconnectionhandler.h \
           $$PWD/httpconnectionhandlerpool.h \
           $$PWD/httprequest.h \
           $$PWD/httpresponse.h \
           $$PWD/httpcookie.h \
           $$PWD/httprequesthandler.h \
           $$PWD/httpsession.h \
           $$PWD/httpsessionstore.h \
           $$PWD/staticfilecontroller.h

SOURCES += $$PWD/httplistener.cpp \
           $$PWD/httpconnectionhandler.cpp \
           $$PWD/httpconnectionhandlerpool.cpp \
           $$PWD/httprequest.cpp \
           $$PWD/httpresponse.cpp \
           $$PWD/httpcookie.cpp \
           $$PWD/httprequesthandler.cpp \
           $$PWD/httpsession.cpp \
           $$PWD/httpsessionstore.cpp \
           $$PWD/staticfilecontroller.cpp
