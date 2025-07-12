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

ExtensionsWidget::ExtensionsWidget(const QString& name, const QString& group,QWidget *parent)
    : SettingsWidget(name,group,parent)
    , ui(new Ui::ExtensionsWidget)
{
    ui->setupUi(this);
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
}

void ExtensionsWidget::doSave()
{
    qDebug()<<"ExtensionsWidget won't to save any more.";
}

void ExtensionsWidget::onDownloadFinished()
{
    qDebug()<<"ExtensionsWidget won't to save any more.";
}
