#!/usr/bin/env python

import struct

class BMPwriter:
    """ This class writes a bitmap.
    Only bitmaps with rgb values are supported.
    """
    
    def __init__(self, size, background):
        self.size = size
        self.background = background
        self.pixels = dict()
        
    def putpixel(self, pos, color):
        if pos[0] >= self.size[0]:
            raise Exception("putpixel: x position out of range")
        if pos[0] < 0:
            raise Exception("putpixel: x position out of range")
        if pos[1] >= self.size[1]:
            raise Exception("putpixel: y position out of range")
        if pos[1] < 0:
            raise Exception("putpixel: y position out of range")
        self.pixels[pos] = color
        
    def writeRawText(self, data):
        self.openfile.write(data)
        
    def writeRaw(self, data, size):
        packstr = ''
        if size == 4:
            packstr = '<I'
            self.openfile.write(struct.pack(packstr, data))
        elif size == 2:
            packstr = '<H'
            self.openfile.write(struct.pack(packstr, data))
        elif size == 1:
            packstr = '<B'
            self.openfile.write(struct.pack(packstr, data))
        else:
            if data != 0:
                raise Exception("no format defined for this data size(" + str(size) + ")")
            packstr = '<' + str(size) + 'x'
            self.openfile.write(struct.pack(packstr))
            
    def write(self, openfile):
        self.openfile = openfile
        # write header
        self.writeRawText('BM');
        # a row must be a multiple of 4 Bytes
        bytesPerRow = self.size[0] * 3
        if bytesPerRow % 4 != 0:
            bytesPerRow += 4 - (bytesPerRow % 4)        
        self.writeRaw(54 + self.size[0] * bytesPerRow, 4)
        self.writeRaw(0, 4)
        self.writeRaw(54, 4)

        # write infoheader
        self.writeRaw(40, 4)
        self.writeRaw(self.size[0], 4)
        self.writeRaw(self.size[1], 4)
        self.writeRaw(0, 2)
        self.writeRaw(24, 2)
        self.writeRaw(0, 24)

        # write data
        for y in range(self.size[1] - 1, -1, -1):
            for x in range(0, self.size[0]):
                if (x, y) in self.pixels:
                    self.writeRaw(self.pixels[(x, y)][2], 1)
                    self.writeRaw(self.pixels[(x, y)][1], 1)
                    self.writeRaw(self.pixels[(x, y)][0], 1)
                    #print ("writing in (" + str(x) + ", " + str(y) + "): " + str(self.pixels[(x, y)]))
                else:
                    self.writeRaw(self.background[2], 1)
                    self.writeRaw(self.background[1], 1)
                    self.writeRaw(self.background[0], 1)
                    #print ("writing in (" + str(x) + ", " + str(y) + "): " + str(self.background))
            # a row must be a multiple of 4 Bytes
            BytesLeft = 4 - ((self.size[0] * 3) % 4)
            if BytesLeft == 4:
                BytesLeft = 0
            self.writeRaw(0, BytesLeft)


if __name__ == "__main__":
    outfile = open('test.bmp', 'wb')
    mybmp = BMPwriter((3, 4), (0, 0, 0))
    mybmp.putpixel((0, 0), (255, 255, 255))
    mybmp.putpixel((2, 0), (255,   0,   0))
    mybmp.putpixel((0, 3), (  0, 255,   0))
    mybmp.putpixel((2, 3), (  0,   0, 255))
    mybmp.write(outfile)
    outfile.close()
    
