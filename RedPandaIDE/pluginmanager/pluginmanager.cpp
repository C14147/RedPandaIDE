#include "pluginmanager.h"

#include <QAction>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPluginLoader>

#include "../mainwindow.h"
#include "plugininterface.h"

PluginManager::PluginManager(QObject* parent) : QObject(parent)
{
}

PluginManager::~PluginManager()
{
    // unload all plugins and free loaders
    for (auto& r : mRecords) {
        if (r.loader) {
            r.loader->unload();
            delete r.loader;
            r.loader = nullptr;
        }
        r.plugin = nullptr;
    }
}

QMap<QString, QString> PluginManager::loadPlugins(const QString& folder, const bool showUI)
{
    QMap<QString, QString> failed;
    failed.clear();

    QDir dir(folder);
    if (!dir.exists()) {
        failed[dir.absolutePath()] = "The path doesn't exists.";
        return failed;
    }

    WaitingWidget w;
    if (showUI)
        w.show();

    auto entryList = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    if (showUI) {
        w.progressBar->setMaximum(entryList.size());
        w.progressBar->setValue(0);
        w.update();
    }

    for (const QString& pluginPath : entryList) {
        QString path = dir.absolutePath() + QDir::separator() + pluginPath;
        QString res = loadPlugin(path);
        if (res != "Success")
            failed[pluginPath] = res;

        if (showUI) {
            w.progressBar->setValue(w.progressBar->value() + 1);
            w.update();
        }
    }

    if (processDepends()) {
        failed[QObject::tr("Can't find depends")] = unprocessedDepends.values().join(", ");
    }

    if (showUI) {
        w.hide();
        w.deleteLater();
    }

    return failed;
}

const QList<IRedPandaPlugin*>& PluginManager::plugins() const
{
    static QList<IRedPandaPlugin*> list;
    list.clear();
    for (const auto& r : mRecords) {
        if (r.plugin)
            list.append(r.plugin);
    }
    return list;
}

QString PluginManager::loadPlugin(const QString& path)
{
    QStringList splitedPath = path.split(QDir::separator());
    QString pluginFolderID = splitedPath[splitedPath.size() - 1];

    // not ours
    if (!pluginFolderID.startsWith("com.redpandaide."))
        return "Success";

    PluginRecord rec;
    rec.path = path;
    // Try to read metadata file alongside plugin binary: <basename>.json
    QString metaPath = QDir(path).absoluteFilePath("metadata.json");
    if (QFileInfo::exists(metaPath))
        return QObject::tr("Can't find metadata file for plugin %1").arg(pluginFolderID);
    else {
        QFile mf(metaPath);
        if (mf.open(QFile::ReadOnly | QFile::Text)) {
            QByteArray data = mf.readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (doc.isObject()) {
                rec.metadata = doc.object().toVariantMap();
            }
            mf.close();
        }
    }

    QFileInfo fi(QDir(path).absoluteFilePath(rec.metadata["pluginName"].toString()));
    if (!fi.exists())
        return QObject::tr("The plugin %1 doesn't exists.").arg(fi.fileName());
    // prevent duplicate loads
    for (const auto& r : mRecords) {
        if (r.path == fi.absoluteFilePath())
            return "Success";
    }

    QPluginLoader* loader = new QPluginLoader(fi.absoluteFilePath(), this);
    QObject* instance = loader->instance();
    if (!instance) {
        qDebug() << "Plugin load failed:" << path << loader->errorString();
        return QString("%1").arg(loader->errorString());
    }
    IRedPandaPlugin* plugin = qobject_cast<IRedPandaPlugin*>(instance);
    if (!plugin) {
        // not our plugin
        loader->unload();
        delete loader;
        return QObject::tr("Found Disguised Plugin: %1").arg(pluginFolderID);
    } else {
        if (plugin->pluginID != pluginFolderID) {
            loader->unload();
            delete loader;
            return QObject::tr("Plugin ID mismatch: installed %1, actual %2")
                .arg(pluginFolderID)
                .arg(plugin->pluginID);
        }
    }
    plugin->initialize(pMainWindow);

    rec.loader = loader;
    rec.plugin = plugin;

    // check the depends for the plugin later
    QStringList depends = plugin->depends;
    if (!depends.length()) {
        for (QString& depend : depends) {
            unprocessedDepends.insert(depend);
        }
    }

    // store contributed UI for bookkeeping; actual integration into UI is
    // handled by MainWindow
    rec.toolActions = plugin->toolActions();
    for (const auto& p : plugin->explorerTabs()) {
        rec.explorerTabs.append(p.second);
    }
    for (const auto& p : plugin->messagesTabs()) {
        rec.messagesTabs.append(p.second);
    }
    rec.settingsWidgets = plugin->settingsWidgets();

    mRecords.append(rec);

    emit pluginLoaded(plugin, rec.path);
    return "Success";
}

bool PluginManager::unloadPlugin(const QString& path)
{
    QString abs = QFileInfo(path).absoluteFilePath();
    for (int i = 0; i < mRecords.size(); ++i) {
        auto& r = mRecords[i];
        if (r.path == abs) {
            // ask MainWindow to remove UI; emit signal and let main window
            // decide
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

bool PluginManager::reloadPlugin(const QString& path)
{
    QString abs = QFileInfo(path).absoluteFilePath();
    unloadPlugin(abs);
    return loadPlugin(abs) == "Success";
}

QStringList PluginManager::availablePlugins(const QString& folder) const
{
    QStringList out;
    QDir dir(folder);
    if (!dir.exists())
        return out;
    auto entryList = dir.entryList(QDir::Files);
    for (const QString& file : entryList) {
        out.append(dir.absoluteFilePath(file));
    }
    return out;
}

QVariantMap PluginManager::pluginMetadata(const QString& path) const
{
    QString abs = QFileInfo(path).absoluteFilePath();
    for (const auto& r : mRecords) {
        if (r.path == abs)
            return r.metadata;
    }
    return {};
}

size_t PluginManager::processDepends() noexcept
{
    for (QString depend : unprocessedDepends) {
        if (findPluginByID(depend) != nullptr)
            unprocessedDepends.remove(depend);
    }
    return unprocessedDepends.size();
}

IRedPandaPlugin* PluginManager::findPluginByID(const QString& pluginID) noexcept
{
    for (auto& plugin : mRecords) {
        if (plugin.metadata["ID"] == pluginID)
            return plugin.plugin;
    }
    return nullptr;
}

IRedPandaPlugin* PluginManager::findPluginByName(const QString& pluginName) noexcept
{
    for (auto& plugin : mRecords) {
        if (plugin.metadata["pluginName"] == pluginName)
            return plugin.plugin;
    }
    return nullptr;
}
