#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <QtPlugin>
#include <QList>
#include <QPair>

class MainWindow;
class SettingsWidget;
class QAction;
class QWidget;

/**
 * @brief Interface for RedPandaIDE plugins
 * 
 * This interface defines the contract between the main application and plugins.
 * All plugins must implement this interface to be recognized by RedPandaIDE.
 */
class IRedPandaPlugin
{
public:
    /**
     * @brief List of plugin dependencies (plugin IDs)
     * 
     * Plugins listed here must be loaded before this plugin.
     * The PluginManager will handle dependency resolution.
     */
    QList<QString> depends;
    
    /**
     * @brief Unique plugin identifier
     * 
     * Must be in reverse domain notation (e.g., "com.redpandaide.example").
     * Must match the plugin folder name and the ID in metadata.json.
     */
    QString pluginID;
    
    /**
     * @brief Display name of the plugin
     * 
     * This name will be shown in the UI (e.g., plugin manager, about dialog).
     */
    QString pluginName;
    
    /**
     * @brief Version string of the plugin
     * 
     * Should follow semantic versioning (e.g., "1.0.0", "2.1.3").
     */
    QString pluginVersion;

public:
    virtual ~IRedPandaPlugin() {}

    /**
     * @brief Initialize the plugin
     * 
     * Called once when the plugin is loaded. Plugins can use this to set up 
     * their functionality and interact with the main window.
     * 
     * @param mainWindow Pointer to the main application window
     */
    virtual void initialize(MainWindow* mainWindow) = 0;

    /**
     * @brief Get settings widgets provided by the plugin
     * 
     * Returns a list of settings widgets that will be added to the 
     * SettingsDialog. The caller takes ownership of the returned widgets.
     * 
     * @return QList<SettingsWidget*> List of settings widgets
     */
    virtual QList<SettingsWidget*> settingsWidgets() = 0;

    /**
     * @brief Get tool actions provided by the plugin
     * 
     * Returns actions that will be added to the Tools menu.
     * 
     * @return QList<QAction*> List of QAction objects
     */
    virtual QList<QAction*> toolActions() = 0;

    /**
     * @brief Get explorer tabs provided by the plugin
     * 
     * Returns tabs that will be added to the Explorer pane. Each pair 
     * contains (title, widget).
     * 
     * @return QList<QPair<QString, QWidget*>> List of (title, widget) pairs
     */
    virtual QList<QPair<QString, QWidget*>> explorerTabs() = 0;

    /**
     * @brief Get messages tabs provided by the plugin
     * 
     * Returns tabs that will be added to the Messages pane. Each pair 
     * contains (title, widget).
     * 
     * @return QList<QPair<QString, QWidget*>> List of (title, widget) pairs
     */
    virtual QList<QPair<QString, QWidget*>> messagesTabs() = 0;
};

Q_DECLARE_INTERFACE(IRedPandaPlugin, "com.redpandaide.PluginInterface.1.0.0")

#endif // PLUGININTERFACE_H