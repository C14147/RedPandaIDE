#include "terminalwidget.h"

#ifdef Q_OS_WIN
const QString DEFAULT_SHELL = "cmd.exe";
#else
const QString DEFAULT_SHELL = "/bin/bash";
#endif

TerminalWidget::TerminalWidget(QWidget *parent) : QWidget(parent) {
    output = new QPlainTextEdit(this);
    output->setReadOnly(true);
    input = new QLineEdit(this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(output);
    layout->addWidget(input);
    setLayout(layout);

    shell = DEFAULT_SHELL;
    process = new QProcess(this);

    connect(process, &QProcess::readyReadStandardOutput, this, &TerminalWidget::onReadyRead);
    connect(process, &QProcess::readyReadStandardError, this, &TerminalWidget::onReadyRead);
    connect(input, &QLineEdit::returnPressed, this, &TerminalWidget::onCommandEntered);

    startShell();
}

TerminalWidget::~TerminalWidget() {
    process->close();
}

void TerminalWidget::startShell() {
    process->start(shell);
    if (!process->waitForStarted(2000)) {
        output->appendPlainText("Launch Terminal Process Failed.");
    }
}

void TerminalWidget::onReadyRead() {
    QByteArray data = process->readAllStandardOutput() + process->readAllStandardError();
    output->moveCursor(QTextCursor::End);
    output->insertPlainText(QString::fromLocal8Bit(data));
    output->moveCursor(QTextCursor::End);
}

void TerminalWidget::onCommandEntered() {
    QString cmd = input->text() + "\n";
    process->write(cmd.toLocal8Bit());
    input->clear();
}

void TerminalWidget::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Tab) {
        process->write("\t");
    } else {
        QWidget::keyPressEvent(event);
    }
}
