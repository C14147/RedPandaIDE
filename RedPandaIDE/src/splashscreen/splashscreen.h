/* The SplashScreen UI for startup. */

#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <QSplashScreen>
#include <QPixmap>
#include <QString>

class SplashScreen
{
public:
    explicit SplashScreen(QWidget* parent = nullptr);
    ~SplashScreen() = default; // does nothing

    void show();
    void quit();

private:
    QSplashScreen* m_splash;
};

#endif // SPLASHSCREEN_H
