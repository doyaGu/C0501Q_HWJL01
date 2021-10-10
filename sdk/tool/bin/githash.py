# pylint: disable=invalid-name

import sys
import os
import os.path
import glob
import platform

git_HEAD_file = "g_H_s_h"
debugFlag     = False

os.system("git rev-parse --short HEAD > " + git_HEAD_file)

try:
    f = open(git_HEAD_file, "r", encoding = 'UTF-8')
    while True:
        line = f.readline()
        git_hash = line.strip()
        break
    f.close()

    if debugFlag:
        print("git HEAD hash: " + git_hash)

    os.remove(git_HEAD_file)
except:
    print("not found git hash, exiting...")
    sys.exit()

try:
    if os.path.exists(sys.argv[1]):
        d = sys.argv[1]
        if platform.system() == 'Windows' and not d.endswith('\\'):
            d = d + '\\'
        if platform.system() != 'Windows' and not d.endswith('/'):
            d = d + '/'

        ex = sys.argv[2].split(":")

        if debugFlag:
            print("dir: " + d)
            print(ex)

        for f in ex:
            g = (glob.glob(d + "*." + f))
            for i in g:
                print(i)
                if os.path.isfile(i):
                    new_file_name = os.path.splitext(i)[0] + "_" + git_hash + os.path.splitext(i)[-1]

                    if debugFlag:
                        print("new file name: " + new_file_name)

                    os.rename(i, new_file_name)
                else:
                    print(i + " is not a file")
    else:
        print("not found " + sys.argv[1])
except:
    print("error")