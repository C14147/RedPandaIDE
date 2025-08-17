## RedPandaIDE Plugin Developing Guide

This document explains the plugin architecture of RedPandaIDE and shows how to build plugins with Qt Creator (QMake). It presents a compact header-only plugin example and explains a mandatory preparatory step: cloning the header definitions repository used by header-only plugin authors.

## Overview

RedPandaIDE loads in-process Qt plugins from a plugins folder (default: `<config>/plugins`). Plugins must implement the `IRedPandaPlugin` interface (see `pluginmanager/plugininterface.h`). The host `PluginManager` loads plugin shared libraries with `QPluginLoader` and calls `initialize(MainWindow*)` on each plugin instance.

Plugin contributions
- Settings pages: `QList<SettingsWidget*> settingsWidgets()` — the host may take ownership of returned widgets when adding them into the Options dialog.
- Tools menu actions: `QList<QAction*> toolActions()` — returned QActions are inserted into the Tools menu by the host.
- Explorer tabs: `QList<QPair<QString, QWidget*>> explorerTabs()` — title/widget pairs added to the Explorer dock QTabWidget.
- Messages tabs: `QList<QPair<QString, QWidget*>> messagesTabs()` — added to the Messages dock QTabWidget.

Plugins receive a pointer to the main window in `initialize(MainWindow *mainWindow)` and can call `mainWindow->uiLanguage()` to obtain the current UI language (for example `"en_US"` or `"zh_CN"`).

Important ownership notes
- The host may take ownership of `SettingsWidget` instances (see `SettingsDialog::addWidget`). Design your widgets so they can be created and destroyed safely.
- QActions are typically owned by menus; the host will remove them on unload.
- Widgets added to tab bars are removed and deleted by the host on plugin unload.

## Mandatory: clone header definitions for header-only development

Many developers prefer header-only plugins (no `.cpp` files). To make this workflow work consistently in this codebase you must clone the headers repository that provides forward declarations and small inline helpers used by header-only plugins.

Clone the definitions repo into a local path once before building header-only plugins:

```powershell
git clone https://github.com/C14147/RedPandaIDE-defs.git C:\path\to\redpanda-defs
```

Then in your plugin `.pro` add the clone directory to `INCLUDEPATH`:

```qmake
INCLUDEPATH += "C:/path/to/redpanda-defs"
```

Notes:
- The `RedPandaIDE-defs` repository contains header-only helper files and public API headers that header-only plugins include directly. Keep the cloned repo updated when hosts change public headers.
- You can place the clone under your `plugin/` directory, e.g. `plugin/defs/`, and add `INCLUDEPATH += $$PWD/plugin/defs`

## Building a plugin (QMake) with Qt Creator

1. Create a new subdirectory under `plugin/`, for example `plugin/headerhelloplugin/`.
2. Create a `.pro` file for the plugin with `TEMPLATE = lib` and `CONFIG += plugin`.
3. Add `INCLUDEPATH` entries to include host headers and the cloned `RedPandaIDE-defs` headers.
4. Implement a class that implements `IRedPandaPlugin` and expose it as a Qt plugin using `Q_INTERFACES(IRedPandaPlugin)` and `Q_PLUGIN_METADATA(IID "com.redpandaide.PluginInterface/1.0")`.
5. Build the plugin in Qt Creator and place the produced shared library (DLL on Windows) into the application plugins folder (`<config>/plugins`) or the location your `PluginManager` loads from.

## Example: Header-only "Hello" plugin (concept)

This example demonstrates a header-only plugin that adds a Tools menu action `Say Hello`. It uses only header files (no `.cpp`). Before building, clone the `RedPandaIDE-defs` repo (see above) so required forward declarations and small helpers are available.

File: `plugin/headerhelloplugin/headerhelloplugin.h`

```cpp
#ifndef HEADERHELLOPLUGIN_H
#define HEADERHELLOPLUGIN_H

#include <QObject>
#include <QAction>
#include <QMessageBox>
#include "../pluginmanager/plugininterface.h" // or include path via defs clone

class MainWindow; // forward declaration from defs

class HeaderHelloPlugin : public QObject, public IRedPandaPlugin
{
	Q_OBJECT
	Q_INTERFACES(IRedPandaPlugin)
	Q_PLUGIN_METADATA(IID "com.redpandaide.PluginInterface/1.0")

public:
	HeaderHelloPlugin() { }
	~HeaderHelloPlugin() override { }

	void initialize(MainWindow *mainWindow) override {
		mMainWindow = mainWindow;
		mAction = new QAction(tr("Say Hello"), mMainWindow);
		connect(mAction, &QAction::triggered, this, [this](){
			QMessageBox::information(mMainWindow, tr("Hello"), tr("Hello from header-only plugin!"));
		});
	}

	QList<SettingsWidget*> settingsWidgets() override { return {}; }
	QList<QAction*> toolActions() override { return { mAction }; }
	QList<QPair<QString, QWidget*>> explorerTabs() override { return {}; }
	QList<QPair<QString, QWidget*>> messagesTabs() override { return {}; }

private:
	MainWindow *mMainWindow = nullptr;
	QAction *mAction = nullptr;
};

#endif // HEADERHELLOPLUGIN_H
```

Notes about the header-only plugin example
- Make every non-trivial method inline or implement it inside the header so no `.cpp` is required.
- Use forward declarations for host types (for example `class MainWindow;`) where possible. The `RedPandaIDE-defs` clone provides small helper headers to make this robust.
- Remember to mark symbols `inline` if you define free functions or static data to avoid multiple-definition linker errors when the header is included in multiple translation units.

## Metadata for plugins

Each plugin may ship an optional JSON metadata file placed next to the plugin binary with the same base name plus `.json` (for example `headerhelloplugin.json`). The `PluginManager` will parse this file at load time and store its contents into an in-memory metadata map available to other code via `PluginManager::pluginMetadata(path)`.

Recommended metadata fields:
- `name` — human-friendly plugin name
- `version` — semantic version string
- `author` — author name or organization
- `description` — short description
- `actions` — list of action identifiers provided by the plugin
- `settings` — list of settings keys the plugin uses

Example `headerhelloplugin.json`:

```json
{
  "name": "HeaderHelloPlugin",
  "version": "0.1.0",
  "author": "Your Name",
  "description": "A tiny header-only plugin that says hello.",
  "actions": ["say_hello"]
}
```

## Plugin tips and best practices

- Prefer lightweight settings widgets and save frequently; hosts may own settings widgets and destroy them on unload.
- Avoid performing heavy initialization in `initialize()`; use background threads or lazy initialization triggered by user actions.
- If you need to support dynamic language switching, coordinate with the host to get a `languageChanged(const QString&)` signal, or re-query `mainWindow->uiLanguage()` and retranslate your UI.

## Build and install quick steps

1. Clone `RedPandaIDE-defs`:

```powershell
git clone https://github.com/C14147/RedPandaIDE-defs.git plugin/defs
```

2. Create your plugin folder and add a `.pro` file. Example minimal `.pro`:

```qmake
TEMPLATE = lib
CONFIG += plugin
QT += widgets

INCLUDEPATH += $$PWD/defs

SOURCES += \
	# none if header-only

HEADERS += \
	headerhelloplugin.h

DISTFILES += \
	headerhelloplugin.json

```

3. Open the `.pro` in Qt Creator, build, then copy the produced DLL into your RedPandaIDE plugins folder.
