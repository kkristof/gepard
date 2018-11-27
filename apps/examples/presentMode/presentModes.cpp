/* Copyright (C) 2018, Gepard Graphics
 * Copyright (C) 2018, Kristof Kosztyo <kkristof@inf.u-szeged.hu>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "gepard.h"
#include "gepard-image.h"
#include "surfaces/gepard-xsurface.h"
#include <chrono>
#include <iostream>
#include <thread>

#define SURFACE_SIZE 600

void generateCheckerBoard(gepard::Gepard& gepard)
{
    const int cellNumb = 15;
    const int cellWidth = SURFACE_SIZE / cellNumb;
    for (int i = 0; i < cellNumb; i++) {
        for (int j = 0; j < cellNumb; j++) {
            if ((i+j) % 2) {
                gepard.setFillColor(77, 77, 77, 1.0f);
            } else {
                gepard.setFillColor(204, 204, 204, 1.0f);
            }
            gepard.fillRect(i * cellWidth, j * cellWidth, cellWidth, cellWidth);
        }
    }
}

int main()
{
    gepard::XSurface surface(SURFACE_SIZE, SURFACE_SIZE);
    gepard::Gepard gepard(&surface);
    gepard.setPresentMode(gepard::Gepard::PresentOnDemand);
    gepard.save();
    for (int i = 0; i < 100; i++) {
        gepard.translate(2.0, 1.5);
        generateCheckerBoard(gepard);
        gepard.finish();
    }
    gepard.setPresentMode(gepard::Gepard::PresentImmediate);
    gepard.restore();
    gepard::Image image = gepard.createImageData(200.0, 200.0);

    gepard::Image image2 = gepard.createImageData(image);

    gepard.putImageData(image, 100, 400, 10, 10, 50, 50);

    gepard.putImageData(image2, 0, 0);

    gepard.setFillColor(0, 255, 0, 1.0f);
    gepard.fillRect(400, 400, 10, 10);
    gepard::Image greenImage = gepard.getImageData(400, 400, 10, 10);
    gepard.setFillColor(255, 0, 0, 1.0f);
    gepard.fillRect(400, 400, 10, 10);
    gepard.putImageData(greenImage, 405, 405);

    gepard::Image testImage = gepard.getImageData(380, 380, 50, 50);
    gepard.setFillColor(255, 0, 0, 1.0f);
    gepard.drawImage(testImage, 200.0, 200.0, 100, 100);

    while (true) {
        std::this_thread::sleep_for(std::chrono::nanoseconds(1));   // Only for CPU sparing.
        if (surface.hasToQuit()) {
            break;
        }
    }

    return 0;
}
