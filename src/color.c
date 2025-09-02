#include "color.h"

// private functions
static uint8_t color_hue2rgb(uint16_t p, uint16_t q, uint16_t t) {
    if (t < 0) t += 255;
    if (t > 255) t -= 255;

    if (t < 43)  return p + (q - p) * t / 43;
    if (t < 128) return q;
    if (t < 171) return p + (q - p) * (171 - t) / 43;
    return p;
}

// public functions
void color_rgb2hsl(const uint8_t* rgb, uint8_t* hsl) {
    const uint8_t r = rgb[0];
    const uint8_t g = rgb[1];
    const uint8_t b = rgb[2];

    uint8_t max = (r > g) ? ((r > b) ? r : b) : ((g > b) ? g : b);
    uint8_t min = (r < g) ? ((r < b) ? r : b) : ((g < b) ? g : b);
    uint8_t delta = max - min;

    // Lightness
    uint16_t l = (max + min) / 2;

    uint16_t h = 0;
    uint16_t s = 0;

    if (delta != 0) {
        // Saturation
        if (l < 128)
            s = (uint16_t)delta * 255 / (max + min);
        else
            s = (uint16_t)delta * 255 / (510 - (max + min));

        // Hue
        if (max == r) {
            h = 43 * (g - b) / delta;
        } else if (max == g) {
            h = 85 + 43 * (b - r) / delta;
        } else {
            h = 171 + 43 * (r - g) / delta;
        }
        if ((int16_t)h < 0) h += 255;
        if (h > 255) h -= 255;
    }

    hsl[0] = (uint8_t)h;
    hsl[1] = (uint8_t)s;
    hsl[2] = (uint8_t)l;
}

void color_hsl2rgb(const uint8_t* hsl, uint8_t* rgb) {
    const uint16_t h = hsl[0];
    const uint16_t s = hsl[1];
    const uint16_t l = hsl[2];

    if (s == 0) {
        // Achromatic (gray)
        rgb[0] = rgb[1] = rgb[2] = (uint8_t)l;
        return;
    }

    uint16_t q = (l < 128) ? (l * (255 + s)) / 255 : (l + s - (l * s) / 255);
    uint16_t p = 2 * l - q;

    rgb[0] = color_hue2rgb(p, q, h + 85);
    rgb[1] = color_hue2rgb(p, q, h);
    rgb[2] = color_hue2rgb(p, q, h - 85);
}
