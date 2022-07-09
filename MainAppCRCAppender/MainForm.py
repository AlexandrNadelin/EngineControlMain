from ast import Not
from PyQt5 import QtGui, uic
from PyQt5.QtWidgets import (QMessageBox,QTextEdit, QWidget, QVBoxLayout, QHBoxLayout,QLabel, QApplication,QLineEdit,QFrame,QPushButton,QFileDialog,QGridLayout,QDesktopWidget)
from PyQt5.QtGui import QFont
import sys  # We need sys so that we can pass argv to QApplication
import os
import os.path
import HexFileAppender

class MainForm(QWidget):    

    def selectFolder(self):
        srcFilePath = ""
        destFilePath = ""   
        mainProgramStartAddr = 0x08020000
        mainProgramSize = 0xA0000
        if(os.path.exists('.property')):            
            fileProperties = open('.property','r')
            srcFilePath = fileProperties.readline().strip()
            destFilePath = fileProperties.readline().strip()
            mainProgramStartAddr = int(fileProperties.readline().strip(),16)
            mainProgramSize = int(fileProperties.readline().strip(),16)
            fileProperties.close()
        else:
            srcFilePath = os.path.abspath(os.curdir).replace('\\','/',20)+'/'
            destFilePath = os.path.abspath(os.curdir).replace('\\','/',20)+'/'
            fileProperties = open('.property','w+')
            fileProperties.write(srcFilePath + '\n')
            fileProperties.write(destFilePath + '\n')
            fileProperties.write('0x'+format(mainProgramStartAddr, 'X') + '\n')
            fileProperties.write('0x'+format(mainProgramSize, 'X') + '\n')
            fileProperties.close()

        srcFilePath = QFileDialog.getOpenFileName(self,"Open sorce .hex file", srcFilePath, "Files (*.hex)")[0]
        if(srcFilePath==''):
            return
        destFilePath = QFileDialog.getSaveFileName(self,"Open destination .hex file", destFilePath, "Files (*.hex)")[0] #, "Files (*.hex)"
        if(destFilePath==''):
            return

        HexFileAppender.appentHexFile(srcFilePath,destFilePath,'0x'+format(mainProgramStartAddr, 'X'),'0x'+format(mainProgramSize, 'X'))

        if(os.path.exists('.property')):            
            fileProperties = open('.property','w+')
            fileProperties.write(srcFilePath + '\n') #os.path.dirname(
            fileProperties.write(destFilePath + '\n') #os.path.dirname(
            fileProperties.write('0x'+format(mainProgramStartAddr, 'X') + '\n')
            fileProperties.write('0x'+format(mainProgramSize, 'X') + '\n')
            fileProperties.close()

        msgBox = QMessageBox()
        msgBox.setIcon(QMessageBox.Information)
        msgBox.setText("File Was Created")
        msgBox.setWindowTitle("Notice")
        msgBox.setStandardButtons(QMessageBox.Ok | QMessageBox.Cancel)
        returnValue = msgBox.exec()

    def __init__(self):
        super().__init__()

        self.setFont(QFont('Arial', 12))

        mainVLayout = QVBoxLayout()
        self.selectFolderBTN = QPushButton("Select files")
        self.selectFolderBTN.setMinimumSize(170,40)
        self.selectFolderBTN.clicked.connect(self.selectFolder)
        mainVLayout.addWidget(self.selectFolderBTN)

        self.setLayout(mainVLayout)

        self.setGeometry(0,0,400,200)
        self.setMinimumHeight(200)
        self.setWindowTitle('Vibo monitoring app')

        qtRectangle = self.frameGeometry()
        centerPoint = QDesktopWidget().availableGeometry().center()
        qtRectangle.moveCenter(centerPoint)
        self.move(qtRectangle.topLeft())

        self.show()


if __name__ == '__main__':
    app = QApplication(sys.argv)
    w = MainForm()
    sys.exit(app.exec_())