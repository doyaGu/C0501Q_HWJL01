#from distutils.core import setup
#import py2exe

import re
import sys
import os.path
import errno

def ParseXMLText(src):
    result = []
    i = 0
    f = open(src, "r", encoding='UTF-8')

    while True:
        line = f.readline()
        text_re = re.compile(r"<Property name=\"(Text)\d{0,}\">(.+)<\/Property>")
        match = text_re.search(line)
        if(match):
            s = match.group(0)
            s = s[s.index(">") + 1:-11]

            try:
                if result.index(s):
                    #print("skip: " + s)
                    continue
            except Exception as e:
                result.append(s)

        items_re = re.compile(r"PublicKeyToken=\w+\">(.+)<\/Item>")
        match = items_re.search(line)
        if(match):
            s = match.group(0)
            s = s[s.index(">") + 1:-7]

            try:
                if result.index(s):
                    #print("skip: " + s)
                    continue
            except Exception as e:
                result.append(s)

        if not line:
            break

    f.close()
    return result

def ConvertListToString(ts):
    result = []
    for i in range(0, len(ts)):
        t = list(ts[i])
        for j in range(0, len(t)):
            if ord(t[j]) > 255:
                result.append(t[j])

    return result

def RemoveDuplicateElements(ts):
    for i in range(len(ts) - 1, 0, -1):
        if ts[i] == ts[i - 1]:
            del ts[i]

def CreateSelectedList(ts):
    t = []
    for i in range(0, 256):
        t.append("0u" + "%04x" % i)

    for i in range(0, len(ts)):
        t.append("0u" + "%04x" % ord(ts[i]))

    return t

def RemoveFile(filename):
    try:
        os.remove(filename)
    except  OSError as e:
        if e.errno != errno.ENOENT:
            raise print("Remove file error.")
            sys.exit()

def GenerateARGV(t):
    s = ""
    for i in range(1, len(t)):
        s = s  + " " + t[i]
    return s

def SplitText(ts):
    tt = []

    for i in range(0, len(ts)):
        s = ts[i]
        for j in range(0, len(s)):
            if ord(s[j]) > 255:
                tt.append(s[j])
                #print(s[j])
    return tt


def ParseArgv(argv, p):
    for i in range(0, len(argv)):
        s = argv[i]

        if s == p:
            return True
            break

    return False

def ParseArgOpt(argv, p):
    ts = []
    for i in range(0, len(argv)):
        s = argv[i]
        if s == p:
            ts.append(ts[3:])
            print("ParseArgOpt " + ts[3:])

def ParseArgc(argv, p):
    ts = []
    for i in range(0, len(argv)):
        s = argv[i][:len(p)]
        c = argv[i][3:]
        if s == p:
            ts.append(c)

    return ts


###############################################################################
#                                                                             #
#                                                                             #
###############################################################################

ts = []
t  = []

xml   = ""
tar   = "text.txt"
font  = ""
fontn = ""
sym   = 1
gb2313Symbol = "　、。·ˉˇ¨〃々—～‖…‘’“”〔〕〈〉《》「」『』〖〗【】±×÷∶∧∨∑∏∪∩∈∷√⊥∥∠⌒⊙∫∮≡≌≈∽∝≠≮≯≤≥∞∵∴♂♀°′″℃＄¤￠￡‰§№☆★○●◎◇◆□■△▲※→←↑↓〓ⅰⅱⅲⅳⅴⅵⅶⅷⅸⅹ⒈⒉⒊⒋⒌⒍⒎⒏⒐⒑⒒⒓⒔⒕⒖⒗⒘⒙⒚⒛⑴⑵⑶⑷⑸⑹⑺⑻⑼⑽⑾⑿⒀⒁⒂⒃⒄⒅⒆⒇①②③④⑤⑥⑦⑧⑨⑩㈠㈡㈢㈣㈤㈥㈦㈧㈨㈩ⅠⅡⅢⅣⅤⅥⅦⅧⅨⅩⅪⅫ！＂＃￥％＆＇（）＊＋，－．／０１２３４５６７８９：；＜＝＞？＠ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺ［＼］＾＿｀ａｂｃｄｅｆｇｈｉｊｋｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚ｛｜｝￣ぁあぃいぅうぇえぉおかがきぎくぐけげこごさざしじすずせぜそぞただちぢっつづてでとどなにぬねのはばぱひびぴふぶぷへべぺほぼぽまみむめもゃやゅゆょよらりるれろゎわゐゑをんァアィイゥウェエォオカガキギクグケゲコゴサザシジスズセゼソゾタダチヂッツヅテデトドナニヌネノハバパヒビピフブプヘベペホボポマミムメモャヤュユョヨラリルレロヮワヰヱヲンヴヵヶΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡΣΤΥΦΧΨΩαβγδεζηθικλμνξοπρστυφχψω︵︶︹︺︿﹀︽︾﹁﹂﹃﹄︻︼︷︸︱︳︴АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдеёжзийклмнопрстуфхцчшщъыьэюяāáǎàēéěèīíǐìōóǒòūúǔùǖǘǚǜüêɑńňǹɡㄅㄆㄇㄈㄉㄊㄋㄌㄍㄎㄏㄐㄑㄒㄓㄔㄕㄖㄗㄘㄙㄚㄛㄜㄝㄞㄟㄠㄡㄢㄣㄤㄥㄦㄧㄨㄩ─━│┃┄┅┆┇┈┉┊┋┌┍┎┏┐┑┒┓└┕┖┗┘┙┚┛├┝┞┟┠┡┢┣┤┥┦┧┨┩┪┫┬┭┮┯┰┱┲┳┴┵┶┷┸┹┺┻┼┽┾┿╀╁╂╃╄╅╆╇╈╉╊╋"

p = GenerateARGV(sys.argv)
if not (p.count("-x:") > 0 or p.count("-s:") != 1 or p.count("-t") != 1):
    print("usage:")
    print("    python font.py -x:file.xml -t:font.ttf -n")
    exit()

# xml
try:
    x = ParseArgc(sys.argv, "-x")
    for i in range(len(x)):
        if os.path.exists(x[i]):
            print("XML file:\t" + x[i])
            t = ParseXMLText(x[i])
            ts = ts + t
        else:
            print("not found:\t" + x[i])

except Exception as e:
    print("Exception, XML ttf file name.")
    sys.exit()

#font name
try:
    font = ParseArgc(sys.argv, "-t")[0]
    if os.path.exists(font):
        fontn = font[:-4] + "_new.ttf"
        print("intput TTF:\t" + font)
        print("output TTF:\t" + fontn)
except Exception as e:
    print("Exception, input ttf file name.")
    sys.exit()

#gb2313 symbol
try:
    if ParseArgc(sys.argv, "-n")[0] == "":
        sym = 0
except Exception as e:
    sym = 1


RemoveFile(tar)
RemoveFile(fontn)

#ts = ConvertListToString(ts)
#ts = CreateSelectedList(ts)

ts = SplitText(ts)

ts.sort();
RemoveDuplicateElements(ts)

f = open(tar, "w", encoding = 'UTF-8')

f.write(''.join(ts))
f.write("\n")

for i in range(0, 255):
    f.write(chr(i))
f.write("\n")

if sym == 1:
    f.write(gb2313Symbol)
else:
    f.write("\n")

f.close()

os.system("pyftsubset.exe " + font + " --text-file=" + tar + " --output-file=" + fontn)