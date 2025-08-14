#include "extensionswidget.h"
#include "ui_extensionswidget.h"
#include "../mainwindow.h"
#include "../settings.h"
#include "../iconsmanager.h"
#include "utils.h"
#include "utils/escape.h"
#include "utils/parsearg.h"
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

ExtensionsWidget::ExtensionsWidget(const QString& name, const QString& group,QWidget *parent)
    : SettingsWidget(name,group,parent)
    , ui(new Ui::ExtensionsWidget)
{
    ui->setupUi(this);
    connect(extMetadata, &DownloadTool::sigProgress, this, &ExtensionsWidget::dealMetadataDownloadProcess);
    connect(extMetadata, &DownloadTool::sigDownloadFinished, this, &ExtensionsWidget::onDownloadFinished);

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
    extMetadata->startDownload();
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

    QDir dir(QApplication::applicationDirPath());
    QString filePath = dir.absoluteFilePath("extensionsList.json");

    if (!QFile::exists(filePath)) {
        QMessageBox::critical(
            this,
            tr("File Not Found"),
            tr("Metadata file does not exist at: %1").arg(filePath)
            );
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(
            this,
            tr("Error Loading Metadata File"),
            tr("Failed to open file: %1\nError: %2")
                .arg(filePath)
                .arg(file.errorString())
            );
        return;
    }

    qint64 fileSize = file.size();
    if (fileSize == 0) {
        QMessageBox::critical(
            this,
            tr("Empty File"),
            tr("Metadata file is empty: %1").arg(filePath)
            );
        file.close();
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    // 移除BOM头
    if (data.startsWith("\xEF\xBB\xBF")) {
        data = data.mid(3);
    }

    QJsonParseError parseError;
    QJsonDocument metadata = QJsonDocument::fromJson(data, &parseError);

    if (metadata.isNull()) {
        QString errorMsg = tr("JSON Parse Error: %1\nAt position: %2")
        .arg(parseError.errorString())
            .arg(parseError.offset);

        int startPos = qMax(0, parseError.offset - 20);
        int length = qMin(40, data.length() - startPos);
        QString context = QString::fromUtf8(data.mid(startPos, length));

        errorMsg += tr("\nContext: %1").arg(context);

        QMessageBox::critical(
            this,
            tr("JSON Parse Error"),
            errorMsg
            );
        return;
    }

    if (!metadata.isObject()) {
        QMessageBox::critical(
            this,
            tr("Invalid JSON Format"),
            tr("The root element is not a JSON object")
            );
        return;
    }

    QJsonObject metadata_obj = metadata.object();
    this->metadata = metadata;
    extensionInfoMap.clear();

    QStringList exts = metadata_obj.keys();
    const int totalCount = exts.count();

    ui->extList->clear();
    ui->progressBar->setValue(0);
    ui->extList->setUpdatesEnabled(false);

    for (const QString& extName : exts) {
        QJsonObject extInfo = metadata_obj.value(extName).toObject();
        extensionInfoMap.insert(extName, extInfo);
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
    QString downloadUrl = extInfo.value("file_path").toString();

    if (downloadUrl.isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("Download URL not available for this extension"));
        return;
    }

    // check the special commands of download
    QStringList att = extInfo.value("special_cmd").toString().split(',');
    if(!att.contains(FILEPATH_FULL_LINK)){
        downloadUrl = "https://raw.githubusercontent.com/C14147/RedPandaIDE-Extensions/main/"+downloadUrl;
    }
    if(att.contains(WIN_ONLY)){
        if(QSysInfo::productType().toLower() != "windows"){
            QMessageBox::warning(
                nullptr,
                tr("Unsupported Platform"),
                tr("This Extension is just for Windows only.")
                );
        }
    }
    if(att.contains(WIN64_ONLY)){
        if(!QSysInfo::currentCpuArchitecture().contains("64")){
            QMessageBox::warning(
                nullptr,
                tr("Unsupported Platform"),
                tr("This Extension is just for Windows 64-bit only.")
                );
        }
    }

    // apply the proxy setting
    QString proxy = ui->cbProxy->currentText();
    if (!proxy.isEmpty()) {
        downloadUrl = proxy + downloadUrl;
    }

    // set save path
    QDir savePath = QApplication::applicationDirPath();
    savePath = savePath.absoluteFilePath(".");
    savePath = QDir::cleanPath(savePath.path()) + QDir::separator();

    if (extInfo.value("type").toString() == "theme") {
        savePath = QDir(savePath.path()+"/config/themes/");
    }else if(extInfo.value("type").toString() == "colorScheme"){
        savePath = QDir(savePath.path()+"/config/scheme/");
    }

    // start download
    if (extFile) {
        delete extFile;
    }
    extFile = new DownloadTool(downloadUrl, savePath.path(), this);

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
    QString fileName = QDir::cleanPath(extFile->m_savePath) + QDir::separator() +
                       extFile->fileName;

    ui->statusLabel->setText(tr("Download completed: %1").arg(fileName));

    // 安装扩展
    installExtension(fileName,extFile->getFileType());
}

void ExtensionsWidget::dealExtDownloadProcess([[maybe_unused]] qint64 bytesRead, [[maybe_unused]] qint64 totalBytes, qreal progress)
{
    ui->progressBar->setValue(static_cast<int>(progress * 100));
}

void ExtensionsWidget::on_cancelButton_clicked()
{
    if (extFile) {
        extFile->cancelDownload();
        ui->statusLabel->setText(tr("Download canceled"));
        ui->downloadButton->setEnabled(true);
        ui->cancelButton->setEnabled(false);
    }
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
        pSettings->environment().setTheme(fileInfo.fileName().split('.')[0]);
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

        QFile::remove(filePath);
    }

    pMainWindow->update();
    // update UI
    QMetaObject::invokeMethod(this, [this]() {
        ui->statusLabel->setText(tr("Extension installed successfully!"));
    }, Qt::QueuedConnection);
}
