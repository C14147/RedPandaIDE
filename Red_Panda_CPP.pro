# +============================ RedPandaIDE Version 4.0.0 Notice ============================+
# | Soluction of Qt6 Quesion based on: https://github.com/msys2/MINGW-packages/issues/18966  |
# |---------------------------------- Compilation Options -----------------------------------|
# | BUILD_MODERN macro: Enable the qt6 feature and build Modern Edition.                     |
# |                     Compilers will build the Light Edition defaultly.                    |
# | Attention: The Extension Manager will only enabled on Modern Edition.                    |
# |----------------------------- Modern Edition Plan & Features -----------------------------|
# | 1. framework upgrade preview                                                             |
# |   - Build with Qt6 Framework.                                                            |
# | 2. alpha 1                                                                               |
# |   - Add complete support for extensions to compressed file types.                        |
# |   - update compiler version to MinGW64 15.1.0                                            |
# | 3. alpha 2                                                                               |
# |   - Intelligent recognition of compiler versions and addition of additional compilation  |
# |     options for specific versions of compilers.                                          |
# |     (e.g. -fmodules-ts for MinGW 11.x, -fmodules for later)                              |                                                                               |
# | 4. new feature preview 1                                                                 |
# |   - Add analysis of C/C++ keywords and prompt users to enable C/C++ standard or          |
# |     precompile modules.                                                                  |
# |----------------------------- Plan & Features For All Edition ----------------------------|
# | 1. alpha 1                                                                               |
# |   - Refactor the appearance of the embedded terminal, and add environment variables such |
# |     as compiler path and IDE path when the embedded terminal starts up.                  |
# | 2. alpha 2                                                                               |
# |   - Optimize code prompt algorithm to improve code parsing speed.                        |
# | 3. new feature preview 2                                                                 |
# |   - Save parsing records when using a library for the first time and read them if        |
# |     necessary (switching to a new compiler will delete previous parsing records).        |
# |   - Choose whether to save parsing in the settings.                                      |
# | 3. beta 1: when upstream fixed over 5 issues and alpha plans are done, will publish this.|
# | 4. beta 2: try to fix some issues.                                                       |
# | 5. RC 1: Disable Extension Manager in Light Edition.                                     |
# +==========================================================================================+

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
contains(DEFINES, BUILD_MODERN) {
    contains(QT, widgets){
        MSYSTEM_PREFIX=$$(MSYSTEM_PREFIX)
        greaterThan(MSYSTEM_PREFIX,' '){
            contains(CONFIG, static) {
	            message("STATIC Qt6 with MSYS2. Extra patch should be introduced.");
                CONFIG += no_lflags_merge
	            LIBS += -ltiff  -lmng.dll -ljpeg -ljbig -ldeflate  -lzstd -llerc -llzma  -lgraphite2 -lbz2 -lusp10 -lRpcrt4 -lsharpyuv -lOleAut32
                #LIBS += -lbz2
            }
        }
    }
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
