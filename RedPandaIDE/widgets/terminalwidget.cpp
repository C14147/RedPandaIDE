#include "terminalwidget.h"
#include <QDebug>
#include <QScrollBar>

TerminalWidget::TerminalWidget(QWidget *parent) : QWidget(parent)
{
    // 初始化UI组件
    outputTextEdit = new QTextEdit(this);
    outputTextEdit->setReadOnly(true);
    outputTextEdit->setLineWrapMode(QTextEdit::NoWrap);
    
    inputLineEdit = new QLineEdit(this);
    
    mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(outputTextEdit);
    mainLayout->addWidget(inputLineEdit);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // 设置字体为等宽字体，适合终端显示
    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    outputTextEdit->setFont(font);
    inputLineEdit->setFont(font);
    
    // 确定要使用的shell
    #ifdef Q_OS_WIN
        shell = "cmd.exe";
        shellArgs << "/K";
    #else
        shell = "/bin/bash";
        shellArgs << "-i";
    #endif
    
    // 初始化进程
    process = new QProcess(this);
    process->setProcessChannelMode(QProcess::MergedChannels);
    
    // 连接信号和槽
    connect(process, &QProcess::readyReadStandardOutput, this, &TerminalWidget::onReadyReadStandardOutput);
    connect(process, &QProcess::readyReadStandardError, this, &TerminalWidget::onReadyReadStandardError);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &TerminalWidget::onProcessFinished);
    connect(inputLineEdit, &QLineEdit::returnPressed, this, &TerminalWidget::onLineEditReturnPressed);
    
    // 启动shell进程
    process->start(shell, shellArgs);
    if (!process->waitForStarted()) {
        outputTextEdit->append("无法启动shell进程!\n");
    } else {
        outputTextEdit->append("终端已启动\n");
    }
    
    // 设置焦点到输入框
    inputLineEdit->setFocus();
}

TerminalWidget::~TerminalWidget()
{
    if (process->state() == QProcess::Running) {
        process->terminate();
        if (!process->waitForFinished(3000)) {
            process->kill();
        }
    }
}

void TerminalWidget::keyPressEvent(QKeyEvent *event)
{
    // 将按键事件传递给输入框
    QKeyEvent *newEvent = new QKeyEvent(*event);
    QCoreApplication::postEvent(inputLineEdit, newEvent);
}

void TerminalWidget::onReadyReadStandardOutput()
{
    QByteArray data = process->readAllStandardOutput();
    QString output = QString::fromLocal8Bit(data);
    
    // 更新输出显示
    outputTextEdit->insertPlainText(output);
    
    // 滚动到底部
    QScrollBar *scrollBar = outputTextEdit->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void TerminalWidget::onReadyReadStandardError()
{
    QByteArray data = process->readAllStandardError();
    QString error = QString::fromLocal8Bit(data);
    
    // 错误输出显示为红色
    outputTextEdit->setTextColor(Qt::red);
    outputTextEdit->insertPlainText(error);
    outputTextEdit->setTextColor(Qt::black);
    
    // 滚动到底部
    QScrollBar *scrollBar = outputTextEdit->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void TerminalWidget::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    outputTextEdit->append(QString("进程已结束，退出码: %1\n").arg(exitCode));
}

void TerminalWidget::onLineEditReturnPressed()
{
    QString command = inputLineEdit->text();
    inputLineEdit->clear();
    
    // 显示命令
    outputTextEdit->insertPlainText("$ " + command + "\n");
    
    // 发送命令到shell
    QByteArray cmdData = (command + "\n").toLocal8Bit();
    process->write(cmdData);
    
    // 滚动到底部
    QScrollBar *scrollBar = outputTextEdit->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}    