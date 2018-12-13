import cv2
import numpy as np
from progressbar import Bar, Percentage, ProgressBar, Timer


def sort(pixels):
  diff = np.amax(pixels, axis=0) - np.amin(pixels, axis=0)
  color = np.argmax(diff)
  pixels = pixels[pixels[:, color].argsort()]
  return np.array_split(pixels, 2)


def distance(lut, pixel):
  return np.sqrt(np.sum((lut - pixel) ** 2, axis=1))


img = cv2.imread('redapple.jpg')

pixels = img.reshape((-1, 3)).astype(np.uint8)

print('Cutting...')

cubes = [pixels]
while len(cubes) != 256:
  left, right = sort(cubes.pop(0))
  cubes.extend([left, right])

print('Building LUT...')

lut = np.empty(shape=[0, 3])

for c in cubes:
  lut = np.append(lut, [np.mean(c, axis=0)], axis=0)

print('Converting...')

progress = ProgressBar(
    widgets=['Progress: ', Percentage(), ' ', Bar('#'), ' ', Timer()])
for i in progress(range(img.shape[0])):
  for j in range(img.shape[1]):
    img[i, j] = lut[np.argmin(distance(lut, img[i, j]))].astype(np.uint8)

print('Saving and showing...')

cv2.imwrite('res.bmp', img)

cv2.imshow('res', img)

cv2.waitKey(0)

cv2.destroyAllWindows()
