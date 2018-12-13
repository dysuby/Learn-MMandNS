import numpy as np
import copy
import utils

B = 8

class JPEG_Decoder:
    def __init__(self):
        self.DCT = utils.genDCTMat()
        self.z = utils.genZigzag()
        self.q = utils.genQY(), utils.genQC()
        self.dc_table = utils.genDC_Huffman()
        self.ac_table = utils.genAC_Huffman()

    def decode(self, DC, AC, h, w):
        """
        解码
        """
        DC, AC = copy.deepcopy(DC), copy.deepcopy(AC)
        self.__iHuffman(DC, AC)

        blocks = [[], [], []]
        for k in range(3):
            dc = self.__idpcm(DC[k])
            for i in range(len(DC[k])):
                ac = self.__irle(AC[k][i])
                z = self.__izigzag([dc[i]] + ac)
                q = self.__iquantizer(z, k != 0)
                d = self.__idct(q)
                blocks[k].append(d)

        blocks = [np.array(blocks[i]) for i in range(3)]
        s = self.__isubsample(blocks, h, w)

        img = self.__ycbcr2rgb(s)

        return img

    def __toNum(self, b):
        """
        将 01 串转回值

        `b` - 01 串
        """
        val = 0
        if len(b):
            val = -int(utils.bitwise(b), 2) if b[0] == '0' else int(b, 2)
        return val

    def __iHuffman(self, DC, AC):
        """
        反 Huffman 编码
        
        `DC` - `DC[channel][index]`

        `AC` - `AC[channel][blocks][index]`
        """
        for k in range(3):
            for i in range(len(DC[k])):
                # 恢复 DC，[[size, val]...]
                b = DC[k][i][1]
                DC[k][i] = self.__toNum(b)

                # 恢复 AC，[[[runlength/size, val]...]...]，与 DC 块数一样
                for j in range(len(AC[k][i])):
                    b = AC[k][i][j][1]
                    s = len(b)
                    r = 0
                    for row in range(len(self.ac_table[k])):
                        if self.ac_table[k][row][s] == AC[k][i][j][0]:
                            r = row
                            break
                    AC[k][i][j] = [r, self.__toNum(b)]

    def __idpcm(self, dc):
        """
        反 DPCM 编码
        """
        ret = [dc[0]]
        for i in range(1, len(dc)):
            ret.append(ret[i - 1] + dc[i])
        return ret

    def __irle(self, rle):
        """
        反 RLE 编码
        """
        ac = []
        for pair in rle:
            ac = ac + [0 for j in range(pair[0])] + [pair[1]]
            if pair == [0, 0]:
                while len(ac) < B * B - 1:
                    ac.append(0)
        return ac

    def __izigzag(self, block):
        """
        转成 8*8 矩阵
        """
        ret = np.zeros((B, B), dtype=np.int32)
        for i in range(B):
            for j in range(B):
                ret[i, j] = block[self.z[i, j]]
        return ret

    def __iquantizer(self, block, ttype):
        """
        反量化

        `ttype == 0` 亮度反量化

        `ttype != 0` 色差反量化
        """
        ret = block * self.q[ttype]
        return ret

    def __idct(self, block):
        """
        二维 IDCT 变换
        """
        ret = self.DCT.T.dot(block).dot(self.DCT) + 128
        return ret

    def __isubsample(self, blocks, h, w):
        """
        反 4:2:0 二次采样

        `blocks` - `[y, cb, cr]` 块数组

        `h`, `w` - 原图高宽
        """
        y, cb, cr = blocks
        c = np.array([cb, cr])
        img = np.zeros((h, w, 3))
        
        # 计算采样后图片的大小
        yw = np.int32(np.ceil(w / B))
        cw = np.int32(np.ceil(w / 2 / B))

        for i in range(0, h):
            block_row = (i // B) * yw, (i // 2 // B) * cw
            row = i % 8, (i // 2) % 8
            for j in range(0, w):
                # 通过 i, j 计算在块中的位置
                img[i, j, 0] = y[block_row[0] + (j // B), row[0], j % B]
                v = j // 2
                img[i, j, [1, 2]] = c[:, block_row[1] + (v // B), row[1], v % B]

        return img

    def __ycbcr2rgb(self, img):
        """
        ycbcr 转 rgb
        """
        xform = np.array(
            [[1, 0, 1.402], [1, -.344136, -.714136], [1, 1.772, 0]])
        rgb = img.astype(np.float)
        rgb[..., [1, 2]] -= 128
        rgb = rgb.dot(xform.T)
        np.putmask(rgb, rgb > 255, 255)
        np.putmask(rgb, rgb < 0, 0)
        return np.uint8(rgb)
