/* pluginutil.h - Provide any interface that can be used for plugins.
 *
 *   We will provide the PluginUtil class here, which is used to obtain
 * software information, capture window signals, and use some internal
 * functions of the software for various plugins connected to RedPandaIDE.
 * Developers can use this file to obtain long-term stable support,
 * because when the internal logic of the software changes, the interfaces
 * provided here will hardly be modified unless there are significant changes
 * in the software. Only then will we write the next version of the PluginUtil
 * class, and support for the previous version of the PluginUtil class will be
 * discontinued.
 *
 * Notice:
 * If you want to make deeper modifications to the software, you can directly
 * obtain all variables of the software without using the interfaces provided
 * here. However, we cannot guarantee compatibility when launching new versions
 * of the software.
 */

#ifndef PLUGINUTIL_H
#define PLUGINUTIL_H

#include <QMainWindow>
#include <QObject>

class PluginUtil : public QObject
{
    Q_OBJECT
public:
    explicit PluginUtil(QObject* parent = nullptr);

signals:
};

#endif // PLUGINUTIL_H
