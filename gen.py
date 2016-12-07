#!/usr/bin/python

##  Author: Jacky Liu
##    Date: 7 Dec 2016

from os import listdir
from os.path import isfile, join
import sys
import re


def natural_key(string_):
    return [int(s) if s.isdigit() else s for s in re.split(r'(\d+)', string_)]
 
def timeFile(fileList, path, subfolder):
    fp = open(path + '/' + subfolder + '.txt', 'w')
    
    count = 0.033333
    
    for fileName in fileList:
        fp.write("%.6f %s/%s\n" % (count, subfolder, fileName))
        count = count + 0.033333
    
    fp.close()

def main():
    #mypath = './rgb'
    if len(sys.argv) < 2:
        print "please input folder path, >python ./gen.py ./dir\ndir contains rgb and depth folder"
        sys.exit(1)

    mypath = sys.argv[1]
    print(mypath)
    

    rgbfiles = [f for f in listdir(mypath+'/rgb') if isfile(join(mypath+'/rgb', f))]
    depthfiles = [f for f in listdir(mypath+'/depth') if isfile(join(mypath+'/depth', f))]

    intersection = list(set(rgbfiles) & set(depthfiles))




    intersection.sort(key=natural_key)
    #print(onlyfiles)
    
    timeFile(intersection, mypath, 'rgb')
    timeFile(intersection, mypath, 'depth')


if __name__ == "__main__":
    main()

