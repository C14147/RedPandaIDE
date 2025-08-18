//
// Copyright (C) 2025 C14147 (C14147@github.com)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

/* 
 * RedPandaIDE Plugin System - Utility functions for plugins
 * @file pluginUtils.h
 * @brief Utility functions for plugin to call software functions.
 */

#ifndef PLUGINUTILS_H
#define PLUGINUTILS_H

namespace {
    #include "../utils.h"
    #include "pluginmanager.h"
    #include "../mainwindow.h"
}

namespace RedPandaIDE {

QPixmap HDPixmap(QPixmap pix, int w, int h)
{
#if QT_VERSION_MAJOR == 6
    QPixmap orgin = pix;
    QScreen *screen = QGuiApplication::primaryScreen();

    orgin.setDevicePixelRatio(screen->devicePixelRatio());
    pix = orgin.scaled(
        w,h,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation);
#endif
    return pix.scaled(w,h);
}

QPixmap HDPixmap(QPixmap pix)
{
    return HDPixmap(pix,pix.width(),pix.height());
}

QPixmap HDPixmap(QString picPath)
{
    return HDPixmap(QPixmap(picPath));
}

QPixmap HDPixmap(QString picPath, int w, int h)
{
    QPixmap pix(picPath);
    return HDPixmap(pix,w,h);
}

IRedPandaPlugin* getPluginByName(const QString &name){
    PluginManager *pm = MainWindow::instance()->pluginManager();
    if (!pm) return nullptr;
    for (IRedPandaPlugin *plugin : pm->plugins()) {
        if (plugin && plugin->metaObject()->className() == name) {
            return plugin;
        }
    return nullptr;
}

int showPluginNotFound(const QString &name){
    if( QMessageBox::warning(
            MainWindow::instance(),
            QObject::tr("Plugin Not Found"),
            QObject::tr("Plugin %1 not found. Do you want to install it?").arg(name),
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        return 1; // todo
    else return 0;
}

}


#endif // PLUGINUTILS_H