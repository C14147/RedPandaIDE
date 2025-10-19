# Plugin Interface Specification

## IRedPandaPlugin

This interface defines the contract between the main application and plugins. All plugins must implement this interface to be recognized by RedPandaIDE.

### Public Properties

| Property | Type | Description |
|----------|------|-------------|
| `depends` | `QList<QString>` | List of plugin IDs that this plugin depends on. These plugins must be loaded before this plugin. |
| `pluginID` | `QString` | **Required**. Unique identifier for the plugin in reverse domain notation (e.g., `"com.redpandaide.example"`). Must match the folder name. |
| `pluginName` | `QString` | **Required**. Display name of the plugin shown in the UI. |
| `pluginVersion` | `QString` | **Required**. Version string of the plugin (e.g., `"1.0.0"`). |

### Required Methods

#### `void initialize(MainWindow* mainWindow)`

Called once when the plugin is loaded. Plugins can use this to set up their functionality and interact with the main window.

- **Parameters**:
  - `mainWindow`: Pointer to the main application window

#### `QList<SettingsWidget*> settingsWidgets()`

Returns a list of settings widgets provided by the plugin.

- **Returns**: List of settings widgets (ownership transferred to caller)

#### `QList<QAction*> toolActions()`

Returns actions to be added to the Tools menu.

- **Returns**: List of QAction objects

#### `QList<QPair<QString, QWidget*>> explorerTabs()`

Returns tabs to be added to the Explorer pane.

- **Returns**: List of (title, widget) pairs

#### `QList<QPair<QString, QWidget*>> messagesTabs()`

Returns tabs to be added to the Messages pane.

- **Returns**: List of (title, widget) pairs

### Implementation Example

```cpp
class ExamplePlugin : public QObject, public IRedPandaPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.redpandaide.PluginInterface.1.0.0" FILE "exampleplugin.json")
    Q_INTERFACES(IRedPandaPlugin)

public:
    ExamplePlugin() {
        pluginID = "com.redpandaide.example";
        pluginName = "Example Plugin";
        pluginVersion = "1.0.0";
    }

    void initialize(MainWindow* mainWindow) override {
        // Initialization code
    }

    QList<SettingsWidget*> settingsWidgets() override {
        return {};
    }

    QList<QAction*> toolActions() override {
        return {};
    }

    QList<QPair<QString, QWidget*>> explorerTabs() override {
        return {};
    }

    QList<QPair<QString, QWidget*>> messagesTabs() override {
        return {};
    }
};
```

### Plugin Metadata Requirements

Each plugin must include a `metadata.json` file in its root directory with the following structure:

```json
{
  "ID": "com.redpandaide.example",
  "Name": "Example Plugin",
  "Version": "1.0.0",
  "Description": "A sample plugin for demonstration",
  "PluginFile": "exampleplugin.dll"
}
```

### Important Notes

1. Plugin folder names **must** start with `com.redpandaide.` to be recognized
2. The `pluginID` must match the folder name and the `ID` field in metadata.json
3. All pointers returned by interface methods become owned by the caller
4. Plugins must be compiled as shared libraries (DLLs on Windows)