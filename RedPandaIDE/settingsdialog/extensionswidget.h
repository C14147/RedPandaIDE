#ifndef EXTENSIONSWIDGET_H
#define EXTENSIONSWIDGET_H

/* special commands defines */
#define FILEPATH_FULL_LINK "FILEPATH_FULL_LINK"    // file_path is a link out of the depository
#define WIN64_ONLY         "WIN64_ONLY"
#define WIN_ONLY           "WIN_ONLY"

#include <QWidget>
#include <QApplication>
#include <QDebug>
#include <QByteArray>
#include <QJsonDocument>
#include <QListWidgetItem>

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
        "https://raw.githubusercontent.com/C14147/RedPandaIDE-Extensions/refs/heads/main/extensionsList.json",
        QApplication::applicationDirPath()
        );
    DownloadTool *extFile = nullptr;
    QJsonDocument metadata;
    QMap<QString, QJsonObject> extensionInfoMap; // 存储扩展名到详细信息的映射

public:
    explicit ExtensionsWidget(const QString& name, const QString& group,QWidget *parent = nullptr);
    ~ExtensionsWidget();
    void doSave() override;
    void doLoad() override;

signals:
    void installFinished(bool success, const QString& message);

public slots:
    void onDownloadFinished();    // also deal the analysis of file into list
    void dealMetadataDownloadProcess(qint64 bytesRead, qint64 totalBytes, qreal progress);
    void onDownloadExtFinished();
    void dealExtDownloadProcess(qint64 bytesRead, qint64 totalBytes, qreal progress);

private slots:
    void on_extList_itemClicked(QListWidgetItem *item);
    void on_downloadButton_clicked();
    void on_cancelButton_clicked();
    void on_searchButton_clicked();

private:
    void installExtension(const QString& filePath, QString type);
    void updateExtensionInfo(const QString& extensionName);
    Ui::ExtensionsWidget *ui;
};

#endif // EXTENSIONSWIDGET_H
