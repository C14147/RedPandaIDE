#ifndef EXTENSIONSWIDGET_H
#define EXTENSIONSWIDGET_H

#include <QWidget>
#include <QApplication>
#include <QDebug>
#include <QByteArray>
#include <QJsonDocument>
#include <atomic>
#include <memory>
#include <QSharedPointer>
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
    explicit ExtensionsWidget(const QString& name, const QString& group, QWidget *parent = nullptr);
    ~ExtensionsWidget() override;
    void doSave() override;
    void doLoad() override;

public slots:
    void onDownloadFinished();
    void dealMetadataDownloadProcess(qint64 bytesRead, qint64 totalBytes, qreal progress);

private slots:
    void onSearchClicked();
    void onDownloadClicked();
    void onCancelClicked();
    void onExtensionSelected(QListWidgetItem* item);

private:
    enum class State {
        Idle,
        DownloadingMetadata,
        ProcessingMetadata,
        DownloadingExtension,
        Cancelling
    };

    void setState(State newState);
    void safeUpdateUI(const std::function<void()>& updateFunc);
    void cancelAllOperations();
    void processMetadata();
    void clearResources();

    Ui::ExtensionsWidget *ui;
    State m_currentState = State::Idle;
    bool m_destroying = false;
    std::unique_ptr<DownloadTool> m_extMetadata;
    std::unique_ptr<DownloadTool> m_extFile;
    QJsonDocument m_metadata;

    // 生命周期守卫
    QSharedPointer<QObject> m_lifeGuard;
};

#endif // EXTENSIONSWIDGET_H
