#ifndef ZIPUNPACKER_H
#define ZIPUNPACKER_H

#include <QObject>
#include <QString>
#include <QThread>
#if QT_VERSION_MAJOR == 5
#  include <QtGui/private/qzipreader_p.h>
#else
#  include<QtCore/private/qzipreader_p.h>
#endif

class ZipUnpacker : public QObject
{
    Q_OBJECT
public:
    explicit ZipUnpacker(QObject *parent = nullptr);
    ~ZipUnpacker();

public slots:
    void unpack(const QString &zipFilePath, const QString &destinationPath);

signals:
    void progressChanged(int value);
    void finished(bool success, const QString &fileName, const QString &errorMessage = QString());
    void currentFileChanged(const QString &fileName);

private:
    QThread *m_workerThread;
};

#endif // ZIPUNPACKER_H
