#include "SoundPlayer.h"
#include <QSound>
#include <QCoreApplication>
#include <QFile>
#include <QDebug>
#include <QMessageBox>

SoundPlayer::SoundPlayer(QObject *parent) : QObject(parent)
{
}

void SoundPlayer::playSound(const QString &filename)
{
    // Play wav file because it has lower latency
    QStringList soundPaths;

    // Look for sound in the following well-known locations
    soundPaths << QCoreApplication::applicationDirPath() + QString("/Resources/Sounds/") + filename
               << QCoreApplication::applicationDirPath() + QString("/../../Resources/Sounds/") + filename
               << QCoreApplication::applicationDirPath() + QString("/../share/filer/") + filename;

    for (const QString &soundPath : soundPaths) {
        if (QFile::exists(soundPath)) {
            QSound::play(soundPath);
            return;
        }
    }

    qDebug() << "Sound not found";
    QMessageBox::critical(nullptr, tr("Error"), tr("Sound not found at: ") + soundPaths.join(", "));
}
