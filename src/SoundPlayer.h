#ifndef SOUNDPLAYER_H
#define SOUNDPLAYER_H

#include <QObject>
#include <QMediaPlayer>

class SoundPlayer : public QObject
{
Q_OBJECT

public:
    explicit SoundPlayer(QObject *parent = nullptr);

    // Note: This can be called without needing an instance of SoundPlayer
    static void playSound(const QString &filename);


};

#endif // SOUNDPLAYER_H
