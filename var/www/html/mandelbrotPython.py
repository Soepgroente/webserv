#!/usr/bin/env python3

import numpy as np
from PIL import Image
import sys
import math
import io

def mandelbrot(h, w, max_iter):
    y, x = np.ogrid[-1.15:1.15:h*1j, -2.03:0.5:w*1j]
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

def generate_color_spectrum():
    sr = np.zeros(256, dtype=np.uint32)
    sg = np.zeros(256, dtype=np.uint32)
    sb = np.zeros(256, dtype=np.uint32)
    spectrum = []

    for i in range(256):
        sr[i] = int(127.5 * (1 + math.cos(2 * math.pi * i / 256 + 2 * math.pi / 3)))
        sg[i] = int(127.5 * (1 + math.cos(2 * math.pi * i / 256 + 4 * math.pi / 3)))
        sb[i] = 255 - int(127.5 * (1 + math.cos(2 * math.pi * i / 256 + 2 * math.pi / 3)))
        spectrum.append((sr[i], sg[i], sb[i], 255))
    
    return spectrum

def create_image(data, max_iter):
    height, width = data.shape
    img = Image.new('RGB', (width, height))
    pixels = img.load()

    spectrum = generate_color_spectrum()

    for y in range(height):
        for x in range(width):
            value = data[y, x]
            if value < max_iter:
                color = spectrum[value % 256][:3]
            else:
                color = (0, 0, 0)
            pixels[x, y] = color

    return img

width, height = 1280 * 247 / 224, 1280
max_iter = 100
data = mandelbrot(height, width, max_iter)
image = create_image(data, max_iter)

# buffer = io.BytesIO()
# image.save(buffer, format='PNG')
# buffer.seek(0)

# with open('fractal.png', 'wb') as file:
#     file.write(buffer.getvalue())

# sys.stdout.write("HTTP/1.1 201 Created\r\n")
# sys.stdout.write("Content-Type: text/plain\r\n")
# sys.stdout.write("Content-Length: 19\r\n")
# sys.stdout.write("\r\n")
# sys.stdout.write("Upload successful.\r\n")

# Generate a unique filename using timestamp
import time
filename = f"fractal_{int(time.time())}.png"
filepath = "./var/www/generated"  # Store in a subdirectory

try:
    # Ensure directory exists
    import os
    os.makedirs(filepath, exist_ok=True)
    
    # Save the image
    full_path = os.path.join(filepath, filename)
    image.save(full_path)
    
    # Send proper 201 response with location of the created resource
    sys.stdout.write("HTTP/1.1 201 Created\r\n")
    sys.stdout.write(f"Location: {filepath}/{filename}\r\n")
    sys.stdout.write("Content-Type: text/plain\r\n")
    sys.stdout.write("Content-Length: 19\r\n")
    sys.stdout.write("\r\n")
    sys.stdout.write("Upload successful.\r\n")
    sys.stdout.flush()
    
except Exception as e:
    # Handle errors gracefully
    error_message = f"Error: {str(e)}"
    sys.stdout.write("HTTP/1.1 500 Internal Server Error\r\n")
    sys.stdout.write("Content-Type: text/plain\r\n")
    sys.stdout.write(f"Content-Length: {len(error_message)}\r\n")
    sys.stdout.write("\r\n")
    sys.stdout.write(error_message)
    sys.stdout.flush()
    