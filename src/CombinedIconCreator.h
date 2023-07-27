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

 /**
 * @class CombinedIconCreator
 * @brief The CombinedIconCreator class is responsible for creating combined icons by overlaying a document icon and an application icon.
 *
 * This class provides two main functionalities:
 * 1. Finding the dominant color in the application icon using a given QPixmap.
 * 2. Creating a new QIcon by combining the document icon and application icon with a color overlay.
 *
 * The dominant color of the application icon is calculated by analyzing the pixel colors of the given QPixmap.
 * The findDominantColor() function scans through the image, identifies vibrant colors, and determines the color that
 * appears most frequently as the dominant color.
 * If the dominant color is either very dark (lightness < 10) or very light (lightness > 220),
 * a neutral gray color is returned instead.
 *
 * The createCombinedIcon() function takes two QIcons, representing the document and application icons,
 * and creates a new QIcon by combining them, using the dominant color to make it more distinct.
 *
 * Example usage:
 *
 * CombinedIconCreator iconCreator;
 * QIcon combinedIcon = iconCreator.createCombinedIcon(documentIcon, applicationIcon);
 *
 */

#ifndef COMBINEDICONCREATOR_H
#define COMBINEDICONCREATOR_H

#include <QIcon>
#include <QPixmap>

class CombinedIconCreator
{
public:
    QIcon createCombinedIcon(const QIcon& applicationIcon) const;

private:
    QColor findDominantColor(const QPixmap& pixmap) const;
};

#endif // COMBINEDICONCREATOR_H
