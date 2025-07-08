#ifndef TERMINALWIDGET_H
#define TERMINALWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QProcess>
#include <QKeyEvent>

class TerminalWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TerminalWidget(QWidget *parent = nullptr);
    ~TerminalWidget();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onReadyReadStandardOutput();
    void onReadyReadStandardError();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onLineEditReturnPressed();

private:
    QTextEdit *outputTextEdit;
    QLineEdit *inputLineEdit;
    QVBoxLayout *mainLayout;
    QProcess *process;
    QString currentCommand;
    QString shell;
    QStringList shellArgs;
};

#endif // TERMINALWIDGET_H    