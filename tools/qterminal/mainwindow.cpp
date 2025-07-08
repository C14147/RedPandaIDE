#include "mainwindow.h"
#include "terminalwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    terminalWidget = new TerminalWidget(this);
    setCentralWidget(terminalWidget);
    setWindowTitle("嵌入式终端");
    resize(800, 600);
}

MainWindow::~MainWindow()
{
}    