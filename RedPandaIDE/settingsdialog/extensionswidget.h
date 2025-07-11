#ifndef EXTENSIONSWIDGET_H
#define EXTENSIONSWIDGET_H

#include <QWidget>
#include "settingswidget.h"
#include "../widgets/macroinfomodel.h"
#include "../toolsmanager.h"

namespace Ui {
class ExtensionsWidget;
}

class ExtensionsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ExtensionsWidget(const QString& name, const QString& group,QWidget *parent = nullptr);
    ~ExtensionsWidget();

private:
    Ui::ExtensionsWidget *ui;
};

#endif // EXTENSIONSWIDGET_H
