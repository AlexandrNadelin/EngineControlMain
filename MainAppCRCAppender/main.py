import sys  # We need sys so that we can pass argv to QApplication
from MainForm import MainForm
from PyQt5.QtWidgets import (QApplication)
import HexFileAppender
import os.path

if sys.argv.__len__()==5:
    HexFileAppender.appentHexFile(sys.argv[1],sys.argv[2],sys.argv[3],sys.argv[4])
elif (sys.argv.__len__()==2)and (sys.argv[1]=='--pathsFromFile'):
    if os.path.exists('.property'):            
        fileProperties = open('.property','r')
        srcFilePath = fileProperties.readline().strip()
        destFilePath = fileProperties.readline().strip()
        mainProgramStartAddr = fileProperties.readline().strip()
        mainProgramSize = fileProperties.readline().strip()
        fileProperties.close()
        if os.path.exists(srcFilePath) and os.path.exists(destFilePath):
            HexFileAppender.appentHexFile(srcFilePath,destFilePath,mainProgramStartAddr,mainProgramSize)
else:    
    app = QApplication(sys.argv)
    w = MainForm()
    sys.exit(app.exec_())

