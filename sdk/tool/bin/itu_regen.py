import sys
from os.path import join, getsize
import os
import errno
import time
import re

def RemoveFile(filename):
    try:
        os.remove(filename)
    except  OSError as e:
        if e.errno != errno.ENOENT:
            raise print("Remove file error.")
            sys.exit()


def getTotalFileSize(ts):
    size  = 0
    total = 0
    for f in range(len(ts)):
        size = getsize(ts[f])
        total += size;

        text_re = re.compile(r"(\w+\.\w{3})$")
        match = text_re.search(ts[f])
        if(match):
            filename = match.group(0)

        #print(str(size) + "        " +filename)

    return total

def divList(ts, hs, p):
    size  = 0
    index = 0
    result = []

    while size < hs:
        if size + getsize(ts[index]) > hs:
            break
        size += getsize(ts[index])
        index += 1

    if p == 0:
        i = 0
        j = index
    elif p == 1:
        i = index
        j = len(ts)

    for n in range(i, j):
        f     = ts[n][0:-4] + ".xml"
        #print("filename: " + f)
        result.append(f)

    print("length of ts: " + str(len(ts)))
    print("start = " + str(i) + ", end = " + str(j) + ", count = " + str(j - i + 1))

    return result


###############################################################################
#                                                                             #
#                                                                             #
###############################################################################

startTime = time.time()
divConv   = -1

if len(sys.argv) >= 2:
    try:
        divConv = int(sys.argv[1])
    except  ValueError:
        print("divConv error, reset value")
        divConv = -1

if len(sys.argv) >= 3:
    if os.path.exists(sys.argv[2]):
        path = sys.argv[2]
    else:
        path = os.path.abspath("../../project/")
else:
    path = os.path.abspath("../../project/")

print("divConv: " + str(divConv))
print("path   : " + path)


print("find itu files in [%s]" % path)

ts     = []
ts_itu = []

RemoveFile("itu_regen.ini")

try:
    if os.path.exists(path):
        for root, dirs, files in os.walk(path):
            #print(root)
            for f in files:
                f = os.path.join(root, f)
                extname = f[-4:]
                #print(os.path.join(root, f))
                if extname == ".itu":
                    ts.append(f[:-4] + ".xml")
                    ts_itu.append(f)
                    #print(f)

    if divConv != -1:
        totalSize = getTotalFileSize(ts_itu)
        halfSize  = totalSize // 2
        print("total size: " + str(totalSize) + ", half size: " + str(halfSize))
        print("ts count    : " + str(len(ts)))
        print("ts_itu count: " + str(len(ts)))
        ts = divList(ts_itu, halfSize, divConv)

    for i in range(len(ts)):
        f = open("itu_regen.ini", "w+", encoding = 'UTF-8')
        f.write('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>' + "\n")
        f.write('<ITURegenWork>' + "\n")
        print(str(i) + ": " + ts[i])
        f.write('    ' + '<JOB'+ str(0) +' XMLFILE="' + ts[i] + "\" WORK=\"1\" JPEGQT=\"0\"/>\n")
        f.write('</ITURegenWork>' + "\n")
        f.close()
        os.system("GUIDesigner.exe")
        #break

except Exception as e:
    print("not found the path: %s" % path)
    print("Error code: " + str(e))

endTime = time.time()
print("Time taken: %d seconds." % (endTime - startTime))