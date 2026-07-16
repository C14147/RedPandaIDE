#include "splashscreen.h"
#include <QApplication>

SplashScreen::SplashScreen(QWidget* parent)
    : m_splash(nullptr)
{
    QPixmap pixmap(":/images/SplashScreen.png");
    if (pixmap.isNull()) {
        // Fallback to a default empty pixmap or handle error
        // For simplicity, create a 1x1 transparent pixmap
        pixmap = QPixmap(1, 1);
        pixmap.fill(Qt::transparent);
    }
    m_splash = new QSplashScreen(parent, pixmap);
}

void SplashScreen::show()
{
    if (m_splash) {
        // Set window flag to stay on top
        m_splash->setWindowFlags(m_splash->windowFlags() | Qt::WindowStaysOnTopHint);
        m_splash->show();
        // Optionally raise to ensure on top
        m_splash->raise();
    }
}

void StartUp::quit()
{
    if (m_splash) {
        m_splash->close();
        delete m_splash;
        m_splash = nullptr;
    }
}