#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <QtPlugin>
#include <QList>
#include <QPair>

class MainWindow;
class SettingsWidget;
class QAction;
class QWidget;

class IRedPandaPlugin
{
public:
    virtual ~IRedPandaPlugin() {}

    // Called once when the plugin is loaded. MainWindow pointer is provided for
    // plugins that need to interact immediately with the application.
    // Plugins may call `mainWindow->uiLanguage()` (added in the host) to query
    // the current UI language code (for example "en_US" or "zh_CN") so they
    // can load/refresh their own translations.
    virtual void initialize(MainWindow *mainWindow) = 0;

    // Return settings pages (caller takes ownership when adding to SettingsDialog)
    virtual QList<SettingsWidget*> settingsWidgets() = 0;

    // Actions to insert into the Tools menu
    virtual QList<QAction*> toolActions() = 0;

    // Tabs to add into the Explorer tab bar. Each pair is (title, widget)
    virtual QList<QPair<QString, QWidget*>> explorerTabs() = 0;

    // Tabs to add into the Messages tab bar. Each pair is (title, widget)
    virtual QList<QPair<QString, QWidget*>> messagesTabs() = 0;
};

Q_DECLARE_INTERFACE(IRedPandaPlugin, "com.redpandaide.PluginInterface/1.0")

#endif // PLUGININTERFACE_H
