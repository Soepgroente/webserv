#!/usr/bin/env python3

import numpy as np
from PIL import Image
import sys
import io

def mandelbrot(h, w, max_iter):
    y, x = np.ogrid[-1.4:1.4:h*1j, -2:0.8:w*1j]
    c = x + y*1j
    z = c
    divtime = max_iter + np.zeros(z.shape, dtype=int)

    for i in range(max_iter):
        z = z**2 + c
        diverge = z * np.conj(z) > 2**2
        div_now = diverge & (divtime == max_iter)
        divtime[div_now] = i
        z[diverge] = 2

    return divtime

def create_image(data, max_iter):
    height, width = data.shape
    img = Image.new('RGB', (width, height))
    pixels = img.load()

    for y in range(height):
        for x in range(width):
            value = data[y, x]
            if value < max_iter:
                color = (int(255 * value / max_iter), 0, 255)
            else:
                color = (0, 0, 0)
            pixels[x, y] = color

    return img

width, height = 1920, 1080
max_iter = 100
data = mandelbrot(height, width, max_iter)
image = create_image(data, max_iter)

# Save the image to a BytesIO buffer
buffer = io.BytesIO()
image.save(buffer, format='PNG')
buffer.seek(0)

# Write the HTTP response headers
sys.stdout.write("HTTP/1.1 200 OK\r\n")
sys.stdout.write("Content-Type: image/png\r\n")
sys.stdout.write("Content-Length: {}\r\n".format(len(buffer.getvalue())))
sys.stdout.write("\r\n")
sys.stdout.flush()

# Write the image content to stdout
sys.stdout.buffer.write(buffer.getvalue())