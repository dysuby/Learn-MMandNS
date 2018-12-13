import numpy as np
import utils

B = 8   # block size


class JPEG_Encoder:

    def __init__(self):
        self.DCT = utils.genDCTMat()
        self.qy = utils.genQY()
        self.qc = utils.genQC()
        self.zigzag = utils.genZigzag()
        self.dc_table = utils.genDC_Huffman()
        self.ac_table = utils.genAC_Huffman()

    def encode(self, img):
        """
        编码

        `img` - rgb 图像
        """
        ycbcr_img = self.__rgb2ycbcr(img)      # rgb to ycbcr

        subsampled = self.__subsampling(ycbcr_img)     # chroma subsampling

        DC = [[], [], []]
        AC = [[], [], []]
        for k in range(3):
            h, w = subsampled[k].shape
            for i in range(0, h, B):
                for j in range(0, w, B):
                    block = subsampled[k][i:i+B, j:j+B]
                    if block.shape != (B, B):
                        # 补均值形成 8*8 的块
                        dh, dw = (B - np.array(block.shape) % B) % B
                        block = np.pad(block, ((0, dh), (0, dw)), 'mean')
                    
                    dct = self.__dct(block)
                    q = self.__quantizer(dct, k)
                    z = self.__zigzag(q)
                    DC[k].append(z[0])
                    AC[k].append(self.__AC_huffman(self.__rle(z[1:]), k))

            DC[k] = self.__DC_huffman(self.__dpcm(DC[k]), k)

        return DC, AC

    def __rgb2ycbcr(self, img):
        """
        rgb 转 ycbcr
        """
        xform = np.array(
            [[.299, .587, .114], [-.168736, -.331264, .5], [.5, -.418688, -.081312]])
        ret = img.dot(xform.T)
        ret[..., [1, 2]] += 128
        return ret.astype(np.int32)

    def __subsampling(self, img):
        """
        4:2:0 二次采样
        """
        y = img[..., 0]
        cb = img[::2, ::2, 1]
        cr = img[1::2, ::2, 2]
        return [y, cb, cr]

    def __dct(self, block):
        """
        二维 DCT 变换
        """
        ret = self.DCT.dot(block - 128).dot(self.DCT.T)
        return ret

    def __quantizer(self, block, ttype):
        """
        量化

        `ttype == 0` 亮度量化

        `ttype != 0` 色差量化
        """
        Q = self.qy if ttype == 0 else self.qc
        ret = np.round(block / Q).astype(np.int32)
        return ret

    def __zigzag(self, block):
        """
        Z 字扫描
        """
        z = np.zeros((B * B), dtype=np.int32)
        for i in range(B):
            for j in range(B):
                z[self.zigzag[i, j]] = block[i, j]
        return z

    def __rle(self, block):
        """
        AC系数 RLE编码
        """
        zero_num = 0
        ret = []
        for i in range(len(block)):
            if block[i] != 0 or zero_num == 15:
                ret.append([zero_num, block[i]])
                zero_num = 0
            else:
                zero_num += 1
        if zero_num:
            while len(ret) and ret[-1] == [15, 0]:
                ret.pop()
            ret.append([0, 0])
        return ret

    def __dpcm(self, dc):
        """
        DC系数 DPCM编码
        """
        ret = [dc[0]] + [dc[i] - dc[i - 1] for i in range(1, len(dc))]
        return ret

    def __toBin(self, num):
        """
        将数字转为 01 串表示
        """
        if num == 0:
            return ''
        b = bin(np.abs(num))[2:]
        if num < 0:
            b = utils.bitwise(b)
        return b

    def __DC_huffman(self, dc, k):
        """
        DC 系数的哈夫曼编码

        `k` - 通道：`0 - y, 1 - cb, 2 - cr`
        """
        ret = []
        for i in range(len(dc)):
            b = self.__toBin(dc[i])
            size = self.dc_table[k, len(b)]
            ret.append([size, b])
        return ret

    def __AC_huffman(self, ac, k):
        """
        AC 系数的哈夫曼编码
        
        `k` - 通道：`0 - y, 1 - cb, 2 - cr`
        """
        ret = []
        for i in range(len(ac)):
            b = self.__toBin(ac[i][1])
            rz = self.ac_table[k, ac[i][0]][len(b)]
            ret.append([rz, b])
        return ret
