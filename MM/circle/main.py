import os
from math import floor

from PIL import Image
from progressbar import Bar, Percentage, ProgressBar, Timer

img1 = Image.open('诺贝尔.jpg')
img2 = Image.open('lena.jpg')

cx, cy = floor(img1.width / 2), floor(img1.height / 2)

seq = []

print('Begin processing...')

progress = ProgressBar(
    widgets=['Progress: ', Percentage(), ' ', Bar('#'), ' ', Timer()])
for radius in progress(range(0, 300, 10)):  # 对角线 / 2
  for r in range(max(cx - radius, 0), min(cx + radius, img1.width)):
    for c in range(max(cy - radius, 0), min(cy + radius, img1.height)):
      if pow(r - cx, 2) + pow(c - cy, 2) <= pow(radius, 2):
        img1.putpixel((r, c), img2.getpixel((r, c)))
  seq.append(img1.copy())

print('Show and save into res.gif...')

seq[0].save('res.gif', save_all=True,
            append_images=seq[1:], duration=0.1, loop=0)

os.system(r'start ./res.gif')

print('Successful!! Goodbye~')
