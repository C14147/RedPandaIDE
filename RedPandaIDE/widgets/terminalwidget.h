#ifndef TerminalWidget_H
#define TerminalWidget_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QProcess>
#include <QVBoxLayout>
#include <QKeyEvent>

class TerminalWidget : public QWidget {
    Q_OBJECT
public:
    explicit TerminalWidget(QWidget *parent = nullptr);
    ~TerminalWidget();

private slots:
    void onReadyRead();
    void onCommandEntered();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    QPlainTextEdit *output;
    QLineEdit *input;
    QProcess *process;
    QString shell;
    void startShell();
};

#endif // TerminalWidget_H
