from jpegEncode import JPEG_Encoder
from jpegDecode import JPEG_Decoder
import cv2
import numpy as np
from PIL import Image

def test(i):
    img = cv2.imread('./test/input{}.jpg'.format(i))[..., ::-1]

    print('Begin to encode')

    encoder = JPEG_Encoder()
    DC, AC = encoder.encode(img)

    print('Encode done')

    print('Begin to decode')

    decoder = JPEG_Decoder()
    output = decoder.decode(DC, AC, img.shape[0], img.shape[1])
    
    print('Decode done')

    cv2.imwrite('./res/{}.jpg'.format(i), output[..., ::-1])


    cv2.imshow('output{}'.format(i), output[..., ::-1])
    cv2.waitKey(0)
    cv2.destroyAllWindows()

    # 计算压缩率和失真率
    gif = Image.open('./test/input{}.gif'.format(i)).convert('RGB')
    print('{} JPEG MSE：{}'.format('input{}'.format(i), calMSE(img, output)))
    print('{} GIF MSE：{}'.format('input{}'.format(i), calMSE(img, np.array(gif))))


def calMSE(ori, out):
    """
    计算 MSE
    """
    return (np.square(ori - out)).mean()

# def calCompression(img, DC, AC):
#     """
#     计算压缩率
#     """
#     ori = img.shape[0] * img.shape[1] * 3 * 8

#     aft = 0
#     for k in range(3):
#         for pair in DC[k]:
#             aft += len(pair[0] + pair[1])
#         for block in AC[k]:
#             for pair in block:
#                 aft += len(pair[0] + pair[1])
#     return aft / ori

if __name__ == '__main__':
    test(1)
    test(2)
