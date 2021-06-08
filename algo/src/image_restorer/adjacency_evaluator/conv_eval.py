# pip install opencv-python
# pip install matplotlib
import numpy as np
import cv2
import matplotlib.pyplot as plt

img_path = "norway.jpg"
img = cv2.imread(img_path, 1)
plt.imshow(img)
plt.show()

kernel = np.full((10, 10), 1 / 100)
print(kernel)

img_2 = cv2.filter2D(img, -1, kernel)
plt.imshow(img_2)
plt.show()
