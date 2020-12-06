QT -= gui
QT += websockets

CONFIG += c++14 console
CONFIG -= app_bundle

include($$PWD/libs/pruefungsplaner-datamodel/pruefungsplaner-datamodel.pri)
include($$PWD/libs/security-provider/client/client.pri)
include($$PWD/libs/qt-jsonrpc-server/qt-jsonrpc-server.pri)
INCLUDEPATH += $$PWD/libs/jwt-cpp/include
INCLUDEPATH += $$PWD/libs/cpptoml/include

LIBS += -lcrypto

SOURCES += \
        src/main.cpp \
        src/legacyscheduler.cpp \
        src/schedulerservice.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    src/legacyscheduler.h \
    src/scheduler.h \
    src/schedulerservice.h

test{
    message(Building tests)
    include(gtest_dependency.pri)

    QT += testlib
    TEMPLATE = app
    TARGET = pruefungsplaner-scheduler-tests
    
    CONFIG += thread
    LIBS += -lgtest -lgtest_main
    INCLUDEPATH += src

    SOURCES -= src/main.cpp
    SOURCES += tests/qthelper.cpp \
            tests/legacyschedulertest.cpp \
            tests/schedulerservicetest.cpp
}
else{
    message(Building app)
    TEMPLATE = app
    TARGET = pruefungsplaner-scheduler
}
