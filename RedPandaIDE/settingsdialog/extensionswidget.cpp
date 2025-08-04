#include "extensionswidget.h"
#include "ui_extensionswidget.h"
#include "../mainwindow.h"
#include "../settings.h"
#include "../iconsmanager.h"
#include "utils.h"
#include "utils/escape.h"
#include "utils/parsearg.h"
#include "extensionsmanager.h"
#include "../systemconsts.h"
#include "../downloadtool.h"

#include <QDir>
#include <QMessageBox>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QStringList>
#include <QProcess>
#include <QStandardPaths>
#include <QSysInfo>
#include <QDesktopServices>
#include <QtConcurrent/QtConcurrent>

ExtensionsManager::ExtensionsManager(QObject* parent) : QObject(parent) {}

void ExtensionsManager::startMetadataDownload() {
    // This would trigger DownloadTool to start metadata download
    // Signal/slot connection should be handled in ExtensionsWidget
}

bool ExtensionsManager::loadMetadata(QJsonDocument& metadata, QMap<QString, QJsonObject>& extensionInfoMap, QString& errorMsg) {
    QDir dir(QApplication::applicationDirPath());
    QString filePath = dir.absoluteFilePath("extensionsList.json");
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        errorMsg = QObject::tr("Failed to open file: %1").arg(filePath);
        return false;
    }
    QByteArray data = file.readAll();
    file.close();
    if (data.startsWith("\xEF\xBB\xBF")) {
        data = data.mid(3);
    }
    QJsonParseError parseError;
    metadata = QJsonDocument::fromJson(data, &parseError);
    if (metadata.isNull() || !metadata.isObject()) {
        errorMsg = QObject::tr("JSON Parse Error: %1").arg(parseError.errorString());
        return false;
    }
    QJsonObject metadata_obj = metadata.object();
    extensionInfoMap.clear();
    for (const QString& extName : metadata_obj.keys()) {
        QJsonObject extInfo = metadata_obj.value(extName).toObject();
        extensionInfoMap.insert(extName, extInfo);
    }
    return true;
}

bool ExtensionsManager::downloadExtension(const QJsonObject& extInfo, QString proxy, QString& savePath, QString& errorMsg, DownloadTool*& extFile) {
    QString downloadUrl = extInfo.value("file_path").toString();
    if (downloadUrl.isEmpty()) {
        errorMsg = QObject::tr("Download URL not available for this extension");
        return false;
    }
    QStringList att = extInfo.value("special_cmd").toString().split(',');
    if(!att.contains("FILEPATH_FULL_LINK")){
        downloadUrl = "https://raw.githubusercontent.com/C14147/RedPandaIDE-Extensions/main/"+downloadUrl;
    }
    if (!proxy.isEmpty()) {
        downloadUrl = proxy + downloadUrl;
    }
    QDir dir(QApplication::applicationDirPath());
    savePath = dir.absoluteFilePath(".");
    if (extInfo.value("type").toString() == "theme") {
        savePath = QDir(savePath+"/config/themes/").path();
    } else if(extInfo.value("type").toString() == "colorScheme") {
        savePath = QDir(savePath+"/config/scheme/").path();
    }
    if (extFile) {
        delete extFile;
    }
    extFile = new DownloadTool(downloadUrl, savePath, nullptr);
    return true;
}

bool ExtensionsManager::installExtension(const QString& filePath, const QString& type, QString& errorMsg) {
    QFileInfo fileInfo(filePath);
    QString extensionDir = filePath;
    if (type == "colorScheme" || type == "theme") {
        // UI update should be handled in ExtensionsWidget
        return true;
    } else {
        QString command;
        QStringList args;
#ifdef Q_OS_WIN
        command = "cmd";
        args << "/C \""
             << QString(QDir(QApplication::applicationDirPath()+"/7z/").absoluteFilePath("7za.exe"))
             << QString(" x '%1' -o '%2' \"").arg(filePath).arg(extensionDir);
#endif
        QProcess process;
        process.start(command, args);
        if (!process.waitForFinished(30000)) {
            errorMsg = QObject::tr("Installation timed out");
            return false;
        }
        if (process.exitCode() != 0) {
            errorMsg = QObject::tr("Installation failed: %1").arg(QString::fromUtf8(process.readAllStandardError()));
            return false;
        }
        QFile::remove(filePath);
    }
    return true;
}

void ExtensionsManager::cancelDownload(DownloadTool* extFile) {
    if (extFile) {
        extFile->cancelDownload();
    }
}

void ExtensionsManager::removeDownloadedFile(const QString& filePath) {
    QFile::remove(filePath);
}

ExtensionsWidget::ExtensionsWidget(const QString& name, const QString& group,QWidget *parent)
    : SettingsWidget(name,group,parent)
    , ui(new Ui::ExtensionsWidget)
    , manager(new ExtensionsManager(this))
{
    ui->setupUi(this);
    connect(manager, &ExtensionsManager::metadataDownloadProgress, this, &ExtensionsWidget::dealMetadataDownloadProcess);
    connect(manager, &ExtensionsManager::metadataDownloadFinished, this, &ExtensionsWidget::onDownloadFinished);
    connect(ui->extList, &QListWidget::itemClicked, this, &ExtensionsWidget::on_extList_itemClicked);
    connect(ui->downloadButton, &QPushButton::clicked, this, &ExtensionsWidget::on_downloadButton_clicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &ExtensionsWidget::on_cancelButton_clicked);
    connect(ui->searchButton, &QPushButton::clicked, this, &ExtensionsWidget::on_searchButton_clicked);
}

ExtensionsWidget::~ExtensionsWidget()
{
    delete ui;
    delete extMetadata;
    delete extFile;
}

void ExtensionsWidget::doLoad()
{
    manager->startMetadataDownload();
    ui->statusLabel->setText(tr("Downloading Metadata..."));
}

void ExtensionsWidget::doSave()
{
    // Nothing
}

void ExtensionsWidget::onDownloadFinished()
{
    ui->statusLabel->setText(tr("Listing Extensions..."));
    QCoreApplication::processEvents();
    QString errorMsg;
    if (!manager->loadMetadata(metadata, extensionInfoMap, errorMsg)) {
        QMessageBox::critical(this, tr("Error Loading Metadata"), errorMsg);
        return;
    }
    QStringList exts = extensionInfoMap.keys();
    const int totalCount = exts.count();
    ui->extList->clear();
    ui->progressBar->setValue(0);
    ui->extList->setUpdatesEnabled(false);
    for (const QString& extName : exts) {
        ui->extList->addItem(extName);
    }
    ui->extList->setUpdatesEnabled(true);
    ui->progressBar->setValue(100);
    ui->statusLabel->setText(tr("%1 extensions loaded").arg(totalCount));
}

void ExtensionsWidget::dealMetadataDownloadProcess([[maybe_unused]] qint64 bytesRead, [[maybe_unused]] qint64 totalBytes, qreal progress)
{
    ui->progressBar->setValue(int(progress * 100));
}

void ExtensionsWidget::on_extList_itemClicked(QListWidgetItem *item)
{
    if (!item) return;

    QString extensionName = item->text();
    updateExtensionInfo(extensionName);
}

void ExtensionsWidget::updateExtensionInfo(const QString& extensionName)
{
    if (!extensionInfoMap.contains(extensionName)) {
        return;
    }

    QJsonObject extInfo = extensionInfoMap.value(extensionName);

    // 更新UI显示扩展信息
    ui->extName->setText(extInfo.value("name").toString(extensionName));
    ui->extType->setText(extInfo.value("type").toString(tr("Unknown")));
    ui->extAuthor->setText(extInfo.value("author").toString(tr("Unknown")));
    ui->introductionEdit->setPlainText(extInfo.value("introduction").toString(tr("No description available")));
}

void ExtensionsWidget::on_downloadButton_clicked()
{
    QListWidgetItem *item = ui->extList->currentItem();
    if (!item) {
        QMessageBox::warning(this, tr("No Selection"), tr("Please select an extension to download"));
        return;
    }
    QString extensionName = item->text();
    if (!extensionInfoMap.contains(extensionName)) {
        QMessageBox::critical(this, tr("Error"), tr("Extension information not found"));
        return;
    }
    QJsonObject extInfo = extensionInfoMap.value(extensionName);
    QString proxy = ui->cbProxy->currentText();
    QString savePath, errorMsg;
    if (!manager->downloadExtension(extInfo, proxy, savePath, errorMsg, extFile)) {
        QMessageBox::critical(this, tr("Download Error"), errorMsg);
        return;
    }
    connect(extFile, &DownloadTool::sigProgress, this, &ExtensionsWidget::dealExtDownloadProcess);
    connect(extFile, &DownloadTool::sigDownloadFinished, this, &ExtensionsWidget::onDownloadExtFinished);
    ui->statusLabel->setText(tr("Downloading %1...").arg(extensionName));
    ui->downloadButton->setEnabled(false);
    ui->cancelButton->setEnabled(true);
    extFile->startDownload(extInfo.value("type").toString());
}

void ExtensionsWidget::onDownloadExtFinished()
{
    ui->downloadButton->setEnabled(true);
    ui->cancelButton->setEnabled(false);
    QListWidgetItem *item = ui->extList->currentItem();
    if (!item) return;
    QString extensionName = item->text();
    QString fileName = QDir::cleanPath(extFile->m_savePath) + QDir::separator() + extFile->fileName;
    ui->statusLabel->setText(tr("Download completed: %1").arg(fileName));
    QString errorMsg;
    if (!manager->installExtension(fileName, extFile->getFileType(), errorMsg)) {
        QMessageBox::critical(this, tr("Install Error"), errorMsg);
        return;
    }
    // UI update logic for colorScheme/theme can remain here
    if (extFile->getFileType() == "colorScheme") {
        pSettings->editor().setColorScheme(QFileInfo(fileName).fileName().replace("_"," ").split('.')[0]);
        pSettings->editor().save();
        pMainWindow->updateEditorColorSchemes();
    } else if (extFile->getFileType() == "theme") {
        pSettings->environment().setTheme(QFileInfo(fileName).fileName());
        pSettings->environment().save();
        pMainWindow->applySettings();
    }
    pMainWindow->update();
    QMetaObject::invokeMethod(this, [this]() {
        ui->statusLabel->setText(tr("Extension installed successfully!"));
    }, Qt::QueuedConnection);
}

void ExtensionsWidget::dealExtDownloadProcess([[maybe_unused]] qint64 bytesRead, [[maybe_unused]] qint64 totalBytes, qreal progress)
{
    ui->progressBar->setValue(static_cast<int>(progress * 100));
}

void ExtensionsWidget::on_cancelButton_clicked()
{
    manager->cancelDownload(extFile);
    ui->statusLabel->setText(tr("Download canceled"));
    ui->downloadButton->setEnabled(true);
    ui->cancelButton->setEnabled(false);
}

void ExtensionsWidget::on_searchButton_clicked()
{
    QString searchText = ui->extLineEdit->text().trimmed();

    for (int i = 0; i < ui->extList->count(); ++i) {
        QListWidgetItem *item = ui->extList->item(i);
        bool match = item->text().contains(searchText, Qt::CaseInsensitive);
        item->setHidden(!match);
    }
}

void ExtensionsWidget::installExtension(const QString& filePath, QString type)
{
    QFileInfo fileInfo(filePath);
    QString extensionDir = filePath;

    if (type == "colorScheme"){
        pSettings->editor().setColorScheme(fileInfo.fileName().replace("_"," ").split('.')[0]);
        pSettings->editor().save();
        pMainWindow->updateEditorColorSchemes();
    }else if(type == "theme"){
        pSettings->environment().setTheme(fileInfo.fileName());
        pSettings->environment().save();
        pMainWindow->applySettings();
    }else{
        // 解压文件
        QString command;
        QStringList args;

#ifdef Q_OS_WIN
        command = "cmd";
        args << "/C \""
             << QString(QDir(QApplication::applicationDirPath()+"/7z/").absoluteFilePath("7za.exe"))
             << QString(" x '%1' -o '%2' \"").arg(filePath).arg(extensionDir);
#endif
        ui->statusLabel->setText(tr("Unziping Extension..."));
        QProcess process;
        process.start(command, args);
        if (!process.waitForFinished(30000)) {
            emit installFinished(false, tr("Installation timed out"));
            return;
        }

        if (process.exitCode() != 0) {
            QString error = QString::fromUtf8(process.readAllStandardError());
            emit installFinished(false, tr("Installation failed: %1").arg(error));
            return;
        }

        // 删除下载的压缩文件
        QFile::remove(filePath);
    }

    pMainWindow->update();
    // update UI
    QMetaObject::invokeMethod(this, [this]() {
        ui->statusLabel->setText(tr("Extension installed successfully!"));
    }, Qt::QueuedConnection);
}
