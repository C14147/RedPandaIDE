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
#include <QTimer>
#include <QListWidgetItem>
#include <QtConcurrent/QtConcurrent>

ExtensionsWidget::ExtensionsWidget(const QString& name, const QString& group, QWidget *parent)
    : SettingsWidget(name, group, parent)
    , ui(new Ui::ExtensionsWidget)
    , m_currentState(State::Idle)
    , m_destroying(false)
{
    ui->setupUi(this);

    // 连接UI信号
    connect(ui->searchButton, &QPushButton::clicked, this, &ExtensionsWidget::onSearchClicked);
    connect(ui->downloadButton, &QPushButton::clicked, this, &ExtensionsWidget::onDownloadClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &ExtensionsWidget::onCancelClicked);
    connect(ui->extList, &QListWidget::itemClicked, this, &ExtensionsWidget::onExtensionSelected);

    // 初始化UI状态
    ui->progressBar->setRange(0, 100);
    ui->progressBar->setValue(0);
    ui->statusLabel->setText(tr("Ready"));
}

ExtensionsWidget::~ExtensionsWidget()
{
    m_destroying = true;
    setState(State::Cancelling);
    cancelAllOperations();

    // 确保所有异步操作完成
    QEventLoop loop;
    QTimer::singleShot(100, &loop, &QEventLoop::quit);
    loop.exec();

    delete ui;
}

void ExtensionsWidget::doSave()
{
    // 不需要保存任何设置
}

void ExtensionsWidget::doLoad()
{
    if (m_destroying) return;
    onSearchClicked();
}

void ExtensionsWidget::onSearchClicked()
{
    if (m_destroying) return;
    setState(State::DownloadingMetadata);

    // 创建新的下载器实例
    m_extMetadata = std::make_unique<DownloadTool>(
        "https://raw.githubusercontent.com/C14147/RedPandaIDE-Extensions/refs/heads/main/extensionsList.json",
        QApplication::applicationDirPath(),
        this
        );

    // 连接信号
    connect(m_extMetadata.get(), &DownloadTool::sigProgress,
            this, &ExtensionsWidget::dealMetadataDownloadProcess);
    connect(m_extMetadata.get(), &DownloadTool::sigDownloadFinished,
            this, &ExtensionsWidget::onDownloadFinished);

    // 开始下载
    m_extMetadata->startDownload();
    safeUpdateUI([this] {
        ui->statusLabel->setText(tr("Downloading metadata..."));
    });
}

void ExtensionsWidget::onDownloadClicked()
{
    if (m_destroying || m_currentState != State::Idle) return;

    QListWidgetItem* selectedItem = ui->extList->currentItem();
    if (!selectedItem) {
        safeUpdateUI([this] {
            QMessageBox::information(this, tr("No Selection"), tr("Please select an extension first."));
        });
        return;
    }

    setState(State::DownloadingExtension);
    QString extensionName = selectedItem->text();

    // 创建新的下载器实例
    m_extFile = std::make_unique<DownloadTool>(
        QString("https://github.com/C14147/RedPandaIDE-Extensions/raw/main/%1.zip").arg(extensionName),
        QApplication::applicationDirPath() + "/extensions",
        this
        );

    // 连接信号
    connect(m_extFile.get(), &DownloadTool::sigProgress, this, [this](qint64 bytesRead, qint64 totalBytes, qreal progress) {
        if (m_destroying) return;
        safeUpdateUI([this, progress] {
            ui->progressBar->setValue(static_cast<int>(progress * 100));
            ui->statusLabel->setText(tr("Downloading: %1%").arg(static_cast<int>(progress * 100)));
        });
    });

    connect(m_extFile.get(), &DownloadTool::sigDownloadFinished, this, [this, extensionName] {
        if (m_destroying) return;
        setState(State::Idle);
        safeUpdateUI([this, extensionName] {
            ui->statusLabel->setText(tr("Download completed: %1").arg(extensionName));
            QMessageBox::information(this, tr("Success"), tr("Extension downloaded successfully!"));
        });
        m_extFile.reset(); // 安全释放资源
    });

    // 开始下载
    m_extFile->startDownload();
    safeUpdateUI([this, extensionName] {
        ui->statusLabel->setText(tr("Starting download: %1").arg(extensionName));
    });
}

void ExtensionsWidget::onCancelClicked()
{
    cancelAllOperations();
    setState(State::Idle);
    safeUpdateUI([this] {
        ui->statusLabel->setText(tr("Operation cancelled"));
        ui->progressBar->setValue(0);
    });
}

void ExtensionsWidget::onExtensionSelected(QListWidgetItem* item)
{
    if (m_destroying || !item) return;

    safeUpdateUI([this, item] {
        // 更新扩展信息显示
        ui->extName->setText(item->text());
        ui->extType->setText(tr("Extension"));
        ui->extAuthor->setText(tr("Unknown"));
        ui->introductionEdit->setPlainText(tr("No description available."));
    });
}

void ExtensionsWidget::onDownloadFinished()
{
    if (m_destroying || m_currentState != State::DownloadingMetadata) return;

    setState(State::ProcessingMetadata);
    safeUpdateUI([this] {
        ui->statusLabel->setText(tr("Processing metadata..."));
    });

    // 在后台线程处理元数据
    QtConcurrent::run([this] {
        processMetadata();
    });
}

void ExtensionsWidget::dealMetadataDownloadProcess(qint64 bytesRead, qint64 totalBytes, qreal progress)
{
    if (m_destroying || m_currentState != State::DownloadingMetadata) return;

    safeUpdateUI([this, progress] {
        ui->progressBar->setValue(static_cast<int>(progress * 100));
        ui->statusLabel->setText(tr("Downloading: %1%").arg(static_cast<int>(progress * 100)));
    });
}

void ExtensionsWidget::setState(State newState)
{
    m_currentState = newState;

    safeUpdateUI([this] {
        // 根据状态更新UI可用性
        const bool isBusy = (m_currentState != State::Idle);
        ui->searchButton->setEnabled(!isBusy);
        ui->downloadButton->setEnabled(!isBusy && ui->extList->currentItem());
        ui->cancelButton->setEnabled(isBusy);
        ui->extList->setEnabled(!isBusy);
    });
}

void ExtensionsWidget::safeUpdateUI(const std::function<void()>& updateFunc)
{
    if (m_destroying) return;

    // 确保UI更新在主线程执行
    if (QThread::currentThread() != this->thread()) {
        QMetaObject::invokeMethod(this, [this, updateFunc] {
            if (!m_destroying) updateFunc();
        }, Qt::QueuedConnection);
    } else {
        if (!m_destroying) updateFunc();
    }
}

void ExtensionsWidget::cancelAllOperations()
{
    // 取消并释放元数据下载器
    if (m_extMetadata) {
        disconnect(m_extMetadata.get(), nullptr, this, nullptr);
        m_extMetadata->cancelDownload();
        m_extMetadata.reset();
    }

    // 取消并释放扩展下载器
    if (m_extFile) {
        disconnect(m_extFile.get(), nullptr, this, nullptr);
        m_extFile->cancelDownload();
        m_extFile.reset();
    }
}

void ExtensionsWidget::processMetadata()
{
    if (m_destroying) return;

    QDir dir(QApplication::applicationDirPath());
    QString filePath = dir.absoluteFilePath("extensionsList.json");

    // 创建局部变量存储错误信息
    QString errorMessage;

    {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            errorMessage = file.errorString(); // 保存错误信息到变量
        } else {
            QByteArray data = file.readAll();
            file.close();

            // 删除BOM头
            if (data.startsWith("\xEF\xBB\xBF")) {
                data = data.mid(3);
            }

            QJsonParseError parseError;
            QJsonDocument metadata = QJsonDocument::fromJson(data, &parseError);

            if (metadata.isNull()) {
                errorMessage = tr("JSON Parse Error: %1\nAt position: %2")
                .arg(parseError.errorString())
                    .arg(parseError.offset);
            } else if (!metadata.isObject()) {
                errorMessage = tr("Invalid JSON Format: The root element is not a JSON object");
            } else {
                QJsonObject metadataObj = metadata.object();
                QStringList extensions = metadataObj.keys();

                safeUpdateUI([this, extensions] {
                    ui->extList->clear();
                    ui->extList->addItems(extensions);
                    ui->progressBar->setValue(100);
                    ui->statusLabel->setText(tr("%1 extensions loaded").arg(extensions.size()));
                });

                setState(State::Idle);
                return;
            }
        }
    }

    // 如果有错误，显示错误消息
    if (!errorMessage.isEmpty()) {
        safeUpdateUI([this, filePath, errorMessage] {
            QMessageBox::critical(nullptr,
                                  tr("Error Processing Metadata"),
                                  tr("Failed to process metadata file: %1\n%2")
                                      .arg(filePath)
                                      .arg(errorMessage));
        });
    }

    setState(State::Idle);
}

void ExtensionsWidget::clearResources()
{
    // 清理临时文件
    QFile::remove(QApplication::applicationDirPath() + "/extensionsList.json");
}
