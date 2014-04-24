#! /usr/python2.6/python

import xml.dom.minidom as xdom
from optparse import OptionParser
import shutil
import sys
import os
import re
import zipfile

parser = OptionParser(usage="usage: %prog [options] checkPath project releasePolicyXml",version="%prog 1.0")
parser.add_option("-p","--precheck",action="store_true",help="Pre-check release policy before custom release.")
(options,args) = parser.parse_args()
if len(args) != 3:
    parser.print_help()
    sys.exit(1)

class Arguments(object):pass
ARGUMENTS = Arguments()
ARGUMENTS.checkPath = os.path.abspath(args[0])
ARGUMENTS.project = args[1]
ARGUMENTS.xml = os.path.abspath(args[2])

# check the arguments correctness
def checkArgument(argu):
    """ check the argument """
    if not os.path.exists(argu.checkPath):
        print >> sys.stderr,"the input checkPath '%s' does not exist!" % argu.checkPath
        sys.exit(1)
    if not os.path.exists(argu.xml):
        print >> sys.stderr,"the input xml '%s' does not exist!" % argu.xml
        sys.exit(1)

checkArgument(ARGUMENTS)

# define our own  XML-DOM class for custom release policy
class XmlDom(object):
    def __init__(self,xml):
        self.xmlDom = xdom.parse(xml)

    def getRoot(self):
        return self.xmlDom.documentElement

    def getDirList(self):
        root = self.getRoot()
        dirElement = root.getElementsByTagName("DirList")[0].getElementsByTagName("ReleaseDirList")[0].getElementsByTagName("Dir")
        dirList = map(str,[item.firstChild.nodeValue for item in dirElement if item.firstChild is not None])
        return dirList

    def getUnreleaseDirList(self):
        root = self.getRoot()
        dirElement = root.getElementsByTagName("DirList")[0].getElementsByTagName("UnReleaseDirList")[0].getElementsByTagName("Dir")
        dirList = map(str,[item.firstChild.nodeValue for item in dirElement if item.firstChild is not None])
        return dirList

    def getFileList(self):
        root = self.getRoot()
        fileElement = root.getElementsByTagName("FileList")[0].getElementsByTagName("ReleaseFileList")[0].getElementsByTagName("File")
        fileList = map(str,[item.firstChild.nodeValue for item in fileElement if item.firstChild is not None])
        return fileList

    def getUnreleaseFileList(self):
        root = self.getRoot()
        fileElement = root.getElementsByTagName("FileList")[0].getElementsByTagName("UnReleaseFileList")[0].getElementsByTagName("File")
        fileList = map(str,[item.firstChild.nodeValue for item in fileElement if item.firstChild is not None])
        return fileList

    def getKernelSourceList(self):
        root = self.getRoot()
        sourceElement = root.getElementsByTagName("KernelRelease")[0].getElementsByTagName("SourceList")[0].getElementsByTagName("Source")
        sourceList = map(str,[item.firstChild.nodeValue for item in sourceElement if item.firstChild is not None])
        return sourceList

    def getKernelBinaryList(self):
        root = self.getRoot()
        binaryElement = root.getElementsByTagName("KernelRelease")[0].getElementsByTagName("BINList")[0].getElementsByTagName("Binary")
        binaryList = map(str,[item.firstChild.nodeValue for item in binaryElement if item.firstChild is not None])
        return binaryList

    def getAppSourceList(self):
        root = self.getRoot()
        sourceElement = root.getElementsByTagName("APPRelease")[0].getElementsByTagName("SourceList")[0].getElementsByTagName("Source")
        sourceList = map(str,[item.firstChild.nodeValue for item in sourceElement if item.firstChild is not None])
        return sourceList

    def getAppBinaryList(self):
        root = self.getRoot()
        binaryElement = root.getElementsByTagName("APPRelease")[0].getElementsByTagName("BINList")[0].getElementsByTagName("Binary")
        binaryList = map(str,[item.firstChild.nodeValue for item in binaryElement if item.firstChild is not None])
        return binaryList

    def getAndroidSourceList(self):
        root = self.getRoot()
        sourceElement = root.getElementsByTagName("AndroidRelease")[0].getElementsByTagName("SourceList")[0].getElementsByTagName("Source")
        sourceList = map(str,[item.firstChild.nodeValue for item in sourceElement if item.firstChild is not None])
        return sourceList

    def getAndroidBinaryList(self):
        root = self.getRoot()
        binaryElement = root.getElementsByTagName("AndroidRelease")[0].getElementsByTagName("BINList")[0].getElementsByTagName("Binary")
        binaryList = map(str,[item.firstChild.nodeValue for item in binaryElement if item.firstChild is not None])
        return binaryList

    def getFrameworkSourceList(self):
        root = self.getRoot()
        sourceElement = root.getElementsByTagName("FrameworkRelease")[0].getElementsByTagName("SourceList")[0].getElementsByTagName("Source")
        sourceList = map(str,[item.firstChild.nodeValue for item in sourceElement if item.firstChild is not None])
        return sourceList

    def getFrameworkBinaryList(self):
        root = self.getRoot()
        binaryElement = root.getElementsByTagName("FrameworkRelease")[0].getElementsByTagName("BINList")[0].getElementsByTagName("Binary")
        binaryList = map(str,[item.firstChild.nodeValue for item in binaryElement if item.firstChild is not None])
        return binaryList

    def getFrameworkPartialList(self):
        root = self.getRoot()
        partialElement = root.getElementsByTagName("FrameworkRelease")[0].getElementsByTagName("PartialSourceList")[0].getElementsByTagName("PartialSource")
        frameworkDict = {}
        for x in partialElement:
            module = str(x.getAttribute("module"))
            base = str(x.getAttribute("base"))
            binaryList = map(str,[item.firstChild.nodeValue for item in x.getElementsByTagName("Binary") if item.firstChild is not None])
            d = {}
            d["base"] = base
            d["binary_list"] = binaryList
            frameworkDict.setdefault(module,[]).append(d) 
        return frameworkDict

#end XmlDom

# create custom release policy DOM
dom = XmlDom(ARGUMENTS.xml)

#begin SmartCheck
class SmartCheck(object):
    def __init__(self):
        """ variable initialization """
        self.source = ARGUMENTS.checkPath
        self.policyXml = ARGUMENTS.xml
        self.unMatchedList = []

    def checkEmpty(self,source):
        if not os.path.exists(source):         
            print >> sys.stdout,"Checking Path '%s' ..." % source
        else:
            if os.path.isfile(source):
                print >> sys.stderr,"Error!Target Path '%s' exists source file!" % source
                sys.exit(0)
            elif os.path.isdir(source):
                files = map(lambda x:x.rstrip(),list(os.popen("find %s -name *.c -o -name *.java" % source)))
                if files:
                    print >> sys.stderr,"Error!Target Path '%s' exists source file!" % source
                    print >> sys.stdout, "  File List: %s" % files
                    sys.exit(0)
                else:
                    print >> sys.stdout,"Checking Path '%s' ..." % source

    def checkPolicy(self,path):
        checkPath = os.path.join(self.source,path)
        if os.path.exists(checkPath):
            print >> sys.stdout,"Checking Path '%s' ..." % path
            return True
        else:
            print >> sys.stdout,"[ERROR] Path '%s' is not exists in this codebase" % path
            self.unMatchedList.append(path)
            return False

    def removePolicy(self,item):
        PolicyContent = open(self.policyXml,"r")
        string = PolicyContent.read()
        newstring = string.replace(item,'')
        PolicyContent.close()
        PolicyContent = open(self.policyXml,"w")
        PolicyContent.write(newstring)
        PolicyContent.close()
#end SmartCheck


#Start ImageCompare
class ImageCompare(object):
    def __init__(self):
        print >> sys.stdout,"Initiallizing..."
 
    def compareKernel(self,src,dst):
        
        
        print >> sys.stdout,"Compare Kernel Image Done~!"

    def compareSysImage(self,src,dst):
        print >> sys.stdout,"Compare System Image Done~!"

#end ImageCompare

checkSourceList = []
checkBinaryList = []
smartcheck = SmartCheck()
DirList = dom.getDirList()
FileList = dom.getFileList()
AndroidSourceList = dom.getAndroidSourceList()
AndroidBinaryList = dom.getAndroidBinaryList()
FrameworkSourceList = dom.getFrameworkSourceList()
FrameworkBinaryList = dom.getFrameworkBinaryList()
FrameworkPartialList = dom.getFrameworkPartialList()
AppSourceList = dom.getAppSourceList()
AppBinaryList = dom.getAppBinaryList()
unrelDirList = dom.getUnreleaseDirList()
unrelFileList = dom.getUnreleaseFileList()
KernelBinaryList = dom.getKernelBinaryList()
KernelSourceList = dom.getKernelSourceList()

#binary release policy list:
checkBinaryList.extend(AndroidBinaryList)
checkBinaryList.extend(FrameworkBinaryList)
checkBinaryList.extend(AppBinaryList)
checkBinaryList.extend(KernelBinaryList)

#source release policy list:
checkSourceList.extend(AndroidSourceList)
checkSourceList.extend(FrameworkSourceList)
checkSourceList.extend(AppSourceList)
checkSourceList.extend(DirList)
checkSourceList.extend(FileList)
checkSourceList.extend(KernelSourceList)

if options.precheck == True:
    for cl in checkBinaryList:
        smartcheck.checkPolicy(cl)
    for cl in checkSourceList:
        smartcheck.checkPolicy(cl)
    length = len(smartcheck.unMatchedList)
    if length < 1:
        print >> sys.stdout,"\nThere is not unmatched release policy item!"
    else:
        print >> sys.stdout,"\n\n\t###There are %s unmatched release policy item###" % length
        for cl in smartcheck.unMatchedList:
            print "[REMOVE]%s" % cl
            smartcheck.removePolicy(cl)
    for pl in FrameworkPartialList:
        for plItem in FrameworkPartialList[pl]:
            base = plItem["base"]
            binary_list = plItem["binary_list"]
            for bl in binary_list:
                blfull = os.path.join(base,bl)
                if smartcheck.checkPolicy(blfull) == False:
                    print "[REMOVE]%s" % blfull
                    smartcheck.removePolicy(bl)
else:
    checkBinaryList.extend(unrelDirList)
    checkBinaryList.extend(unrelFileList)
    for cl in checkBinaryList:
        cl = os.path.join(ARGUMENTS.checkPath,cl)
        smartcheck.checkEmpty(cl)
    print >> sys.stdout,"Smart Check Done~!"
