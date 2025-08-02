# +=========================================================================================+
# | RedPandaIDE Version 3.4.0 Notice                                                        |
# | beta3   : (abandoned)Add zipped file support in ExtensionManager                        |
# | beta4   : Add the latest C++26 grammar support                                          |
# | RC1     : (abandoned)fix the bug of 'using namespace'                                   |
# | RC2     : (abandoned)Upload MinGW64 15.1.0 to installer(online)                         |
# | Release : Release the version if all tests passed                                       |
# |                                                                                         |
# | Notice: the beta3 plan abandoned because the author can't find a library to unpack zip  |
# |         file.This plan will join in v4.0.0, and just support in Modern Edition, because |
# |         we plan to use system command to unpack but win7 or earlier didn't support it.  |
# |         In LightEdition, we plan to save theme and color scheme download support for it.|
# |         The RC1 version abandoned because it not easy to fix.                           |
# |         The RC2 version abandoned because it not support windows7 or earlier.           |
# +-----------------------------------------------------------------------------------------+
# | RedPandaIDE Version 4.0.0 Notice                                                        |
# | After v3.4.x, we'll release 4.0 version.                                                |
# | Soluction of Qt6 Quesion based on: https://github.com/msys2/MINGW-packages/issues/18966 |
# | Enable the BUILD_WITH_QT6 macro will enable the qt6 feature.                            |
# | Compilers always build the Light Edition, build Modern Edition must enable BUILD_MODERN |
# | macro.                                                                                  |
# +=========================================================================================+

TEMPLATE = subdirs

SUBDIRS += \
    RedPandaIDE \
    consolepauser \
    redpanda_qt_utils \
    qsynedit \
    lua \

consolepauser.subdir = tools/consolepauser
redpanda_qt_utils.subdir = libs/redpanda_qt_utils
qsynedit.subdir = libs/qsynedit
lua.subdir = libs/lua
#qmarkdowntextedit.subdir = libs/qmarkdowntextedit

# OpenSSL static link config  (for MSYS2)
contains(DEFINES, BUILD_INCLUDE_OPENSSL) {
    win32 {
        # path of OpenSSL in MSYS2
        MSYS2_ROOT = /
        OPENSSL_ROOT = $${MSYS2_ROOT}msys64/mingw64

        INCLUDEPATH += $${OPENSSL_ROOT}/include

        LIBS += -L$${OPENSSL_ROOT}/lib
        LIBS += -llibeay32
        LIBS += -lssleay32

        # Windows depends
        LIBS += -lcrypt32 -lws2_32
    }

    unix:!macos {
        LIBS += -lssl -lcrypto
        LIBS += -ldl -lpthread
    }
}

CONFIG += static
DEFINES += OPENSSL_NO_ENGINE
RedPandaIDE.depends = consolepauser qsynedit lua

# Qt6 Feature
contains(DEFINES, BUILD_WITH_QT6) {
    include(qt6_feature.pri)
}

# Add the dependencies so that the RedPandaIDE project can add the depended programs
# into the main app bundle
qsynedit.depends = redpanda_qt_utils

APP_NAME = RedPandaIDE
include(version.inc)
#include($${qmarkdowntextedit.subdir}/qmarkdowntextedit.pri)

!isEmpty(APP_VERSION_SUFFIX): {
    APP_VERSION = "$${APP_VERSION}$${APP_VERSION_SUFFIX}"
}

# win32: {
# SUBDIRS += \
#     redpanda-win-git-askpass
# redpanda-win-git-askpass.subdir = tools/redpanda-win-git-askpass
# RedPandaIDE.depends += redpanda-win-git-askpass
# }

# unix: {
# SUBDIRS += \
#     redpanda-git-askpass
#     redpanda-git-askpass.subdir = tools/redpanda-git-askpass
#     RedPandaIDE.depends += redpanda-git-askpass
# }

unix:!macos: {
    isEmpty(PREFIX) {
        PREFIX = /usr/local
    }
    isEmpty(LIBEXECDIR) {
        LIBEXECDIR = libexec
    }

    QMAKE_SUBSTITUTES += platform/linux/RedPandaIDE.desktop.in

    resources.path = $${PREFIX}/share/$${APP_NAME}
    resources.files += platform/linux/templates
    INSTALLS += resources

    docs.path = $${PREFIX}/share/doc/$${APP_NAME}
    docs.files += README.md
    docs.files += NEWS.md
    docs.files += LICENSE
    INSTALLS += docs

    xdgicons.path = $${PREFIX}/share/icons/hicolor/scalable/apps/
    xdgicons.files += platform/linux/redpandaide.svg
    INSTALLS += xdgicons

    desktop.path = $${PREFIX}/share/applications
    desktop.files += platform/linux/RedPandaIDE.desktop
    INSTALLS += desktop

    mime.path = $${PREFIX}/share/mime/packages
    mime.files = platform/linux/redpandaide.xml
    INSTALLS += mime
}

win32: {
    !isEmpty(PREFIX) {
        target.path = $${PREFIX}

        resources.path = $${PREFIX}

        resources.files += platform/windows/templates
        resources.files += platform/windows/qt.conf
        resources.files += README.md
        resources.files += NEWS.md
        resources.files += LICENSE
        resources.files += RedPandaIDE/images/devcpp.ico

        INSTALLS += resources

        equals(X86_64, "ON") {
            extra_templates.path = $${PREFIX}/templates
            extra_templates.files += platform/windows/templates-win64/*
            INSTALLS += extra_templates
        }
    }
}
