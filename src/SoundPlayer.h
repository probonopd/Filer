/*-
 * Copyright (c) 2022-23 Simon Peter <probono@puredarwin.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef SOUNDPLAYER_H
#define SOUNDPLAYER_H

#include <QObject>
#include <QMediaPlayer>

/**
 * @brief The SoundPlayer class provides functionality to play sound effects.
 */
class SoundPlayer : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a SoundPlayer object.
     * @param parent The parent QObject (optional).
     */
    explicit SoundPlayer(QObject *parent = nullptr);

    /**
     * @brief Plays a sound effect from the specified file.
     * @param filename The path to the sound file to be played.
     *
     * This static method allows you to play a sound without needing an instance of SoundPlayer.
     */
    static void playSound(const QString &filename); // Note: This can be called without needing an instance of SoundPlayer
};

#endif // SOUNDPLAYER_H
