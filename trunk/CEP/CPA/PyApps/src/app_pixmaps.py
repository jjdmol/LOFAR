#!/usr/bin/python

from qt import QPixmap

# A QPixMap wrapper defers initialization of a pixmap until the pixmap
# is actually retrieved with the pm() method for the first time.
class QPixmapWrapper(object):
  def __init__(self,xpmstr):
    self._xpmstr = xpmstr;
    self._pm = None;
  def pm (self):
    if self._pm is None:
      self._pm = QPixmap(self._xpmstr);
    return self._pm;

exclaim = QPixmapWrapper([ "14 14 3 1",
          "       c None",
          ".      c red",
          "X      c yellow",
          "              ",
          "     ....     ",
          "    ......    ",
          "    ......    ",
          "    ......    ",
          "    ......    ",
          "    ......    ",
          "     ....     ",
          "     ....     ",
          "      ..      ",
          "              ",
          "      ..      ",
          "     ....     ",
          "      ..      " ]);
          
cancel = QPixmapWrapper(["16 16 5 1",
                         "p c #800000",
                         ". c #400000",
                         "X c #FF0000",
                         "o c #C00000",
                         "  c None",
                         "                ",
                         "                ",
                         "            X   ",
                         "    XX     Xoo  ",
                         "   XooXX  Xoo.  ",
                         "     pooXooo.   ",
                         "      poooo.    ",
                         "       XooX     ",
                         "      XoooX     ",
                         "     Xo. poX    ",
                         "     X.   po    ",
                         "    Xo     po   ",
                         "    X.      o   ",
                         "    X           ",
                         "                ",
                         "                " ])

check = QPixmapWrapper(["16 14 8 1",
                        "  c #000000",
                        ". c #400000",
                        "X c None",
                        "o c #00C000",
                        "O c #008000",
                        "+ c #C0FFC0",
                        "@ c #004000",
                        "# c None",
                        "XXXXXXXXXXXXXXX+",
                        "XXXXXXXXXXXXXX+o",
                        "XXXXXXXXXXXXX+oO",
                        "XXXXXXXXXXXX+oO@",
                        "XXXXXXXXXXX+oO@ ",
                        "XX+OXXXXXX+oO@ X",
                        "X+ooOXXXX+oO@.XX",
                        " @OooOXX+oO@ XXX",
                        "X @OooO+oO@.XXXX",
                        "XX.@OoooO@.XXXXX",
                        "XXX @OoO@ XXXXXX",
                        "XXXX @O@ XXXXXXX",
                        "XXXXX . XXXXXXXX",
                        "XXXXXXXXXXXXXXXX" ]);
          
pause_green = QPixmapWrapper(["22 15 4 1",
                       "  c None",
                       ". c #303030",
                       "X c #00FF00",
                       "o c None",
                       "                      ",
                       "      XXX.   XXX.     ",
                       "     XXX..  XXX..     ",
                       "     XXX..  XXX..     ",
                       "     XXX..  XXX..     ",
                       "     XXX..  XXX..     ",
                       "     XXX..  XXX..     ",
                       "     XXX..  XXX..     ",
                       "     XXX..  XXX..     ",
                       "     XXX..  XXX..     ",
                       "     XXX..  XXX..     ",
                       "     XXX..  XXX..     ",
                       "     XXX.   XXX.      ",
                       "                      ",
                       "                      " ]);
pause_normal = QPixmapWrapper(["22 15 4 1",
                       "  c None",
                       ". c #303030",
                       "X c #0000FF",
                       "o c None",
                       "                      ",
                       "      XXX.   XXX.     ",
                       "     XXX..  XXX..     ",
                       "     XXX..  XXX..     ",
                       "     XXX..  XXX..     ",
                       "     XXX..  XXX..     ",
                       "     XXX..  XXX..     ",
                       "     XXX..  XXX..     ",
                       "     XXX..  XXX..     ",
                       "     XXX..  XXX..     ",
                       "     XXX..  XXX..     ",
                       "     XXX..  XXX..     ",
                       "     XXX.   XXX.      ",
                       "                      ",
                       "                      " ]);


refresh = QPixmapWrapper([ "16 16 7 1",
                   "  c #000000",
                   ". c #00FF00",
                   "X c None",
                   "o c #00C000",
                   "O c #008000",
                   "+ c #004000",
                   "@ c None",
                   "XXXXXX     XXXXX",
                   "XXXX  .oo  + XXX",
                   "XXX .ooO XX   XX",
                   "XXX ooO XXXXX XX",
                   "XX .oO XXXXXXX X",
                   "XX ooO XXXXXXXXX",
                   " ..ooooO XX  XXX",
                   "X .oooO XX    XX",
                   "XX .oO XX      X",
                   "XXX O XX        ",
                   "XXXX XXXXX    XX",
                   "X XXXXXXXX    XX",
                   "XX XXXXXX    XXX",
                   "XX   XX      XXX",
                   "XXX         XXXX",
                   "XXXXX     XXXXXX"  ]);


pin_up = QPixmapWrapper([ "16 16 5 1",
                          "  c None",
                          ". c #A0A0A0",
                          "X c #707070",
                          "o c #FFFFFF",
                          "O c None",
                          "                ",
                          "                ",
                          "                ",
                          "      XX     X  ",
                          "      XoX   XX  ",
                          "      XooXXXoX  ",
                          "ooooooX.o.o.oX  ",
                          "......X.o.o.oX  ",
                          "XXXXXXX..X.X.X  ",
                          "      X.XXXXXX  ",
                          "      XXX   XX  ",
                          "      XX     X  ",
                          "                ",
                          "                ",
                          "                ",
                          "                "]);

pin_down = QPixmapWrapper([ "16 16 5 1",
                            "  c None",
                            ". c #A0A0A0",
                            "X c #707070",
                            "o c #FFFFFF",
                            "O c None",
                            "                ",
                            "                ",
                            "        XXXX    ",
                            "      XXooooX   ",
                            "    XXXooooooX  ",
                            "    XoXooo...X  ",
                            "   XooXoo...XX  ",
                            "   XooXXo..XXX  ",
                            "   XooooXXXXX   ",
                            "   Xooo....XX   ",
                            "    Xo....XXX   ",
                            "    X....XXX    ",
                            "     XXXXXX     ",
                            "                ",
                            "                ",
                            "                "]);

matrix = QPixmapWrapper(["16 16 4 1",
                         "  c None",
                         ". c None",
                         "X c #800080",
                         "o c None",
                         "... ... ....... ",
                         ".XXXX XXXX.XXXX ",
                         ".X. X.X .X.X..X ",
                         " X  X X  X X  X ",
                         ".XXXX.XXXX.XXXX ",
                         ". . . . . . . . ",
                         ".XXXX.XXXX.XXXX ",
                         " X  X X  X.X  X ",
                         ".X..X.X..X.X..X ",
                         ".XXXX XXXX.XXXX.",
                         "... ... ... ... ",
                         " XXXX XXXX XXXX ",
                         ".X. X.X .X.X..X ",
                         ".X. X.X .X.X. X ",
                         ".XXXX.XXXX.XXXX ",
                         "                "
                         ]);

view_right = QPixmapWrapper(["16 16 48 1",
                             "  c None",
                             ". c #ADADAD",
                             "X c #A5A5A5",
                             "o c #A1A1A1",
                             "O c #C1C10A",
                             "+ c #FFFF85",
                             "@ c #494949",
                             "# c #FAFAFA",
                             "$ c #F6F6F6",
                             "% c #F2F2F2",
                             "& c #F0F0F0",
                             "* c #EEEEEE",
                             "= c #ECECEC",
                             "- c #EAEAEA",
                             "; c #E8E8E8",
                             ": c #E6E6E6",
                             "> c #E4E4E4",
                             ", c #E2E2E2",
                             "< c #E0E0E0",
                             "1 c #DADADA",
                             "2 c #D6D6D6",
                             "3 c #CCCCCC",
                             "4 c #C8C8C8",
                             "5 c #C6C6C6",
                             "6 c #C0C000",
                             "7 c #AEAEAE",
                             "8 c #ACACAC",
                             "9 c #A0A0A0",
                             "0 c #585858",
                             "q c #FFFFFF",
                             "w c #FFFF00",
                             "e c #FFFFC0",
                             "r c #F9F9F9",
                             "t c #F5F5F5",
                             "y c #F3F3F3",
                             "u c #F1F1F1",
                             "i c #EDEDED",
                             "p c #E9E9E9",
                             "a c #E5E5E5",
                             "s c #303030",
                             "d c #E1E1E1",
                             "f c #DFDFDF",
                             "g c #D9D9D9",
                             "h c #D3D3D3",
                             "j c #CFCFCF",
                             "k c #C9C9C9",
                             "l c #C7C7C7",
                             "z c None",
                             "                ",
                             " 0000000000000@ ",
                             " 02gg1q0ohj3k5s ",
                             " 0gyt$q08=;><5s ",
                             " 0gtr#q0.ipadls ",
                             " 01$#qq07*-:,4s ",
                             " 0gtr#q0.ipadls ",
                             " 0gyt$q08= +<5s ",
                             " 02&u%q +- wf + ",
                             " 0h=i*q0 w6w6ws ",
                             " 0j;p-q0XOe+e6s ",
                             " 03>a: +ww+q+ww+",
                             " 0k<d,q096e+e6s ",
                             " 055l4q0 w6w6ws ",
                             " @sssss +s ws + ",
                             "           +    "]);

view_split = QPixmapWrapper(["16 16 40 1",
                             "  c None",
                             ". c #A1A1A1",
                             "X c #979797",
                             "o c #494949",
                             "O c #FAFAFA",
                             "+ c #F6F6F6",
                             "@ c #EEEEEE",
                             "# c #ECECEC",
                             "$ c #EAEAEA",
                             "% c #E8E8E8",
                             "& c #E6E6E6",
                             "* c #E4E4E4",
                             "= c #E2E2E2",
                             "- c #E0E0E0",
                             "; c #DEDEDE",
                             ": c #DCDCDC",
                             "> c #DADADA",
                             ", c #D6D6D6",
                             "< c #CCCCCC",
                             "1 c #C8C8C8",
                             "2 c #C6C6C6",
                             "3 c #ACACAC",
                             "4 c #585858",
                             "5 c #FFFFFF",
                             "6 c #F9F9F9",
                             "7 c #F5F5F5",
                             "8 c #F3F3F3",
                             "9 c #EDEDED",
                             "0 c #E9E9E9",
                             "q c #E7E7E7",
                             "w c #E5E5E5",
                             "e c #303030",
                             "r c #E1E1E1",
                             "t c #DDDDDD",
                             "y c #D9D9D9",
                             "u c #D3D3D3",
                             "i c #CFCFCF",
                             "p c #C9C9C9",
                             "a c #C7C7C7",
                             "s c None",
                             "                ",
                             " 4444444444444o ",
                             " 4,yy>543ui<p2e ",
                             " 4y87+54.#%*-2e ",
                             " 4y76O54.90wrae ",
                             " 4>+O554.@$&=1e ",
                             " 4555554.55555e ",
                             " 4444444444444e ",
                             " 43....4.....3e ",
                             " 4u#9@54.q*rt2e ",
                             " 4i%0$54.*r;:2e ",
                             " 4<*w&54.r;::2e ",
                             " 4p-r=54.t:::2e ",
                             " 422a154X22222e ",
                             " oeeeeeeeeeeeee ",
                             "                "]);                             
                             
remove = QPixmapWrapper(["16 16 15 1",
                         "  c #000000",
                         ". c #E4E4E4",
                         "X c #DEDEDE",
                         "o c #D4D4D4",
                         "O c #CECECE",
                         "+ c #BEBEBE",
                         "@ c #6E6E6E",
                         "# c #FFFFFF",
                         "$ c #E9E9E9",
                         "% c #D9D9D9",
                         "& c #C9C9C9",
                         "* c #C3C3C3",
                         "= c #B9B9B9",
                         "- c #B7B7B7",
                         "; c None",
                         "---------------#",
                         "-@@@@@@@@@@@@@@#",
                         "-@###########-@#",
                         "-@#$$$$$$$$$$-@#",
                         "-@#.. .... ..-@#",
                         "-@#X   XX   X-@#",
                         "-@#%%      %%-@#",
                         "-@#ooo    ooo-@#",
                         "-@#OOO    OOO-@#",
                         "-@#&&      &&-@#",
                         "-@#*   **   *-@#",
                         "-@#++ ++++ ++-@#",
                         "-@#==========-@#",
                         "-@------------@#",
                         "-@@@@@@@@@@@@@@#",
                         "################" ]);
                             
                             
tab_remove = QPixmapWrapper(["16 16 15 1",
                             "  c None",
                             ". c #CD4135",
                             "X c #999999",
                             "o c #820202",
                             "O c #800000",
                             "+ c #E8786C",
                             "@ c #FFC0C0",
                             "# c #D6D6D6",
                             "$ c #C8C8C8",
                             "% c #9E9E9E",
                             "& c #8C8C8C",
                             "* c #FFFFFF",
                             "= c #F1F1F1",
                             "- c #E3E3E3",
                             "; c None",
                             "                ",
                             "                ",
                             "     +@  @+     ",
                             "     o.@@.o     ",
                             "      o..o      ",
                             "      @..@      ",
                             "     @.oO.@     ",
                             "    @+o  o.     ",
                             "     o    o     ",
                             "                ",
                             "  *===X   %%%%% ",
                             "  ****$   %%%%% ",
                             " *=====X %%%%%%%",
                             "$&&&&&&&&&&&&&&&",
                             "$&**************",
                             "$&**************"  ]);
                             
tab_new = QPixmapWrapper(["16 16 16 1",
                          "  c None",
                          ". c #999999",
                          "X c #C1C10A",
                          "o c #FFFF85",
                          "O c #494600",
                          "+ c #D6D6D6",
                          "@ c #C8C8C8",
                          "# c #C0C000",
                          "$ c #9E9E9E",
                          "% c #8C8C8C",
                          "& c #FFFFFF",
                          "* c #FFFF00",
                          "= c #FFFFC0",
                          "- c #F1F1F1",
                          "; c #E3E3E3",
                          ": c None",
                          "        O       ",
                          "     O OoO O    ",
                          "    OoOO*OOoO   ",
                          "     O*#*#*O    ",
                          "    OOX=o=#OO   ",
                          "   Oo**o&o**oO  ",
                          "    OO#=o=#OO   ",
                          "     O*#*#*O    ",
                          "    OoOO*OOoO   ",
                          "     O OoO O    ",
                          "  $$$$$ O &---. ",
                          "  $$$$$   &&&&@ ",
                          " $$$$$$$ &-----.",
                          "@%%%%%%%%%%%%%%%",
                          "@%&&&&&&&&&&&&&&",
                          "@%&&&&&&&&&&&&&&" ]);

magnify = QPixmapWrapper(["16 16 9 1",
                         "  c #000000",
                         ". c #800000",
                         "X c #DCDCDC",
                         "o c #FF8000",
                         "O c #A0A0A0",
                         "+ c #C05800",
                         "@ c None",
                         "# c #C3C3C3",
                         "$ c None",
                         "@@@@@   @@@@@@@@",
                         "@@@  O#O  @@@@@@",
                         "@@ O##X##O @@@@@",
                         "@ O#X@@XX#O @@@@",
                         "@ #X@XXXXX# @@@@",
                         " O#@XXXXXX#O @@@",
                         " #X@XXXXX#X# @@@",
                         " O#XXXXXX##O @@@",
                         "@ #XXXXX#X# @@@@",
                         "@ O#XX##X#O @@@@",
                         "@@ O##X##O . @@@",
                         "@@@  O#O  .o. @@",
                         "@@@@@   @@ .o. @",
                         "@@@@@@@@@@@ .+. ",
                         "@@@@@@@@@@@@ . @",
                         "@@@@@@@@@@@@@ @@" ]);


eventnew = QPixmapWrapper(["16 16 7 1",
                           "  c None",
                           ". c #87852B",
                           "X c #918F2E",
                           "o c #FFFFFF",
                           "O c #FFFF00",
                           "+ c #FFFFC0",
                           "@ c None",
                           "                ",
                           "                ",
                           "        O       ",
                           "   O    O.   O  ",
                           "    O. +O. OO   ",
                           "    OO.XOX+O.   ",
                           "     +OOOOO.    ",
                           "   ..XOo+oOX+   ",
                           "  OOOOO+o+OOOOO ",
                           "    +XOo+oOX..  ",
                           "     .OOOOO+    ",
                           "    .O+XOX.OO   ",
                           "    OO .O+ .O   ",
                           "   O   .O    O  ",
                           "        O       ",
                           "                " ]);


view_tree = QPixmapWrapper(["16 16 5 1",
                            "  c #000000",
                            ". c None",
                            "X c #303030",
                            "o c #FFDCA8",
                            "O c None",
                            " ...............",
                            " ...............",
                            " ...XXX ........",
                            " ...X.. ........",
                            "   .X.o ..  .   ",
                            " ...    ........",
                            " ...............",
                            " ...............",
                            " ...............",
                            " ...XXX ........",
                            " ...X.. ........",
                            "   .X.o ..  .   ",
                            "....    ........",
                            "................",
                            "................",
                            "................"]);

