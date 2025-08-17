#include "pluginmanager.h"
#include "plugininterface.h"
#include <QPluginLoader>
#include <QDebug>
#include <QFileInfo>
#include <QAction>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>


PluginManager::PluginManager(MainWindow *mainWindow, QObject *parent)
    : QObject(parent), mMainWindow(mainWindow)
{
}

PluginManager::~PluginManager()
{
    // unload all plugins and free loaders
    for (auto &r : mRecords) {
        if (r.loader) {
            r.loader->unload();
            delete r.loader;
            r.loader = nullptr;
        }
        r.plugin = nullptr;
    }
}

void PluginManager::loadPlugins(const QString &folder)
{
    QDir dir(folder);
    if (!dir.exists())
        return;

    auto entryList = dir.entryList(QDir::Files);
    for (const QString &file : entryList) {
        QString path = dir.absoluteFilePath(file);
        loadPlugin(path);
    }
}

const QList<IRedPandaPlugin*> &PluginManager::plugins() const
{
    static QList<IRedPandaPlugin*> list;
    list.clear();
    for (const auto &r : mRecords) {
        if (r.plugin)
            list.append(r.plugin);
    }
    return list;
}

bool PluginManager::loadPlugin(const QString &path)
{
    QFileInfo fi(path);
    if (!fi.exists()) return false;
    // prevent duplicate loads
    for (const auto &r : mRecords) {
        if (r.path == fi.absoluteFilePath()) return true;
    }

    QPluginLoader *loader = new QPluginLoader(fi.absoluteFilePath(), this);
    QObject *instance = loader->instance();
    if (!instance) {
        qDebug() << "Plugin load failed:" << path << loader->errorString();
        delete loader;
        return false;
    }
    IRedPandaPlugin *plugin = qobject_cast<IRedPandaPlugin*>(instance);
    if (!plugin) {
        // not our plugin
        loader->unload();
        delete loader;
        return false;
    }
    plugin->initialize(mMainWindow);
    PluginRecord rec;
    rec.path = fi.absoluteFilePath();
    rec.loader = loader;
    rec.plugin = plugin;

    // Try to read metadata file alongside plugin binary: <basename>.json
    QString metaPath = fi.absolutePath() + QDir::separator() + fi.completeBaseName() + ".json";
    QFile mf(metaPath);
    if (mf.open(QFile::ReadOnly | QFile::Text)) {
        QByteArray data = mf.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isObject()) {
            rec.metadata = doc.object().toVariantMap();
        }
        mf.close();
    }

    // store contributed UI for bookkeeping; actual integration into UI is handled by MainWindow
    rec.toolActions = plugin->toolActions();
    for (const auto &p : plugin->explorerTabs()) {
        rec.explorerTabs.append(p.second);
    }
    for (const auto &p : plugin->messagesTabs()) {
        rec.messagesTabs.append(p.second);
    }
    rec.settingsWidgets = plugin->settingsWidgets();

    mRecords.append(rec);

    emit pluginLoaded(plugin, rec.path);
    return true;
}

bool PluginManager::unloadPlugin(const QString &path)
{
    QString abs = QFileInfo(path).absoluteFilePath();
    for (int i = 0; i < mRecords.size(); ++i) {
        auto &r = mRecords[i];
        if (r.path == abs) {
            // ask MainWindow to remove UI; emit signal and let main window decide
            emit pluginUnloaded(r.plugin, r.path);

            if (r.loader) {
                bool ok = r.loader->unload();
                delete r.loader;
                r.loader = nullptr;
                r.plugin = nullptr;
                mRecords.removeAt(i);
                return ok;
            }
            mRecords.removeAt(i);
            return true;
        }
    }
    return false;
}

bool PluginManager::reloadPlugin(const QString &path)
{
    QString abs = QFileInfo(path).absoluteFilePath();
    unloadPlugin(abs);
    return loadPlugin(abs);
}

QStringList PluginManager::availablePlugins(const QString &folder) const
{
    QStringList out;
    QDir dir(folder);
    if (!dir.exists()) return out;
    auto entryList = dir.entryList(QDir::Files);
    for (const QString &file : entryList) {
        out.append(dir.absoluteFilePath(file));
    }
    return out;
}

QVariantMap PluginManager::pluginMetadata(const QString &path) const
{
    QString abs = QFileInfo(path).absoluteFilePath();
    for (const auto &r : mRecords) {
        if (r.path == abs) return r.metadata;
    }
    return {};
}
