import numpy as np

def mandelbrot(h, w, max_iter):
    y, x = np.ogrid[-1.4:1.4:h*1j, -2:0.8:w*1j]
    c = x + y*1j
    z = c
    divtime = max_iter + np.zeros(z.shape, dtype=int)

    for i in range(max_iter):
        z = z**2 + c
        diverge = z*np.conj(z) > 2**2
        div_now = diverge & (divtime == max_iter)
        divtime[div_now] = i
        z[diverge] = 2

    return divtime

# Generate Mandelbrot data
width, height = 1920, 1080
max_iter = 100
data = mandelbrot(height, width, max_iter)

# Create SVG content
print('<?xml version="1.0" encoding="UTF-8" standalone="no"?>')
print(f'<svg width="{width}" height="{height}" xmlns="http://www.w3.org/2000/svg" version="1.1">')

# Add each point as a colored rectangle
for y in range(height):
    for x in range(width):
        value = data[y, x]
        if value < max_iter:
            color = f"#{int(255 * value/max_iter):02x}00ff"
        else:
            color = "#000000"
        print(f'<rect x="{x}" y="{y}" width="1" height="1" fill="black"/>')

print('</svg>')