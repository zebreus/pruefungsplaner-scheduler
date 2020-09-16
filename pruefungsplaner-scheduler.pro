QT -= gui
QT += websockets

CONFIG += c++11 console
CONFIG -= app_bundle

include($$PWD/libs/pruefungsplaner-datamodel/pruefungsplaner-datamodel.pri)
include($$PWD/libs/security-provider/client/client.pri)
include($$PWD/libs/qt-jsonrpc-server/qt-jsonrpc-server.pri)
INCLUDEPATH += $$PWD/libs/jwt-cpp/include

LIBS += -lcrypto

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
        src/plancsvhelper.cpp \
        src/schedulerservice.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    src/plancsvhelper.h \
    src/scheduler.h \
    src/schedulerservice.h
