#ifndef EXTENSIONSWIDGET_H
#define EXTENSIONSWIDGET_H

#include <QWidget>
#include <QApplication>
#include <QDebug>

#include "settingswidget.h"
#include "../widgets/macroinfomodel.h"
#include "../toolsmanager.h"
#include "../downloadtool.h"

namespace Ui {
class ExtensionsWidget;
}

class ExtensionsWidget : public SettingsWidget
{
    Q_OBJECT

public:
    DownloadTool *extMetadata = new DownloadTool(
        "https://raw.githubusercontent.com/C14147/RedPandaIDE-Extensions/refs/heads/main/extensionsList.csv",
        QApplication::applicationDirPath()
        );
    DownloadTool *extFile = nullptr;

public:
    explicit ExtensionsWidget(const QString& name, const QString& group,QWidget *parent = nullptr);
    ~ExtensionsWidget();
    void doSave() override;
    void doLoad() override;

public slots:
    void onDownloadFinished();

private:
    Ui::ExtensionsWidget *ui;
};

#endif // EXTENSIONSWIDGET_H
