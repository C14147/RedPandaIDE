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

ExtensionsWidget::ExtensionsWidget(const QString& name, const QString& group,QWidget *parent)
    : SettingsWidget(name,group,parent)
    , ui(new Ui::ExtensionsWidget)
{
    ui->setupUi(this);
    connect(extMetadata, &DownloadTool::sigProgress, this, &ExtensionsWidget::dealMetadataDownloadProcess);
    connect(extMetadata, &DownloadTool::sigDownloadFinished, this, &ExtensionsWidget::onDownloadFinished);
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
    qDebug()<<"ExtensionsWidget won't to save any more.";
}

// extensionswidget.cpp
void ExtensionsWidget::onDownloadFinished()
{
    ui->statusLabel->setText(tr("Listing Extensions..."));
    QCoreApplication::processEvents();

    QDir dir(QApplication::applicationDirPath());
    QString filePath = dir.absoluteFilePath("extensionsList.json");

    // 添加文件存在性检查
    if (!QFile::exists(filePath)) {
        QMessageBox::critical(
            nullptr,
            tr("File Not Found"),
            tr("Metadata file does not exist at: %1").arg(filePath)
            );
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(
            nullptr,
            tr("Error Loading Metadata File"),
            tr("Failed to open file: %1\nError: %2")
                .arg(filePath)
                .arg(file.errorString())
            );
        return;
    }

    // 添加文件大小检查
    qint64 fileSize = file.size();
    if (fileSize == 0) {
        QMessageBox::critical(
            nullptr,
            tr("Empty File"),
            tr("Metadata file is empty: %1").arg(filePath)
            );
        file.close();
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    // 调试输出文件内容
    qDebug() << "File content (first 200 bytes):" << data.left(200);
    qDebug() << "File size:" << fileSize << "bytes";

    // 检查并移除可能的BOM头
    if (data.startsWith("\xEF\xBB\xBF")) {
        data = data.mid(3);
        qDebug() << "Removed UTF-8 BOM";
    }

    QJsonParseError parseError;
    QJsonDocument metadata = QJsonDocument::fromJson(data, &parseError);

    // 详细的错误处理
    if (metadata.isNull()) {
        QString errorMsg = tr("JSON Parse Error: %1\nAt position: %2")
        .arg(parseError.errorString())
            .arg(parseError.offset);

        // 在错误位置附近显示内容
        int startPos = qMax(0, parseError.offset - 20);
        int length = qMin(40, data.length() - startPos);
        QString context = QString::fromUtf8(data.mid(startPos, length));

        errorMsg += tr("\nContext: %1").arg(context);

        QMessageBox::critical(
            nullptr,
            tr("JSON Parse Error"),
            errorMsg
            );
        return;
    }

    if (!metadata.isObject()) {
        QMessageBox::critical(
            nullptr,
            tr("Invalid JSON Format"),
            tr("The root element is not a JSON object")
            );
        return;
    }

    QJsonObject metadata_obj = metadata.object();
    QStringList exts = metadata_obj.keys();
    const int totalCount = exts.count();

    qDebug() << "Found" << totalCount << "extensions in JSON";

    if (totalCount == 0) {
        QMessageBox::information(
            nullptr,
            tr("No Extensions"),
            tr("No extensions found in metadata file. The file may have an unexpected structure.")
            );
        return;
    }

    // 优化列表填充
    ui->extList->clear();
    ui->progressBar->setValue(0);
    ui->extList->setUpdatesEnabled(false);

    QList<QListWidgetItem*> items;
    items.reserve(totalCount);

    for (const QString& item : exts) {
        items.append(new QListWidgetItem(item));
    }

    ui->extList->addItems(exts); // 使用批量添加

    ui->extList->setUpdatesEnabled(true);
    ui->progressBar->setValue(100);
    ui->statusLabel->setText(tr("%1 extensions loaded").arg(totalCount));

    // 调试输出前10个键
    qDebug() << "First 10 keys:" << exts.mid(0, 10);
}

void ExtensionsWidget::dealMetadataDownloadProcess(qint64 bytesRead, qint64 totalBytes, qreal progress)
{
    ui->progressBar->setValue(int(progress));
}
