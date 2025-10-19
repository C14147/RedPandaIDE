// This file copied from CSDN:
// https://blog.csdn.net/weixin_42602900/article/details/140606196

#ifndef DOWNLOADTOOL_H
#define DOWNLOADTOOL_H

#include <QObject>
#include <QDebug>
#include <QUrl>
#include <QFile>
#include <QDir>
#include <QPointer>
#include <QApplication>
#include <QNetworkReply>
#include <QNetworkAccessManager>

#include <memory>

#define DOWNLOAD_DEBUG

class DownloadTool : public QObject
{
    Q_OBJECT

public:
    QString m_downloadUrl;  // 保存构造时传入的下载url
    QString m_savePath;     // 保存构造时传入的保存路径
    QString fileName;

public:
    // 构造函数参数:  1)http文件完整的url  2)保存的路径
    explicit DownloadTool(const QString& downloadUrl, const QString& savePath, QObject* parent = nullptr);
    ~DownloadTool();

    void startDownload();  // 开始下载文件
    void startDownload(QString type);
    void cancelDownload(); // 取消下载文件
    QString getFileType(void);

Q_SIGNALS:
    void sigProgress(qint64 bytesRead, qint64 totalBytes, qreal progress);  // 下载进度信号
    void sigDownloadFinished();  // 下载完成信号

private Q_SLOTS:
    void httpFinished();    // QNetworkReply::finished对应的槽函数
    void httpReadyRead();   // QIODevice::readyRead对应的槽函数

    void networkReplyProgress(qint64 bytesRead, qint64 totalBytes);  // QNetworkReply::downloadProgress对应的槽函数

private:
    void startRequest(const QUrl& requestedUrl);
    std::unique_ptr<QFile> openFileForWrite(const QString& fileName);

private:
    QString type;
    const QString defaultFileName = "tmp"; // 默认文件名

    QUrl url;
    QNetworkAccessManager qnam;
    QPointer<QNetworkReply> reply;
    std::unique_ptr<QFile> file;
    bool httpRequestAborted = false; // 初始化为false，避免未定义行为
};

#endif // DOWNLOADTOOL_H