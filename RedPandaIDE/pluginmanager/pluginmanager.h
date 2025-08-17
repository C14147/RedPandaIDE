#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QObject>
#include <QList>
#include <QDir>
#include <QStringList>
#include <QPluginLoader>
#include <QAction>
#include <QWidget>

#include "settingsdialog/settingswidget.h"

class IRedPandaPlugin;
class MainWindow;

class PluginManager : public QObject
{
    Q_OBJECT
public:
    explicit PluginManager(MainWindow *mainWindow, QObject *parent = nullptr);
    ~PluginManager();

    // load all plugins from a folder (non-recursive)
    void loadPlugins(const QString &folder);

    // load a single plugin by absolute path
    bool loadPlugin(const QString &path);

    // unload a previously loaded plugin by path
    bool unloadPlugin(const QString &path);

    // reload a plugin (unload then load)
    bool reloadPlugin(const QString &path);

    // list of loaded plugin instances
    const QList<IRedPandaPlugin*> &plugins() const;

    // list plugin file paths found in folder
    QStringList availablePlugins(const QString &folder) const;

    // get metadata parsed from plugin json file for a loaded plugin
    QVariantMap pluginMetadata(const QString &path) const;

signals:
    void pluginLoaded(IRedPandaPlugin* plugin, const QString &path);
    void pluginUnloaded(IRedPandaPlugin* plugin, const QString &path);

private:
    MainWindow *mMainWindow;
    QVariantMap metadata; // parsed from plugin_name.json if present
    struct PluginRecord {
        QString path;
        QPluginLoader *loader{nullptr};
        IRedPandaPlugin *plugin{nullptr};
        QList<QAction*> toolActions;
        QList<QWidget*> explorerTabs;
        QList<QWidget*> messagesTabs;
        QList<class SettingsWidget*> settingsWidgets;
    };

    QList<PluginRecord> mRecords;
};

#endif // PLUGINMANAGER_H
