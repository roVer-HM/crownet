

import os,sys,tkinter, datetime
from tkinter.filedialog import askdirectory, askopenfilename



class Project:

    def __init__(self):
        self.dirname = None
        self.projectfilepath = None
        self.projectfilename = "sensitivity.project"
        self.logfilename = "data.log"

        self.statusPreprocessing = "none"
        self.statusSolving = "none"
        self.statusPostprocessing = "none"


    def setupProject(self):
        index, __ = GUI_CreateOpen().getStatus()
        if index == 0:
            self.createProject()
        if index == 1:
            self.openProject()


    def createProject(self):
        dirname =  GUI_ProjectDirectory().getProjectDirectory()
        timestemp = datetime.datetime.now().strftime("%Y-%m-%d_%H-%M")
        dirname = dirname + "/Sensitivity__" + timestemp
        os.mkdir( dirname )
        os.mkdir( dirname + "/basis_input_files" )

        projectfilepath = os.path.join(dirname, self.projectfilename)

        f = open(projectfilepath, "w+")
        f.write( f"Project created: {str(timestemp) }\nUse this file to identify your sensitivity project. \n")
        f.write("Preprocessing: none\n")
        f.write("Solving: none\n")
        f.write("Postprocessing: none\n")
        f.close()

        self.dirname = dirname
        self.projectfilepath = projectfilepath



    def openProject(self):

        filename = askopenfilename(title="Select project file", filetypes= [("Project file", self.projectfilename)] )

        if os.path.basename(filename) != self.projectfilename:
            raise Exception('Error with sensitivity project file')
        else:
            print("Located sensitivity project in " + filename )

        self.dirname = os.path.dirname(filename)
        self.projectfilepath = filename

        f = open(filename, "r")
        status = f.read().splitlines()

        for k in range(2,5):
            status[k] = status[k].split(": ")[1]

        self.statusPreprocessing = status[2]
        self.statusSolving = status[3]
        self.statusPostprocessing = status[4]

    def getDirectory(self):
        return self.dirname

    def getProjectStatus(self):
        return  self.statusPreprocessing, self.statusSolving, self.statusPostprocessing





class GUI_CreateOpen:

    status = "undefined"
    index = -1

    def __init__(self):
        self.__askcreateopen()

    def __askcreateopen(self):
        self.root = tkinter.Tk()
        self.root.withdraw()

        top = tkinter.Toplevel()
        top.title("About this application...")

        msg = tkinter.Message(top, text="Sensitivity analysis project ...")
        msg.pack()

        button = tkinter.Button(top, text="... create ", command = lambda : self.__setStatus(0))
        button.pack()

        button = tkinter.Button(top, text="... open", command = lambda : self.__setStatus(1))
        button.pack()

        self.root.mainloop()

    def __setStatus(self,value):
        if value == 0:
            status = "create project"
        elif value == 1:
            status = "open project"

        self.status = status
        self.index = value
        self.root.destroy()

    def getStatus(self):
        return self.index, self.status

class GUI_ProjectDirectory:

    dirname = ""

    def __init__(self):
        root = tkinter.Tk()
        root.withdraw()
        self.dirname = askdirectory(title='Please select a directory')
        tkinter.filedialog.Directory()
        root.destroy()

    def getProjectDirectory(self):
        return self.dirname

if __name__ == "__main__":

    Project().setupProject()