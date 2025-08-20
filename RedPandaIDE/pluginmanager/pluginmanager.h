/*
 * @file pluginmanager.h
 *
 * meta data file ({PluginName}.json) template:
 * {
 *  "pluginName": "Plugin API",
 *  "pluginVersion": "1.0.0",
 *  "introduction": "the core api for all plugin",
 *  "ID" : "com.redpandaide.apicore.1.0.0"
 * }
 */

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
#include "../utils.h"

class IRedPandaPlugin;
class MainWindow;

class PluginManager : public QObject
{
    Q_OBJECT
public:
    explicit PluginManager(QObject* parent = nullptr);
    ~PluginManager();

    // load all plugins from a folder (non-recursive)
    QMap<QString, QString> loadPlugins(const QString& folder, const bool showUI = false);

    // load a single plugin by absolute path
    QString loadPlugin(const QString& path);

    // unload a previously loaded plugin by path
    bool unloadPlugin(const QString& path);

    // reload a plugin (unload then load)
    bool reloadPlugin(const QString& path);

    // list of loaded plugin instances
    const QList<IRedPandaPlugin*>& plugins() const;

    // list plugin file paths found in folder
    QStringList availablePlugins(const QString& folder) const;

    // get metadata parsed from plugin json file for a loaded plugin
    QVariantMap pluginMetadata(const QString& path) const;

    // check the depends if loaded
    size_t processDepends() noexcept;

    IRedPandaPlugin* findPluginByID(const QString& pluginID) noexcept;
    IRedPandaPlugin* findPluginByName(const QString& pluginName) noexcept;

signals:
    void pluginLoaded(IRedPandaPlugin* plugin, const QString& path);
    void pluginUnloaded(IRedPandaPlugin* plugin, const QString& path);

public:
    struct PluginRecord {
        QString path;
        QPluginLoader* loader{nullptr};
        IRedPandaPlugin* plugin{nullptr};
        QList<QAction*> toolActions;
        QList<QWidget*> explorerTabs;
        QList<QWidget*> messagesTabs;
        QList<class SettingsWidget*> settingsWidgets;
        QVariantMap metadata; // parsed from plugin_name.json if present
    };

    QList<PluginRecord> mRecords;
    QSet<QString> unprocessedDepends;
};

#endif // PLUGINMANAGER_H
