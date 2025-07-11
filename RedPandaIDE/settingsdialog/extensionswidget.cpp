#include "extensionswidget.h"
#include "ui_extensionswidget.h"
#include "../mainwindow.h"
#include "../settings.h"
#include "../iconsmanager.h"
#include "utils.h"
#include "utils/escape.h"
#include "utils/parsearg.h"
#include "../systemconsts.h"

ExtensionsWidget::ExtensionsWidget(const QString& name, const QString& group,QWidget *parent)
    : SettingsWidget(name,group,parent)
    , ui(new Ui::ExtensionsWidget)
{
    ui->setupUi(this);
}

ExtensionsWidget::~ExtensionsWidget()
{
    delete ui;
}
