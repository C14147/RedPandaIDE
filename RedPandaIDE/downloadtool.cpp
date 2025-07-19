#include "downloadtool.h"
#include <QFileInfo>
#include <QDebug>
#include <QEventLoop>

DownloadTool::DownloadTool(const QString& downloadUrl, const QString& savePath, QObject* parent)
    : QObject(parent)
    , m_downloadUrl(downloadUrl)
    , m_savePath(savePath)
    , httpRequestAborted(false)
{
}

DownloadTool::~DownloadTool()
{
    cancelDownload();
}

void DownloadTool::startDownload()
{
    // 检查生命周期守卫
    if (!m_lifeGuard) return;

    const QUrl newUrl = QUrl::fromUserInput(m_downloadUrl);

    if (!newUrl.isValid()) {
#ifdef DOWNLOAD_DEBUG
        qDebug() << QString("Invalid URL: %1: %2").arg(m_downloadUrl, newUrl.errorString());
#endif // DOWNLOAD_DEBUG
        return;
    }

    QString fileName = newUrl.fileName();

    if (fileName.isEmpty()) fileName = defaultFileName;
    if (m_savePath.isEmpty()) { m_savePath = QApplication::applicationDirPath() + "/tmp"; }
    if (!QFileInfo(m_savePath).isDir()) {
        QDir dir;
        dir.mkpath(m_savePath);
    }

    fileName.prepend(m_savePath + '/');
    if (QFile::exists(fileName)) { QFile::remove(fileName); }
    file = openFileForWrite(fileName);
    if (!file) return;

    startRequest(newUrl);
}

void DownloadTool::cancelDownload() {
    httpRequestAborted = true;
    if (reply) {
        reply->disconnect(this);
        reply->abort();
        reply->deleteLater();
        reply = nullptr;
    }

    if (file) {
        file->close();
        file.reset();
    }
}

void DownloadTool::httpFinished()
{
    // 检查生命周期守卫
    if (!m_lifeGuard) return;

    QFileInfo fi;
    if (httpRequestAborted) {
        if (file) {
            file->close();
            file.reset();
        }
        return;
    }
    if (file) {
        fi.setFile(file->fileName());
        file->close();
        file.reset();
    }

    if (httpRequestAborted) {
        return;
    }

    if (reply->error()) {
        QFile::remove(fi.absoluteFilePath());
#ifdef DOWNLOAD_DEBUG
        qDebug() << QString("Download failed: %1.").arg(reply->errorString());
#endif // DOWNLOAD_DEBUG
        return;
    }

    const QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

    if (!redirectionTarget.isNull()) {
        const QUrl redirectedUrl = url.resolved(redirectionTarget.toUrl());
        file = openFileForWrite(fi.absoluteFilePath());
        if (!file) { return; }
        startRequest(redirectedUrl);
        return;
    }

    Q_EMIT sigDownloadFinished();

#ifdef DOWNLOAD_DEBUG
    qDebug() << QString(tr("Downloaded %1 bytes to %2 in %3")
                            .arg(fi.size()).arg(fi.fileName(), QDir::toNativeSeparators(fi.absolutePath())));
    qDebug() << "Finished";
#endif // DOWNLOAD_DEBUG
}

void DownloadTool::httpReadyRead()
{
    // 检查生命周期守卫
    if (!m_lifeGuard) return;

    if (file) file->write(reply->readAll());
}

void DownloadTool::networkReplyProgress(qint64 bytesRead, qint64 totalBytes)
{
    // 检查生命周期守卫
    if (!m_lifeGuard) return;

    qreal progress = qreal(bytesRead) / qreal(totalBytes);
    Q_EMIT sigProgress(bytesRead, totalBytes, progress);

#ifdef DOWNLOAD_DEBUG
    qDebug() << QString::number(progress * 100, 'f', 2) << "%    "
             << bytesRead / (1024 * 1024) << "MB" << "/" << totalBytes / (1024 * 1024) << "MB";
#endif // DOWNLOAD_DEBUG
}

void DownloadTool::startRequest(const QUrl& requestedUrl)
{
    // 检查生命周期守卫
    if (!m_lifeGuard) return;

    url = requestedUrl;
    httpRequestAborted = false;

    reply = qnam.get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, &DownloadTool::httpFinished);
    connect(reply, &QIODevice::readyRead, this, &DownloadTool::httpReadyRead);
    connect(reply, &QNetworkReply::downloadProgress, this, &DownloadTool::networkReplyProgress);

#ifdef DOWNLOAD_DEBUG
    qDebug() << QString(tr("Downloading %1...").arg(url.toString()));
#endif // DOWNLOAD_DEBUG
}

std::unique_ptr<QFile> DownloadTool::openFileForWrite(const QString& fileName)
{
    // 检查生命周期守卫
    if (!m_lifeGuard) return nullptr;

    std::unique_ptr<QFile> file(new QFile(fileName));
    if (!file->open(QIODevice::WriteOnly)) {
#ifdef DOWNLOAD_DEBUG
        qDebug() << QString("Unable to save the file %1: %2.")
                        .arg(QDir::toNativeSeparators(fileName), file->errorString());
#endif // DOWNLOAD_DEBUG
        return nullptr;
    }
    return file;
}
