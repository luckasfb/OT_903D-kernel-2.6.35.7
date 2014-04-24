#! /usr/python2.6/python

import xml.dom.minidom as xdom
from optparse import OptionParser
import shutil
import sys
import os
import re
import zipfile

parser = OptionParser(usage="usage: %prog [options] releaseSrc releaseDest project releasePolicyXml",version="%prog 1.0")
parser.add_option("-d","--dump",action="store_true",help="dump the binary modules dependency information")
(options,args) = parser.parse_args()
if len(args) != 4:
    parser.print_help()
    sys.exit(1)

class Arguments(object):pass
ARGUMENTS = Arguments()
ARGUMENTS.releaseSrc = os.path.abspath(args[0])
ARGUMENTS.releaseDest = os.path.abspath(args[1])
ARGUMENTS.project = args[2]
ARGUMENTS.xml = os.path.abspath(args[3])

# check the arguments correctness
def checkArgument(argu):
    """ check the argument """
    if not os.path.exists(argu.releaseSrc):
        print >> sys.stderr,"the input releaseSrc '%s' does not exist!" % argu.releaseSrc
        sys.exit(1)
    if not os.path.exists(argu.releaseDest):
        print >> sys.stderr,"the input releaseDest '%s' does not exist!" % argu.releaseDest
        sys.exit(1)
    if not os.path.exists(argu.xml):
        print >> sys.stderr,"the input xml '%s' does not exist!" % argu.xml
        sys.exit(1)
    projTargetFolder = "%s/out/target/product/%s" % (argu.releaseSrc,argu.project)
    if not os.path.exists(projTargetFolder):
        print >> sys.stderr,"the input project '%s' is illegal or the codebase '%s' had not full built yet!please check it!" % (argu.project,argu.releaseSrc)
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

###################################################
#       release relative path definition
###################################################

out = "out"
outTarget = os.path.join(out,"target")
outProduct = os.path.join(outTarget,"product",ARGUMENTS.project)
outSystem = os.path.join(outProduct,"system")
outCommon = os.path.join(outTarget,"common")
outIntermediate = os.path.join(outProduct,"obj")
outCommonIntermediate = os.path.join(outCommon,"obj")
outHost = os.path.join(out,"host","linux-x86")
outHostBin = os.path.join(outHost, "bin")
outHostIntermediate = os.path.join(outHost,"obj")

vendor = "vendor/mediatek/%s/artifacts" % ARGUMENTS.project

# the target module release path
archiveFolder = os.path.join(ARGUMENTS.releaseSrc,outIntermediate,"STATIC_LIBRARIES")
sharelibFolder = os.path.join(ARGUMENTS.releaseSrc,outSystem,"lib")
executeBinFolder = os.path.join(ARGUMENTS.releaseSrc,outSystem,"bin")
executeXbinFolder = os.path.join(ARGUMENTS.releaseSrc,outSystem,"xbin")
appSrcSystem = os.path.join(ARGUMENTS.releaseSrc,outSystem,"app")
frameworkFolder = os.path.join(ARGUMENTS.releaseSrc,outSystem,"framework")

sharelibIntermediate = os.path.join(ARGUMENTS.releaseSrc,outIntermediate,"lib")
frameworkIntermediate = os.path.join(ARGUMENTS.releaseSrc,outCommonIntermediate,"JAVA_LIBRARIES")
appIntermediate = os.path.join(ARGUMENTS.releaseSrc,outCommonIntermediate,"APPS")

# the host module release path
archiveHostFolder = os.path.join(ARGUMENTS.releaseSrc,outHostIntermediate,"STATIC_LIBRARIES")
sharelibHostFolder = os.path.join(ARGUMENTS.releaseSrc,outHost,"lib")
sharelibHostIntermediate = os.path.join(ARGUMENTS.releaseSrc,outHostIntermediate,"lib")
executeHostFolder = os.path.join(ARGUMENTS.releaseSrc,outHostBin)
frameworkHostFolder = os.path.join(ARGUMENTS.releaseSrc,outHost,"framework")

# the vendor relative paths that save the bianry release module
archiveVendor = os.path.join(ARGUMENTS.releaseDest,vendor,outIntermediate,"STATIC_LIBRARIES")
sharelibVendor = os.path.join(ARGUMENTS.releaseDest,vendor,outSystem,"lib")
sharelibVendorIntermediate = os.path.join(ARGUMENTS.releaseDest,vendor,outIntermediate,"lib")
executeBinVendor = os.path.join(ARGUMENTS.releaseDest,vendor,outSystem,"bin")
executeXbinVendor = os.path.join(ARGUMENTS.releaseDest,vendor,outSystem,"xbin")
frameworkVendor = os.path.join(ARGUMENTS.releaseDest,vendor,outSystem,"framework")
frameworkVendorIntermediate = os.path.join(ARGUMENTS.releaseDest,vendor,outCommonIntermediate,"JAVA_LIBRARIES")
appDestVendor = os.path.join(ARGUMENTS.releaseDest,vendor,outSystem,"app")
kernelDestVendor = os.path.join(ARGUMENTS.releaseDest,vendor,"kernel")

archiveHostVendor = os.path.join(ARGUMENTS.releaseDest,vendor,outHostIntermediate,"STATIC_LIBRARIES")
sharelibHostVendor = os.path.join(ARGUMENTS.releaseDest,vendor,outHost,"lib")
sharelibHostVendorIntermediate = os.path.join(ARGUMENTS.releaseDest,vendor,outHostIntermediate,"lib")
executeHostVendor = os.path.join(ARGUMENTS.releaseDest,vendor,outHostBin)
frameworkHostVendor = os.path.join(ARGUMENTS.releaseDest,vendor,outHost,"framework")
destCls = os.path.join(ARGUMENTS.releaseDest,vendor,"cls")
destJar = os.path.join(ARGUMENTS.releaseDest,vendor,"jar")

# the LOCAL_PATH -> module_id record file path
releasePath = os.path.join(ARGUMENTS.releaseSrc,outProduct,"release")
releasePathHash = os.path.join(releasePath,"path_module_maptable")
installModulePathHash = os.path.join(releasePath,"module_installedpath_maptable")
# define a module_id list which save all the binary release modules' module_id
# this list is used to generate the module dependency for MP release
binModules = []

#define target project flag
flag_targetProject = True
if ARGUMENTS.project == "generic" or ARGUMENTS.project == "banyan_addon":
    flag_targetProject = False

##################################################
# get the LOCAL_PATH -> module_id mapping table
##################################################

def getReleasePathTable():
    if not os.path.exists(releasePathHash):
        print >> sys.stderr,"Error!the path map file '%s' does not exist!" % releasePathHash
        sys.exit(1)
    pathTable = {}
    pattern = re.compile("(\S+)\s*->\s*(\S+)")
    output = open(releasePathHash,"r")
    for oput in output.readlines():
        match = pattern.match(oput)
        if match:
            key = match.group(1)
            key = re.sub("//", "/", key)
            value = match.group(2)
            value = re.sub("//", "/", value)
            pathTable.setdefault(key,[]).append(value)
    output.close()
    return pathTable

def getReleaseModuleTable():
    if not os.path.exists(releasePathHash):
        print >> sys.stderr,"Error!the path map file '%s' does not exist!" % releasePathHash
        sys.exit(1)
    moduleTable = {}
    pattern = re.compile("(\S+)\s*->\s*(\S+)")
    output = open(releasePathHash,"r")
    for oput in output.readlines():
        match = pattern.match(oput)
        if match:
            key = match.group(2)
            key = re.sub("//", "/", key)
            value = match.group(1)
            value = re.sub("//", "/", value)
            moduleTable[key] = value
    output.close()
    return moduleTable

def getInstalledPathTable():
    if not os.path.exists(installModulePathHash):
        print >> sys.stderr,"Error!the path map file '%s' does not exist!" % installModulePathHash
        sys.exit(1)
    moduleTable = {}
    pattern = re.compile("(\S+)\s*->\s*(\S+)")
    output = open(installModulePathHash,"r")
    for oput in output.readlines():
        match = pattern.match(oput)
        if match:
            key = match.group(1)
            key = re.sub("//", "/", key)
            value = match.group(2)
            value = re.sub("//", "/", value)
            moduleTable[key] = value
    output.close()
    return moduleTable

pathModuleIDTable = getReleasePathTable()
moduleIDPathTable = getReleaseModuleTable()
moduleIDInstalledPathTable = getInstalledPathTable()

###################################################
#    all class definitions of the ReleaseType
###################################################

class DirRelease(object):
    def __init__(self):
        """ dir release initialization """
        self.dirs = dom.getDirList()
        self.src = ARGUMENTS.releaseSrc
        self.dest = ARGUMENTS.releaseDest

    def getDirReleaseList(self):
        return self.dirs

    def sourceRelease(self):
        for d in self.dirs:
            sourceDir = os.path.join(self.src,d)
            destinationDir = os.path.join(self.dest,d)
            if not os.path.exists(sourceDir):
                print >> sys.stderr,"Error!Dir Release Source Directory '%s' does not exists!" % sourceDir
                sys.exit(2)
            if not os.path.isdir(sourceDir):
                print >> sys.stderr,"Error!%s in DirList is not a directory,check your releasePolicy!" % sourceDir
                sys.exit(2)
            print >> sys.stdout,"Dir Release:release %s ..." % sourceDir
            if not os.path.exists(destinationDir):
                os.makedirs(destinationDir)
            os.system("rsync -a --delete --force %s/ %s" % (sourceDir,destinationDir))

    def release(self):
        self.sourceRelease()

# end DirRelease

class FileRelease(object):
    def __init__(self):
        """ file release initialization """    
        self.files = dom.getFileList()
        self.src = ARGUMENTS.releaseSrc
        self.dest = ARGUMENTS.releaseDest

    def getFileReleaseList(self):
        return self.files

    def sourceRelease(self):
        for f in self.files:
            sourceFile = os.path.join(self.src,f)
            destinationFile = os.path.join(self.dest,f)
            if not os.path.exists(sourceFile):
                print >> sys.stderr,"Error!File Release Source File '%s' does not exists!" % sourceFile
                sys.exit(3)
            if not os.path.isfile(sourceFile):
                print >> sys.stderr,"Error!%s in FileList is not a file,check your releasePolicy!" % sourceFile
                sys.exit(3)
            dirPath = os.path.dirname(destinationFile)
            if not os.path.exists(dirPath):
                os.makedirs(dirPath)
            print >> sys.stdout,"File Release:release %s ..." % sourceFile
            os.system("rsync -a %s %s" % (sourceFile,destinationFile))

    def release(self):
        self.sourceRelease()

# end FileRelease

class KernelRelease(object):
    def __init__(self):
        """ kernel release initialization """
        self.sources = dom.getKernelSourceList()
        self.binarys = dom.getKernelBinaryList()
        self.src = ARGUMENTS.releaseSrc
        self.dest = ARGUMENTS.releaseDest
        self.kernelDestVendor = kernelDestVendor

    def getKernelSourceList(self):
        return self.sources

    def getKernelBinaryList(self):
        return self.binarys

    def sourceRelease(self):
        for ks in self.sources:
            source = os.path.join(self.src,ks)
            destination = os.path.join(self.dest,ks)
            if not os.path.exists(source):
                print >> sys.stderr,"Error!Kernel Source Release File/Dir '%s' does not exists!" % source
                sys.exit(4)
            if os.path.isdir(source):
                print >> sys.stdout,"Kernel Source Release:release %s ..." % source
                if not os.path.exists(destination):
                    os.makedirs(destination)
                os.system("rsync -a --delete --force %s/ %s" % (source,destination))
            elif os.path.isfile(source):
                dirPath = os.path.dirname(destination)
                if not os.path.exists(dirPath):
                    os.makedirs(dirPath)
                print >> sys.stdout,"Kernel Source Release:release %s ..." % source
                os.system("rsync -a %s %s" % (source,destination))
                 
    def binaryRelease(self):
        for kb in self.binarys:
            makefile = os.path.join(self.src,kb,"Makefile")
            if os.path.exists(makefile):
                destDir = os.path.join(self.dest,kb)
                if os.path.exists(destDir):
                    os.system("rm -rf %s" % destDir)
                self.releaseHeader(kb)
                self.releaseKo(kb)
                destMakefile = os.path.join(destDir,"Makefile")
                if not os.path.exists(os.path.join(self.dest,kb)):
                    os.makedirs(os.path.join(self.dest,kb))
                os.system("touch %s;echo 'obj-  := dummy.o' > %s" % (destMakefile,destMakefile))
            else :
                destDir = os.path.join(self.dest,kb)
                if os.path.exists(destDir):
                    os.system("rm -rf %s" % destDir)
                self.releaseHeader(kb)
                self.releaseObj(kb)
                self.releaseMak(kb)
 
    def release(self):
        self.sourceRelease()
        self.binaryRelease()
    
    def releaseMak(self,kbFolder):
        srcDir = os.path.join(self.src,kbFolder)
        srcObjs = map(lambda x:x.rstrip(),list(os.popen("find %s -name '*.o' -exec basename {} \\;" % srcDir)))
        pathList = kbFolder.split('/')
        for i in range(len(pathList)-1,0,-1):
            makDir = '/'.join(pathList[:i])
            makefile = os.path.join(os.path.join(self.dest, makDir), "Makefile")
            if os.path.exists(makefile):
                writefile = []
                makefileOutput = open(makefile,"r")
                for line in makefileOutput.readlines():
                    for pattern in srcObjs:
                        matchObj = re.match(r'^\s*obj-', line, re.M)
                        if matchObj:
                            break;
                        line = re.sub(pattern+'\\b',os.path.splitext(pattern)[0]+'.module',line) 
                        line = re.sub(os.path.splitext(pattern)[0]+'.module.new'+'\\b',pattern+'.new',line)
                    writefile.append(line)
                makefileOutput.close()
                makefileInput = open(makefile, "w")
                makefileInput.writelines(writefile)
                makefileInput.close()

    def releaseObj(self,kbFolder):
        srcDir = os.path.join(self.src,kbFolder)
        srcObjs = map(lambda x:x.rstrip(),list(os.popen("find %s -name '*.o'" % srcDir)))
        for obj in srcObjs:
            obj_module = "%s.module" % os.path.splitext(os.path.basename(obj))[0]
            destDir = os.path.join(self.kernelDestVendor,os.path.dirname(obj[len(self.src)+1:]))
            if not os.path.exists(destDir):
                os.makedirs(destDir)
            print >> sys.stdout,"Kernel Binary(.o) Release:release %s ..." % obj
            os.system("rsync -a %s %s/%s" % (obj,destDir,obj_module))

#    def modifyMakefile(self,objfile,destKbFolder,destMaks):
#        objfile = os.path.basename(objfile)
#        objfile_slash = '/%s' % objfile
#        objfile_equal = '=%s' % objfile
#        objfile_bracket = ')%s' % objfile
#        objfile_b = ' %s' % objfile
#        objModulefile_slash = "/%s.module" % os.path.splitext(objfile)[0]
#        objModulefile_equal = "=%s.module" % os.path.splitext(objfile)[0]
#        objModulefile_bracket = ")%s.module" % os.path.splitext(objfile)[0]
#        objModulefile = " %s.module" % os.path.splitext(objfile)[0]
#        makefiles = destMaks   
#        for makefile in makefiles:
#            writefile = []
#            makefile = makefile.rstrip()
#            makefileOutput = open(makefile,"r")
#            for line in makefileOutput.readlines():
#                line = re.sub(re.escape(objfile_slash),objModulefile_slash,line)
#                line = re.sub(re.escape(objfile_equal),objModulefile_equal,line)
#                line = re.sub(re.escape(objfile_bracket),objModulefile_bracket,line)
#                line = re.sub(objfile_b,objModulefile,line)
#                writefile.append(line)
#            makefileOutput.close()
#            makefileInput = open(makefile,"w")
#            makefileInput.writelines(writefile)
#            makefileInput.close()

    def releaseHeader(self,kbFolder):
        srcDir = os.path.join(self.src,kbFolder)
        srcHeader = list(os.popen("find %s -name '*.h'" % srcDir))
        for head in srcHeader:
            head = head.rstrip()
            destDir = os.path.join(self.dest,os.path.dirname(head[len(self.src)+1:]))
            if not os.path.exists(destDir):
                os.makedirs(destDir)
            print >> sys.stdout,"Kernel Binary(header) Release:release %s ..." % head
            os.system("rsync -a %s %s" % (head,destDir))

    def releaseKo(self,kbFolder):
        koDir = os.path.join(sharelibVendor,"modules")
        if not os.path.exists(koDir):
            os.system("mkdir -p %s" % koDir)
        srcDir = os.path.join(self.src,kbFolder)
        srcKos = list(os.popen("find %s -name '*.ko'" % srcDir))
        for ko in srcKos:
            ko = ko.rstrip()
            print >> sys.stdout,"Kernel Binary(.ko) Release:release %s ..." % ko
            os.system("cp -a %s %s" % (os.path.join(sharelibFolder,"modules",os.path.basename(ko)),koDir))

#    def releaseCfile(self,kbFolder):
#        srcDir = os.path.join(self.src,kbFolder)
#        srcCfile = list(os.popen("find %s -name '*.c'" % srcDir))
#        for cfile in srcCfile:
#            cfile = cfile.rstrip()
#            cfileName = os.path.basename(cfile)
#            ctmpfile = "%s.tmp" % cfile
#            os.system("cat /dev/null > %s" % ctmpfile)
#            destDir = os.path.join(self.dest,os.path.dirname(cfile[len(self.src)+1:]))
#            if not os.path.exists(destDir):
#                os.makedirs(destDir)
#            print >> sys.stdout,"Kernel Binary Release:release %s ..." % cfile
#            os.system("rsync -a %s %s/%s" % (ctmpfile,destDir,cfileName))
#            os.remove(ctmpfile)
         
# end KernelRelease

class AndroidRelease(object):
    def __init__(self):
        """ android release initialization """
        self.sources = dom.getAndroidSourceList()
        self.binarys = dom.getAndroidBinaryList()
        self.src = ARGUMENTS.releaseSrc
        self.dest = ARGUMENTS.releaseDest
        self.archiveFolder = archiveFolder 
        self.sharelibFolder = sharelibFolder
        self.executeBinFolder = executeBinFolder
        self.executeXbinFolder = executeXbinFolder 
        self.sharelibIntermediate = sharelibIntermediate
        self.sharelibHostIntermediate = sharelibHostIntermediate

        self.frameworkFolder = frameworkFolder
        self.frameworkIntermediate = frameworkIntermediate
        self.frameworkHostFolder = frameworkHostFolder
        self.frameworkVendor = frameworkVendor
        self.frameworkVendorIntermediate = frameworkVendorIntermediate
        self.frameworkHostVendor = frameworkHostVendor

        self.archiveHostFolder = archiveHostFolder
        self.sharelibHostFolder = sharelibHostFolder
        self.executeHostFolder = executeHostFolder

        self.archiveVendor = archiveVendor
        self.sharelibVendor = sharelibVendor
        self.sharelibHostVendorIntermediate = sharelibHostVendorIntermediate
        self.executeBinVendor = executeBinVendor
        self.executeXbinVendor = executeXbinVendor
        self.sharelibVendorIntermediate = sharelibVendorIntermediate
        
        self.archiveHostVendor = archiveHostVendor
        self.sharelibHostVendor = sharelibHostVendor
        self.executeHostVendor = executeHostVendor

        self.releasePath = releasePath
        self.releasePathTable = pathModuleIDTable
        self.releaseModuleTable = moduleIDPathTable
        self.intallledModulePathTable = moduleIDInstalledPathTable

    def sourceRelease(self):
        for dr in self.sources:
            flag = True
            if dr.find("/") == -1:
                mdIdList = self.transformModuleId(dr)
                for md in mdIdList:
                    if self.releaseModuleTable.has_key(md):
                        flag = False 
                        self.sourceReleaseByDep(md)
                        self.sourceReleaseByP(md)
            if flag:
                source = os.path.join(self.src,dr)
                destination = os.path.join(self.dest,dr)
                if not os.path.exists(source):
                    print >> sys.stderr,"Error!Android Release Source Directory '%s' does not exists!" % source
                    sys.exit(6)
                if os.path.isdir(source):
                    print >> sys.stdout,"Android Source Release:release %s ..." % source
                    if not os.path.exists(destination):
                        os.makedirs(destination)
                    os.system("rsync -a --delete --force %s/ %s" % (source,destination))
                elif os.path.isfile(source):
                    dirPath = os.path.dirname(dr)
                    dirPath = os.path.join(self.dest,dirPath)
                    if not os.path.exists(dirPath):
                        os.system("mkdir -p %s" % dirPath)
                    print >> sys.stdout,"Android Source Release:release %s ..." % source
                    os.system("cp -f %s %s" % (source,destination))

    def sourceReleaseByDep(self,moduleId):
        pattern1 = re.compile("LOCAL_DEP_VAR_LIST += (\S+)")
        pattern2 = re.compile("LOCAL_MAKEFILE = (\S+)")
        depFile = os.path.join(self.releasePath,"%s.dep" % moduleId)
        depFileOutput = open(depFile,"r")
        for line in depFileOutput.readlines():
            match1 = pattern1.match(line)
            match2 = pattern2.match(line)
            if match1:
                releasepart = match1.group(1) 
                source = os.path.join(self.src,releasepart)
                destination = os.path.join(self.dest,releasepart)
                if not os.path.exists(source):
                    print >> sys.stderr,"Error!Android Release Source File or Directory '%s' does not exists!" % source
                    sys.exit(6)
                if os.path.isfile(source):
                    dirPath = os.path.dirname(destination)
                    if not os.path.exists(dirPath):
                        os.makedirs(dirPath)
                    print >> sys.stdout,"Android Source Release:release %s ..." % source
                    os.system("rsync -a %s %s" % (source,destination))
                elif os.path.isdir(source):
                    if not os.path.exists(destination):
                        os.makedirs(destination)
                    print >> sys.stdout,"Android Release:release %s ..." % source
                    os.system("rsync -a --delete --force %s/ %s" % (source,destination))
            elif match2:
                releasepart = match2.group(1) 
                source = os.path.join(self.src,releasepart)
                destination = os.path.join(self.dest,releasepart)
                if not os.path.exists(source):
                    print >> sys.stderr,"Error!Android Release Source File or Directory '%s' does not exists!" % source
                    sys.exit(6)
                if os.path.isfile(source):
                    dirPath = os.path.dirname(destination)
                    if not os.path.exists(dirPath):
                        os.makedirs(dirPath)
                    print >> sys.stdout,"Android Source Release:release %s ..." % source
                    os.system("rsync -a %s %s" % (source,destination))
        depFileOutput.close()

    def sourceReleaseByP(self,moduleId):
        pattern = re.compile("MODULE\.(.*?)\.(.*?)\.(.*)")
        moduleMatch = pattern.match(moduleId)
        if moduleMatch:
            moduleTarget = moduleMatch.group(1)
            moduleClass = moduleMatch.group(2)
            moduleName = moduleMatch.group(3)
            if moduleTarget == "TARGET":
                searchFolder = os.path.join(self.src,outIntermediate,moduleClass,"%s_intermediates" % moduleName)
            elif moduleTarget == "HOST":
                searchFolder = os.path.join(self.src,outHostIntermediate,moduleClass,"%s_intermediates" % moduleName)
            dotPfiles = map(lambda x:x.rstrip(),list(os.popen("find %s -name '*.P'" % searchFolder)))
            for dotPfile in dotPfiles:
                dotPfileOutput = open(dotPfile,"r")
                for line in dotPfileOutput.readlines():
                    if line.find("\\") != -1 or line.find(":") == -1:
                        continue
                    else:
                        releaseHeaders = line.replace(":","").strip().split(" ")
                        for header in releaseHeaders:
                            source = os.path.join(self.src,releasepart)
                            destination = os.path.join(self.dest,releasepart)
                            if not os.path.exists(source):
                                print >> sys.stderr,"Error!Android Release Source File or Directory '%s' does not exists!" % source
                                sys.exit(6)
                            if os.path.isfile(source):
                                dirPath = os.path.dirname(destination)
                                if not os.path.exists(dirPath):
                                    os.makedirs(dirPath)
                                print >> sys.stdout,"Android Source Release:release %s ..." % source
                                os.system("rsync -a %s %s" % (source,destination))
                dotPfileOutput.close()

    def transformModuleId(self,moduleName):
        mdList = ["MODULE.TARGET.SHARED_LIBRARIES.%s" % moduleName,
                  "MODULE.TARGET.STATIC_LIBRARIES.%s" % moduleName,
                  "MODULE.TARGET.EXECUTABLES.%s" % moduleName,
                  "MODULE.TARGET.JAVA_LIBRARIES.%s" % moduleName,
                  "MODULE.HOST.SHARED_LIBRARIES.%s" % moduleName,
                  "MODULE.HOST.STATIC_LIBRARIES.%s" % moduleName,
                  "MODULE.HOST.EXECUTABLES.%s" % moduleName,
                  "MODULE.HOST.JAVA_LIBRARIES.%s" % moduleName]
        return mdList

    def binaryRelease(self):
        for dr in self.binarys:
            flag = True 
            if dr.find("/") == -1:
                mdIdList = self.transformModuleId(dr)
                for md in mdIdList:
                    if self.releaseModuleTable.has_key(md):
                        # -------------------------------------
                        # save the moduleIdLIst into binModules
                        binModules.extend(md)
                        # -------------------------------------
                        flag = False 
                        # remove the destination relative part
                        destPart = os.path.join(self.dest,self.releaseModuleTable.get(md))
                        if flag_targetProject:
                            if os.path.exists(destPart):
                                os.system("rm -rf %s" % destPart)
                            self.releaseHeader(self.releaseModuleTable.get(md))
                        self.copyBinary(md)
            if flag:    
                srcPart = os.path.join(self.src,dr)
                if not os.path.exists(srcPart):
                    print >> sys.stderr,"Error!Android Release Binary Directory '%s' does not exists!" % srcPart
                    sys.exit(6)
                # remove the destination relative part
                destPart = os.path.join(self.dest,dr)
                if flag_targetProject:
                    if os.path.exists(destPart):
                        os.system("rm -rf %s" % destPart)
                    self.releaseHeader(dr)
                self.getBinary(dr)

    def releaseHeader(self,abFolder):
        srcDir = os.path.join(self.src,abFolder)
        srcHeader = list(os.popen("find %s -name '*.h'" % srcDir))
        for head in srcHeader:
            head = head.rstrip()
            destDir = os.path.join(self.dest,os.path.dirname(head[len(self.src)+1:]))
            if not os.path.exists(destDir):
                os.makedirs(destDir)
            print >> sys.stdout,"Android Binary Release:release %s ..." % head
            os.system("rsync -a %s %s" % (head,destDir))

    def getBinary(self,drFolder):
        drFolder = os.path.join(self.src,drFolder)
        # find all the binary release Android.mk under given folder
        androidMkList = map(lambda x:x.rstrip(),list(os.popen("find %s -name Android.mk" % drFolder)))
        androidMkPaths = map(lambda x:os.path.dirname(x),androidMkList)
        # release binaries through the file "path_module_maptable"
        for adrPath in androidMkPaths:
            # adrPath is an absolute path,so for using the LOCAL_PATH -> module_id mapping relationship,remove the prefix(self.src)
            adrPath = adrPath[len(self.src)+1:]
            # touch the destination Android.mk
            destAndroidMk = os.path.join(self.dest,adrPath,"Android.mk")
            if not os.path.exists(os.path.join(self.dest,adrPath)):
                os.makedirs(os.path.join(self.dest,adrPath))
            os.system("touch %s;echo '#empty' > %s" % (destAndroidMk,destAndroidMk))
            if self.releasePathTable.has_key(adrPath):
                moduleIdList = self.releasePathTable.get(adrPath)
                for md in moduleIdList:
                    self.copyBinary(md)
                # -------------------------------------
                # save the moduleIdLIst into binModules
                binModules.extend(moduleIdList)
                # -------------------------------------

    def copyInstalledModule(self,moduleId):
        if self.intallledModulePathTable.has_key(moduleId):
            installedModule = self.intallledModulePathTable.get(moduleId)
            installedPath = os.path.dirname(installedModule)
            installedPath_dst = os.path.join(self.dest, vendor, installedPath)
            installedModule_src = os.path.join(self.src, installedModule)
            if not os.path.exists(installedModule_src):
                print >> sys.stdout,"Android Binary Release: %s does not exist..." % installedModule_src
            else:
                if not os.path.exists(installedPath_dst):
                    os.system("mkdir -p %s" % installedPath_dst)
                print >> sys.stdout,"Android Binary Release:release %s ..." % installedModule
                os.system("cp -f %s %s" % (installedModule_src, installedPath_dst))

    def copyBinary(self,moduleId):
        self.copyInstalledModule(moduleId)
        moduleIdPattern = re.compile("MODULE\.(.*?)\.(.*?)\.(.*)")
        moduleMatch = moduleIdPattern.match(moduleId)
        if moduleMatch:
            moduleTarget = moduleMatch.group(1)
            moduleClass = moduleMatch.group(2)
            moduleName = moduleMatch.group(3)
            if moduleClass == "STATIC_LIBRARIES":
                if moduleTarget == "TARGET":
                    source = os.path.join(self.archiveFolder,"%s_intermediates/%s.a" % (moduleName,moduleName))
                    destination = os.path.join(self.archiveVendor,"%s_intermediates/%s.a" % (moduleName,moduleName))
                elif moduleTarget == "HOST":
                    source = os.path.join(self.archiveHostFolder,"%s_intermediates/%s.a" % (moduleName,moduleName))
                    destination = os.path.join(self.archiveHostVendor,"%s_intermediates/%s.a" % (moduleName,moduleName))
                if os.path.exists(source):
                    dirPath = os.path.dirname(destination)
                    if not os.path.exists(dirPath):
                        os.makedirs(dirPath)
                    print >> sys.stdout,"Android Binary Release:release %s ..." % source
                    os.system("rsync -a %s %s" % (source,destination))
            elif moduleClass == "SHARED_LIBRARIES":
                if moduleTarget  == "TARGET":
                    # copy the share library to TARGET_OUT_INTERMEDIATE_LIBRARIES for dependency
                    source = os.path.join(self.sharelibIntermediate,"%s.so" % moduleName)
                    destination = os.path.join(self.sharelibVendorIntermediate,"%s.so" % moduleName)
                    if os.path.exists(source):
                        dirPath = os.path.dirname(destination)
                        if not os.path.exists(dirPath):
                            os.makedirs(dirPath)
                        print >> sys.stdout,"Android Binary Release:release %s ..." % source
                        os.system("rsync -a %s %s" % (source,destination))
                elif moduleTarget == "HOST":
                    source = os.path.join(self.sharelibHostIntermediate,"%s.so" % moduleName)
                    destination = os.path.join(self.sharelibHostVendorIntermediate,"%s.so" % moduleName)
                    if os.path.exists(source):
                        dirPath = os.path.dirname(destination)
                        if not os.path.exists(dirPath):
                            os.makedirs(dirPath)
                        print >> sys.stdout,"Android Binary Release:release %s ..." % source
                        os.system("rsync -a %s %s" % (source,destination))
            elif moduleClass == "JAVA_LIBRARIES":
                if moduleTarget == "HOST":
                    source = os.path.join(self.frameworkHostFolder,"%s.jar" % moduleName)     
                    destination = os.path.join(self.frameworkHostVendor,"%s.jar" % moduleName)
                    if not os.path.exists(source):
                        print >> sys.stderr,"Error!Android Release Binary '%s' does not exists!" % source
                        sys.exit(7)
                    if not os.path.exists(destination):
                        dirPath = os.path.dirname(destination)
                        os.makedirs(dirPath)
                    print >> sys.stdout,"Android Binary Release:release %s ..." % source
                    os.system("rsync -a %s %s" % (source,destination))
                if moduleTarget == "TARGET":
                    sourceSysJar = os.path.join(self.frameworkFolder,"%s.jar" % moduleName)
                    destinationSysJar = os.path.join(self.frameworkVendor,"%s.jar" % moduleName)
                    sourceJavalibJar = os.path.join(self.frameworkIntermediate,"%s_intermediates" % moduleName,"javalib.jar")
                    destinationJavalibJar = os.path.join(self.frameworkVendorIntermediate,"%s_intermediates" % moduleName,"javalib.jar")
                    sourceClassesJar = os.path.join(self.frameworkIntermediate,"%s_intermediates" % moduleName,"classes.jar")
                    destinationClassesJar = os.path.join(self.frameworkVendorIntermediate,"%s_intermediates" % moduleName,"classes.jar")
                    if os.path.exists(sourceSysJar):
                        self.copyJar(sourceSysJar,destinationSysJar)
                    if os.path.exists(sourceJavalibJar):
                        self.copyJar(sourceJavalibJar,destinationJavalibJar)
                    if os.path.exists(sourceClassesJar):
                        self.copyJar(sourceClassesJar,destinationClassesJar)

    def copyJar(self,src,dest):
        dirPath = os.path.dirname(dest)
        if not os.path.exists(dirPath):
            os.makedirs(dirPath)
        print >> sys.stdout,"Android Binary Release:release %s ..." % src
        os.system("rsync -a %s %s" % (src,dest))

    def release(self):
        if flag_targetProject:
            self.sourceRelease()
        self.binaryRelease()

# end  AndroidRelease

class FrameworkRelease(object):
    def __init__(self):
        """ framework release initialization """
        self.sources = dom.getFrameworkSourceList()
        self.binarys = dom.getFrameworkBinaryList()
        self.partials = dom.getFrameworkPartialList()
        self.src = ARGUMENTS.releaseSrc
        self.dest = ARGUMENTS.releaseDest
        self.frameworkFolder = frameworkFolder
        self.frameworkIntermediate = frameworkIntermediate
        self.frameworkHostFolder = frameworkHostFolder
        self.frameworkVendor = frameworkVendor
        self.frameworkVendorIntermediate = frameworkVendorIntermediate
        self.frameworkHostVendor = frameworkHostVendor
        self.appIntermediate = appIntermediate
        self.destCls = destCls
        self.destJar = destJar
        
        self.releasePathTable = pathModuleIDTable

    def sourceRelease(self):
        for frm in self.sources:
            source = os.path.join(self.src,frm)
            destination = os.path.join(self.dest,frm)
            if not os.path.exists(source):
                print >> sys.stderr,"Error!Framework Release Source Directory '%s' does not exists!" % source
                sys.exit(5)
            print >> sys.stdout,"Framework Source Release:release %s ..." % source
            if not os.path.exists(destination):
                os.makedirs(destination)
            os.system("rsync -a --delete --force %s/ %s" % (source,destination))

    def binaryRelease(self):
        for fw in self.binarys:
            # remove the destination relative part
            destPart = os.path.join(self.dest,fw)
            if flag_targetProject:
                if os.path.exists(destPart):
                    os.system("rm -rf %s" % destPart)
                self.releaseHeader(fw)
            self.getBinary(fw)

    def releaseHeader(self,fwFolder):
        srcDir = os.path.join(self.src,fwFolder)
        srcHeader = list(os.popen("find %s -name '*.h'" % srcDir))
        for head in srcHeader:
            head = head.rstrip()
            destDir = os.path.join(self.dest,os.path.dirname(head[len(self.src)+1:]))
            if not os.path.exists(destDir):
                os.makedirs(destDir)
            print >> sys.stdout,"Framework Binary Release:release %s ..." % head
            os.system("rsync -a %s %s" % (head,destDir))

    def getBinary(self,fwFolder):
        fwFolder = os.path.join(self.src,fwFolder)
        if not os.path.exists(fwFolder):
            print >> sys.stderr,"Error!Framework Release Source Directory/File '%s' does not exists!" % fwFolder 
            sys.exit(7)
        androidMkList = map(lambda x:x.rstrip(),list(os.popen("find %s -name Android.mk" % fwFolder)))
        androidMkPaths = map(lambda x:os.path.dirname(x),androidMkList)
        # release binaries through the file "path_module_maptable"
        for adrPath in androidMkPaths:
            # adrPath is an absolute path,so for using the LOCAL_PATH -> module_id mapping relationship,remove the prefix(self.src)
            adrPath = adrPath[len(self.src)+1:]
            if self.releasePathTable.has_key(adrPath):
                moduleIdList = self.releasePathTable.get(adrPath)
                for md in moduleIdList:
                    self.copyBinary(md)
                # -------------------------------------
                # save the moduleIdLIst into binModules
                binModules.extend(moduleIdList)
                # -------------------------------------

    def copyBinary(self,moduleId):
        moduleIdPattern = re.compile("MODULE\.(.*?)\.(.*?)\.(.*)")
        moduleMatch = moduleIdPattern.match(moduleId)
        if moduleMatch:
            moduleTarget = moduleMatch.group(1)
            moduleClass = moduleMatch.group(2)
            moduleName = moduleMatch.group(3)
            if moduleClass == "JAVA_LIBRARIES":
                if moduleTarget == "HOST":
                    source = os.path.join(self.frameworkHostFolder,"%s.jar" % moduleName)     
                    destination = os.path.join(self.frameworkHostVendor,"%s.jar" % moduleName)
                    if not os.path.exists(source):
                        print >> sys.stderr,"Error!Framework Release Binary '%s' does not exists!" % source
                        sys.exit(7)
                    if not os.path.exists(destination):
                        dirPath = os.path.dirname(destination)
                        os.makedirs(dirPath)
                    print >> sys.stdout,"Framework Binary Release:release %s ..." % source
                    os.system("rsync -a %s %s" % (source,destination))
                if moduleTarget == "TARGET":
                    sourceSysJar = os.path.join(self.frameworkFolder,"%s.jar" % moduleName)
                    destinationSysJar = os.path.join(self.frameworkVendor,"%s.jar" % moduleName)
                    sourceJavalibJar = os.path.join(self.frameworkIntermediate,"%s_intermediates" % moduleName,"javalib.jar")
                    destinationJavalibJar = os.path.join(self.frameworkVendorIntermediate,"%s_intermediates" % moduleName,"javalib.jar")
                    sourceClassesJar = os.path.join(self.frameworkIntermediate,"%s_intermediates" % moduleName,"classes.jar")
                    destinationClassesJar = os.path.join(self.frameworkVendorIntermediate,"%s_intermediates" % moduleName,"classes.jar")
                    if os.path.exists(sourceSysJar):
                        self.copyJar(sourceSysJar,destinationSysJar)
                    if os.path.exists(sourceJavalibJar):
                        self.copyJar(sourceJavalibJar,destinationJavalibJar)
                    if os.path.exists(sourceClassesJar):
                        self.copyJar(sourceClassesJar,destinationClassesJar)

    def copyJar(self,src,dest):
        dirPath = os.path.dirname(dest)
        if not os.path.exists(dirPath):
            os.makedirs(dirPath)
        print >> sys.stdout,"Framework Binary Release:release %s ..." % src
        os.system("rsync -a %s %s" % (src,dest))
        
    def partialRelease(self):
        for fw in self.partials:
            appFlag = False
            classesJar = os.path.join(self.frameworkIntermediate,"%s_intermediates/classes.jar" % fw)
            for element in self.partials[fw]: 
                base = element["base"]
                binary_list = element["binary_list"]
                if not os.path.exists(classesJar):
                    classesJar = os.path.join(self.appIntermediate,"%s_intermediates/classes.jar" % fw)
                    if not os.path.exists(classesJar):
                        print >> sys.stderr,"Error!Framework Partial Release Binary '%s' does not exists!" % classesJar
                        sys.exit(7)
                    else: appFlag = True
                zfile = zipfile.ZipFile(classesJar)
                if appFlag:
                    classes = os.path.join(self.appIntermediate,"%s_intermediates/classes" % fw)
                else:
                    classes = os.path.join(self.frameworkIntermediate,"%s_intermediates/classes" % fw)
                zfile.extractall(classes)
                if os.path.exists(".tmp"):
                    shutil.rmtree(".tmp")
                print >> sys.stdout,"Framework Partial Release ..."
                # release relative class files
                for bi in binary_list:
                    if not os.path.exists(os.path.join(self.src,base,bi)):
                        print >> sys.stderr,"Error!Framework Partial Release '%s' does not exists!" % os.path.join(self.src,base,bi)
                        sys.exit(7)
                    if os.path.isdir(os.path.join(self.src,base,bi)):
                        # remove the destination binary relative directory
                        os.system("rm -rf %s" % os.path.join(self.dest,base,bi))
                        os.system("mkdir -p %s" % os.path.join(".tmp",bi))
                        os.system("mkdir -p %s" % os.path.join(self.destCls,bi))
                        os.system("mkdir -p %s" % os.path.join(self.destJar,fw))
                        os.system("cp -a %s %s" % (os.path.join(classes,bi,"*"),os.path.join(".tmp",bi)))
                        os.system("cp -a %s %s" % (os.path.join(classes,bi,"*"),os.path.join(destCls,bi)))
                    elif os.path.isfile(os.path.join(self.src,base,bi)):
                        # remove the destination binary relative file
                        os.system("rm -rf %s" % os.path.join(self.dest,base,bi))
                        biPath = os.path.dirname(bi)
                        biBaseName = os.path.splitext(bi)[0]
                        os.system("mkdir -p %s" % os.path.join(".tmp",biPath))
                        os.system("mkdir -p %s" % os.path.join(self.destCls,biPath))
                        os.system("mkdir -p %s" % os.path.join(self.destJar,fw))
                        os.system("cp -a %s* %s" % (os.path.join(classes,biBaseName),os.path.join(".tmp",biPath)))
                        os.system("cp -a %s* %s" % (os.path.join(classes,biBaseName),os.path.join(destCls,biPath)))
                if not os.path.exists(os.path.join(destJar,fw,"policy.jar")):
                    print >> sys.stdout,"Framework Partial Release:create the %s/policy.jar ..." % fw
                    os.system("jar -cf %s -C .tmp ." % os.path.join(destJar,fw,"policy.jar"))
                else:
                    print >> sys.stdout,"Framework Partial Release:add class files into %s/policy.jar ..." % fw
                    os.system("jar -uf %s -C .tmp ." % os.path.join(destJar,fw,"policy.jar"))
                # release relative aidl files
                if not os.path.exists(os.path.join(self.src,base)):
                    print >> sys.stderr,"Error!Framework Partial Release '%s' does not exists!" % os.path.join(self.src,base)
                    sys.exit(7)
                if not os.path.isdir(os.path.join(self.src,base)):
                    print >> sys.stderr,"Error!Framework Partial Release the base '%s' must be a folder!" % os.path.join(self.src,base)
                    sys.exit(7)
                aidlFiles = map(lambda x:x.rstrip(),list(os.popen("find %s -name '*.aidl'" % os.path.join(self.src,base))))
                for aidl in aidlFiles:
                    vendorAidl = os.path.join(self.destCls,aidl[len(os.path.join(self.src,base))+1:])
                    if not os.path.exists(os.path.dirname(vendorAidl)):
                        os.makedirs(os.path.dirname(vendorAidl))
                    print >> sys.stdout,"Framework Partial Release:release %s ..." % aidl
                    os.system("rsync -a %s %s" % (aidl,vendorAidl))
                if os.path.exists(".tmp"):
                    shutil.rmtree(".tmp") 
                # remove the generated classes folder
                if os.path.exists(classes):
                    shutil.rmtree(classes)

    def release(self):
        if flag_targetProject:
            self.sourceRelease()
        self.binaryRelease()
        self.partialRelease()

# end FrameworkRelease

class AppRelease(object):
    def __init__(self):
        """ app release initialization """
        self.sources = dom.getAppSourceList()
        self.binarys = dom.getAppBinaryList()
        self.src = ARGUMENTS.releaseSrc
        self.dest = ARGUMENTS.releaseDest 

        self.appSrcSystem = appSrcSystem
        self.appDestVendor = appDestVendor

        self.frameworkFolder = frameworkFolder
        self.frameworkIntermediate = frameworkIntermediate
        self.frameworkHostFolder = frameworkHostFolder
        self.frameworkVendor = frameworkVendor
        self.frameworkVendorIntermediate = frameworkVendorIntermediate
        self.frameworkHostVendor = frameworkHostVendor

        self.sharelibFolder = sharelibFolder
        self.sharelibHostFolder = sharelibHostFolder
        self.sharelibIntermediate = sharelibIntermediate
        self.sharelibHostIntermediate = sharelibHostIntermediate
        self.sharelibVendor = sharelibVendor
        self.sharelibHostVendor = sharelibHostVendor
        self.sharelibHostVendorIntermediate = sharelibHostVendorIntermediate
        self.sharelibVendorIntermediate = sharelibVendorIntermediate

        self.releasePathTable = pathModuleIDTable

    def sourceRelease(self):
        for ap in self.sources:
            source = os.path.join(self.src,ap)
            destination = os.path.join(self.dest,ap) 
            if not os.path.exists(source):
                print >> sys.stderr,"Error!App Release Source Directory '%s' does not exists!" % source
                sys.exit(5)
            print >> sys.stdout,"App Source Release:release %s ..." % source
            if not os.path.exists(destination):
                os.makedirs(destination)
            os.system("rsync -a --delete --force %s/ %s" % (source,destination))

    def binaryRelease(self):
        for ap in self.binarys:
            # remove the destination relative part
            destPart = os.path.join(self.dest,ap)
            if flag_targetProject:
                if os.path.exists(destPart):
                    os.system("rm -rf %s" % destPart)
                self.releaseHeader(ap)
            self.getBinary(ap)

    def releaseHeader(self,apFolder):
        srcDir = os.path.join(self.src,apFolder)
        srcHeader = list(os.popen("find %s -name '*.h'" % srcDir))
        for head in srcHeader:
            head = head.rstrip()
            destDir = os.path.join(self.dest,os.path.dirname(head[len(self.src)+1:]))
            if not os.path.exists(destDir):
                os.makedirs(destDir)
            print >> sys.stdout,"APP Binary Release:release %s ..." % head
            os.system("rsync -a %s %s" % (head,destDir))

    def getBinary(self,apFolder):
        apFolder = os.path.join(self.src,apFolder)
        androidMkList = map(lambda x:x.rstrip(),list(os.popen("find %s -name Android.mk" % apFolder)))
        androidMkPaths = map(lambda x:os.path.dirname(x),androidMkList)
        # release binaries through the file "path_module_maptable"
        for adrPath in androidMkPaths:
            # adrPath is an absolute path,so for using the LOCAL_PATH -> module_id mapping relationship,remove the prefix(self.src)
            adrPath = adrPath[len(self.src)+1:]
            if self.releasePathTable.has_key(adrPath):
                moduleIdList = self.releasePathTable.get(adrPath)
                for md in moduleIdList:
                    self.copyBinary(md)
                # -------------------------------------
                # save the moduleIdLIst into binModules
                binModules.extend(moduleIdList)
                # -------------------------------------

    def copyBinary(self,moduleId):
        moduleIdPattern = re.compile("MODULE\.(.*?)\.(.*?)\.(.*)")
        moduleMatch = moduleIdPattern.match(moduleId)
        if moduleMatch:
            moduleTarget = moduleMatch.group(1)
            moduleClass = moduleMatch.group(2)
            moduleName = moduleMatch.group(3)
            if moduleClass == "APPS":
                source = "%s/%s.apk" % (self.appSrcSystem,moduleName)
                destination = "%s/%s.apk" % (self.appDestVendor,moduleName)
                if not os.path.exists(source):
                    print >> sys.stderr,"Error!App Release Binary '%s' does not exists!" % source
                    sys.exit(5)
                dirPath = os.path.dirname(destination)
                if not os.path.exists(dirPath):
                    os.makedirs(dirPath)
                print >> sys.stdout,"App Binary Release:release %s ..." % source
                os.system("rsync -a %s %s" % (source,destination))
            elif moduleClass == "JAVA_LIBRARIES":
                if moduleTarget == "HOST":
                    source = os.path.join(self.frameworkHostFolder,"%s.jar" % moduleName)     
                    destination = os.path.join(self.frameworkHostVendor,"%s.jar" % moduleName)
                    if not os.path.exists(source):
                        print >> sys.stderr,"Error!APP Release Binary '%s' does not exists!" % source
                        sys.exit(7)
                    if not os.path.exists(destination):
                        dirPath = os.path.dirname(destination)
                        os.makedirs(dirPath)
                    print >> sys.stdout,"APP Binary Release:release %s ..." % source
                    os.system("rsync -a %s %s" % (source,destination))
                if moduleTarget == "TARGET":
                    sourceSysJar = os.path.join(self.frameworkFolder,"%s.jar" % moduleName)
                    destinationSysJar = os.path.join(self.frameworkVendor,"%s.jar" % moduleName)
                    sourceJavalibJar = os.path.join(self.frameworkIntermediate,"%s_intermediates" % moduleName,"javalib.jar")
                    destinationJavalibJar = os.path.join(self.frameworkVendorIntermediate,"%s_intermediates" % moduleName,"javalib.jar")
                    sourceClassesJar = os.path.join(self.frameworkIntermediate,"%s_intermediates" % moduleName,"classes.jar")
                    destinationClassesJar = os.path.join(self.frameworkVendorIntermediate,"%s_intermediates" % moduleName,"classes.jar")
                    if os.path.exists(sourceSysJar):
                        self.copyJar(sourceSysJar,destinationSysJar)
                    if os.path.exists(sourceJavalibJar):
                        self.copyJar(sourceJavalibJar,destinationJavalibJar)
                    if os.path.exists(sourceClassesJar):
                        self.copyJar(sourceClassesJar,destinationClassesJar)
            elif moduleClass == "SHARED_LIBRARIES":
                # copy the share library to system folder for build system.img
                if moduleTarget  == "TARGET":
                    source = os.path.join(self.sharelibFolder,"%s.so" % moduleName)
                    destination = os.path.join(self.sharelibVendor,"%s.so" % moduleName)
                    if os.path.exists(source):
                        dirPath = os.path.dirname(destination)
                        if not os.path.exists(dirPath):
                            os.makedirs(dirPath)
                        print >> sys.stdout,"APP Binary Release:release %s ..." % source
                        os.system("rsync -a %s %s" % (source,destination))
                    # copy the share library to TARGET_OUT_INTERMEDIATE_LIBRARIES for dependency
                    source = os.path.join(self.sharelibIntermediate,"%s.so" % moduleName)
                    destination = os.path.join(self.sharelibVendorIntermediate,"%s.so" % moduleName)
                    if os.path.exists(source):
                        dirPath = os.path.dirname(destination)
                        if not os.path.exists(dirPath):
                            os.makedirs(dirPath)
                        print >> sys.stdout,"APP Binary Release:release %s ..." % source
                        os.system("rsync -a %s %s" % (source,destination))
                elif moduleTarget == "HOST":
                    source = os.path.join(self.sharelibHostFolder,"%s.so" % moduleName)
                    destination = os.path.join(self.sharelibHostVendor,"%s.so" % moduleName)
                    if os.path.exists(source):
                        dirPath = os.path.dirname(destination)
                        if not os.path.exists(dirPath):
                            os.makedirs(dirPath)
                        print >> sys.stdout,"APP Binary Release:release %s ..." % source
                        os.system("rsync -a %s %s" % (source,destination))
                    source = os.path.join(self.sharelibHostIntermediate,"%s.so" % moduleName)
                    destination = os.path.join(self.sharelibHostVendorIntermediate,"%s.so" % moduleName)
                    if os.path.exists(source):
                        dirPath = os.path.dirname(destination)
                        if not os.path.exists(dirPath):
                            os.makedirs(dirPath)
                        print >> sys.stdout,"APP Binary Release:release %s ..." % source
                        os.system("rsync -a %s %s" % (source,destination))

    def copyJar(self,src,dest):
        dirPath = os.path.dirname(dest)
        if not os.path.exists(dirPath):
            os.makedirs(dirPath)
        print >> sys.stdout,"APP Binary Release:release %s ..." % src
        os.system("rsync -a %s %s" % (src,dest))

    def release(self):
        if flag_targetProject:
            self.sourceRelease()
        self.binaryRelease()

# end AppRelease

# this class is used for some misc release files/folders
# TODO:maybe we'll use hardcode here
class MiscRelease(object):
    def __init__(self):
        """ misc release initialization """
        self.src = ARGUMENTS.releaseSrc
        self.dest = ARGUMENTS.releaseDest
        self.prj = ARGUMENTS.project
        self.unreleaseFolder = dom.getUnreleaseDirList()
        self.unreleaseFile = dom.getUnreleaseFileList()

    def release(self):
        self.releaseHeaderUnderOut()
        self.removeUnreleasePart()
        self.modifyKconfig()
        self.ProjectSpecify()
        self.others()
#        self.workaroundKernelModule()
 
    def releaseHeaderUnderOut(self):
        self.outProduct = outProduct
        self.releaseHeader(os.path.join(self.outProduct,"obj/include"))
        
    def releaseHeader(self,folder):
        srcDir = os.path.join(self.src,folder)
        srcHeader = list(os.popen("find %s -name '*.h'" % srcDir))
        for head in srcHeader:
            head = head.rstrip()
            destDir = os.path.join(self.dest,vendor,os.path.dirname(head[len(self.src)+1:]))
            if not os.path.exists(destDir):
                os.makedirs(destDir)
            print >> sys.stdout,"Misc Release:release %s ..." % head
            os.system("rsync -a %s %s" % (head,destDir))

    def removeUnreleasePart(self):
        for d in self.unreleaseFolder:
            print >> sys.stdout,"Misc Release:removing %s ..." % os.path.join(self.dest,d)
            os.system("rm -rf %s" % os.path.join(self.dest,d))
        for f in self.unreleaseFile:
            print >> sys.stdout,"Misc Release:removing %s ..." % os.path.join(self.dest,f)
            os.system("rm -rf %s" % os.path.join(self.dest,f))

    def modifyKconfig(self):
        kconfig = os.path.join(self.dest,"mediatek/source/kernel/Kconfig")
        kconfigOutput = open(kconfig,"r")
        kconfigWrite = []
        flag = True
        for line in kconfigOutput.readlines():
            if line == "if ARCH_MT6516\n":
                flag = False
            elif line == "endif\n" and flag == False:
                flag = True
            elif flag:
                kconfigWrite.append(line)
        kconfigOutput.close()
        kconfigInput = open(kconfig,"w")
        kconfigInput.writelines(kconfigWrite)
        kconfigInput.close()

    def ProjectSpecify(self):
        prj_productfile = os.path.join("build/target/product", "%s.mk" % self.prj)
        prj_config = os.path.join("mediatek/config", "%s/" % self.prj)
        prj_customerfolder = os.path.join("mediatek/custom", "%s/" % self.prj)
        os.system("rsync -a %s %s" % (os.path.join(self.src, prj_productfile), os.path.join(self.dest, prj_productfile)))
        os.system("rsync -a %s %s" % (os.path.join(self.src, prj_config), os.path.join(self.dest, prj_config)))
        os.system("rsync -a %s %s" % (os.path.join(self.src, prj_customerfolder), os.path.join(self.dest, prj_customerfolder)))

    def workaroundKernelModule(self):
        KOpathsrc = os.path.join(sharelibFolder,"modules")
        KOpathdst = os.path.join(sharelibVendor,"modules")
        if not os.path.exists(KOpathdst):
            os.system("mkdir -p %s" % KOpathdst)
        os.system("cp -a %s %s" % (os.path.join(KOpathsrc,"c*.ko"),KOpathdst))
        os.system("cp -a %s %s" % (os.path.join(KOpathsrc,"wlan.ko"),KOpathdst))
        os.system("cp -a %s %s" % (os.path.join(KOpathsrc,"xlog.ko"),KOpathdst))
        os.system("cp -a %s %s" % (os.path.join(KOpathsrc,"mt6573_m4u.ko"),KOpathdst))
        os.system("cp -a %s %s" % (os.path.join(KOpathsrc,"mtk_fm_priv.ko"),KOpathdst))
        os.system("cp -a %s %s" % (os.path.join(KOpathsrc,"mt6573_mfv_kernel_driver.ko"),KOpathdst))

    def others(self):
        signtool = "signapk.jar"
        src = os.path.join(self.src,outHost,"framework",signtool)
        dest = os.path.join(self.dest,vendor,outHost,"framework")
        if not os.path.exists(src):
            print >> sys.stderr,"the sign tool '%s' does not exist!" % src
            sys.exit(1)
        if not os.path.exists(dest):
            os.system("mkdir -p %s" % dest)
        os.system("cp -f %s %s" % (src,os.path.join(dest,signtool)))

# end MiscRelease

# dump the dependency information for MP release
def dumpDep(binMods):
    print >> sys.stdout,"dump dependency information ..."
    binmds = binMods
    targetTxt = os.path.join(ARGUMENTS.releaseDest,vendor,"target.txt")
    if os.path.exists(targetTxt):
        os.system("rm -rf %s" % targetTxt)
    os.system("mkdir -p %s" % os.path.dirname(targetTxt))
    os.system("touch %s" % targetTxt)
    targetPat1 = re.compile("LOCAL_BUILT_MODULE\s*=\s*(\S+)")
    targetPat2 = re.compile("LOCAL_INSTALLED_MODULE\s*=\s*(\S+)")
    dependPat = re.compile("LOCAL_DEP_BUILT_FILES\s*\+=\s*(\S+)")
    for binmd in binmds:
        targetMatchFlag1 = 0
        targetMatchFlag2 = 0
        dep_files = []
        depFile = os.path.join(releasePath,"%s.dep" % binmd) 
        depFileOutput = open(depFile,"r")
        for line in depFileOutput.readlines():
            # handle
            targetMatch1 = targetPat1.match(line)
            targetMatch2 = targetPat2.match(line)
            dependMatch = dependPat.match(line)
            if targetMatch1:
                targetModule1 = targetMatch1.group(1)
                targetMatchFlag1 = 1
            if targetMatch2:
                targetModule2 = targetMatch2.group(1)
                targetMatchFlag2 = 1
            if dependMatch:
                dependModule = dependMatch.group(1)
                dep_files.append(dependModule)
        depFileOutput.close()
        for dep in dep_files:
            if targetMatchFlag1:
                 os.system("echo %s:%s >> %s" % (targetModule1,dep,targetTxt))
            if targetMatchFlag2:
                 os.system("echo %s:%s >> %s" % (targetModule2,dep,targetTxt))  

###############################################
#            begin to release
###############################################

def mtkRelease():
    """ mediatek custom release """
    # initialzation
    dirs = DirRelease()
    files = FileRelease()
    kernel = KernelRelease()
    android = AndroidRelease()
    framework = FrameworkRelease()
    apps = AppRelease()
    misc = MiscRelease()
    # release steps
    if flag_targetProject:
        dirs.release()
        files.release()
        kernel.release()
    android.release()
    apps.release()
    framework.release()
    misc.release()
    print >> sys.stdout,"custom release[done]!"

# end mtkRelease

### MTK RELEASE ENTRY ###
mtkRelease()
### MTK RELEASE ENTRY ###

# ---------------------------------------------#
#   give the -d and dump the dep information   #
# ---------------------------------------------#
if options.dump == True:
    dumpDep(binModules)
# ---------------------------------------------#
