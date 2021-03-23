QT -= gui
QT += websockets

CONFIG += c++2a console
CONFIG -= app_bundle

include($$PWD/libs/pruefungsplaner-datamodel/pruefungsplaner-datamodel.pri)
include($$PWD/libs/pruefungsplaner-auth/client/client.pri)
include($$PWD/libs/qt-jsonrpc-server/qt-jsonrpc-server.pri)
INCLUDEPATH += $$PWD/libs/jwt-cpp/include
INCLUDEPATH += $$PWD/libs/cpptoml/include

LIBS += -lcrypto

SOURCES += \
        src/configuration.cpp \
        src/main.cpp \
        src/legacyscheduler.cpp \
        src/schedulerservice.cpp

HEADERS += \
    src/configuration.h \
    src/legacyscheduler.h \
    src/scheduler.h \
    src/schedulerservice.h

test{
    message(Building tests)
    include($$PWD/libs/gtest/gtest_dependency.pri)

    QT += testlib
    TEMPLATE = app
    TARGET = pruefungsplaner-scheduler-tests
    
    CONFIG += thread
    LIBS += -lgtest
    INCLUDEPATH += src

    SOURCES -= src/main.cpp
    SOURCES += tests/qthelper.cpp \
            tests/legacyschedulertest.cpp \
            tests/schedulerservicetest.cpp \
            libs/gtest/main.cpp
}
else{
    message(Building app)
    TEMPLATE = app
    TARGET = pruefungsplaner-scheduler
}

unix{
    # Install executable
    target.path = /usr/bin

    spa_algorithm.path = /usr/bin
    spa_algorithm.files = SPA-algorithmus

    # Install default config file
    config.path = /etc/$${TARGET}/
    config.files = res/config.toml

    # Create data directory
    datadir.path = /usr/share/$${TARGET}
    datadir.extra = " "
    datadir.uninstall = " "

    # Create directory for keys
    keys.path = $${datadir.path}/keys
    keys.extra = " "
    keys.uninstall = " "

    # Create directory for storage
    storage.path = $${datadir.path}/data
    storage.extra = " "
    storage.uninstall = " "
}

!isEmpty(target.path): INSTALLS += target
!isEmpty(spa_algorithm.path): INSTALLS += spa_algorithm
!isEmpty(config.path): INSTALLS += config
!isEmpty(keys.path): INSTALLS += keys
!isEmpty(storage.path): INSTALLS += storage
!isEmpty(datadir.path): INSTALLS += datadir

DEFINES += DEFAULT_CONFIG_PATH=\"\\\"$${config.path}\\\"\" \
           DEFAULT_STORAGE_PATH=\"\\\"$${storage.path}\\\"\"
