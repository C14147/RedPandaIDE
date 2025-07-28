#include "UnzipUtil.h"

// 解压整个ZIP
UnzipUtil unzipper;
QObject::connect(&unzipper, &UnzipUtil::progressChanged, 
				 [](int current, int total) {
					 qDebug() << "Progress:" << current << "/" << total;
				 });
QObject::connect(&unzipper, &UnzipUtil::finished,
				 [](bool success, const QString& msg) {
					 if (success) qDebug() << "Extraction complete!";
					 else qCritical() << "Error:" << msg;
				 });

unzipper.extractAll(":/archive.zip", QDir::tempPath());

// 解压单个文件
unzipper.extractFile(":/archive.zip", "config/settings.ini", "/app/config/settings.ini");
