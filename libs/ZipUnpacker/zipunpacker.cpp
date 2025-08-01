#include "zipunpacker.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>

class UnpackWorker : public QObject
{
    Q_OBJECT
public:
    explicit UnpackWorker(QObject *parent = nullptr) : QObject(parent) {}

public slots:
    void doUnpack(const QString &zipFilePath, const QString &destinationPath)
    {
        QZipReader reader(zipFilePath);
        if (!reader.isReadable()) {
            emit finished(false, tr("无法读取ZIP文件"));
            return;
        }

        const QVector<QZipReader::FileInfo> entries = reader.fileInfoList();
        const int totalFiles = entries.size();
        int processedFiles = 0;

        for (const auto &fileInfo : entries) {
            emit currentFileChanged(fileInfo.filePath);
            emit progressChanged(100 * processedFiles / totalFiles);

            QString filePath = fileInfo.filePath;
            if (filePath.endsWith('/') || filePath.endsWith('\\')) {
                // 这是一个目录
                QDir().mkpath(destinationPath + QDir::separator() + filePath);
            } else {
                // 这是一个文件
                QFileInfo destFileInfo(destinationPath + QDir::separator() + filePath);
                QDir().mkpath(destFileInfo.path());

                QFile file(destFileInfo.absoluteFilePath());
                if (file.open(QIODevice::WriteOnly)) {
                    file.write(reader.fileData(filePath));
                    file.close();

                    // // 设置文件时间
                    // if (fileInfo.lastModified.isValid()) {
                    //     QFileInfo fi(file);
                    //     QFile::setFileTime(file.fileName(), fileInfo.lastModified, QFile::FileModificationTime);
                    // }
                } else {
                    emit finished(false, tr("无法创建文件: %1").arg(destFileInfo.absoluteFilePath()));
                    return;
                }
            }

            processedFiles++;
        }

        emit progressChanged(100);
        emit finished(true, zipFilePath);
    }

signals:
    void progressChanged(int value);
    void finished(bool success, const QString &fileName, const QString &errorMessage = QString());
    void currentFileChanged(const QString &fileName);
};

ZipUnpacker::ZipUnpacker(QObject *parent) : QObject(parent)
{
    m_workerThread = new QThread(this);
    UnpackWorker *worker = new UnpackWorker();
    worker->moveToThread(m_workerThread);

    connect(m_workerThread, &QThread::started, worker, [worker, this]() {
        // 这里会在子线程启动后执行
    });

    connect(this, &ZipUnpacker::unpack, worker, &UnpackWorker::doUnpack);
    connect(worker, &UnpackWorker::progressChanged, this, &ZipUnpacker::progressChanged);
    connect(worker, &UnpackWorker::finished, this, &ZipUnpacker::finished);
    connect(worker, &UnpackWorker::currentFileChanged, this, &ZipUnpacker::currentFileChanged);
    connect(worker, &UnpackWorker::finished, m_workerThread, &QThread::quit);
    connect(worker, &UnpackWorker::finished, worker, &UnpackWorker::deleteLater);

    m_workerThread->start();
}

ZipUnpacker::~ZipUnpacker()
{
    m_workerThread->quit();
    m_workerThread->wait();
}
