#include "splashscreen.h"
#include <QApplication>
#include <QScreen>
#include <QGuiApplication>

SplashScreen::SplashScreen(QWidget* parent)
    : m_splash(nullptr)
{
    m_splash = new QSplashScreen();
    m_splash->setParent(parent);

    QScreen* _s = QGuiApplication::primaryScreen();
    int h = _s->geometry().height() / 2;
    int w = h / 3 * 4;
    QPixmap pixmap = QPixmap(":/icons/images/SplashScreen.png").scaled(w,h);
    if (pixmap.isNull()) {
        pixmap = QPixmap(1, 1);
        pixmap.fill(Qt::transparent);
    }
    m_splash->setPixmap(pixmap);
}

void SplashScreen::show()
{
    if (m_splash) {
        // Set window flag to stay on top
        m_splash->show();
        // Optionally raise to ensure on top
        m_splash->raise();
    }
}

void SplashScreen::quit()
{
    if (m_splash) {
        m_splash->close();
        delete m_splash;
    }
}