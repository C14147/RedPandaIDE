#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class TerminalWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    TerminalWidget *terminalWidget;
};

#endif // MAINWINDOW_H    