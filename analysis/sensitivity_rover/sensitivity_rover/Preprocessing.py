

import os,sys,tkinter, datetime, shutil
from tkinter.filedialog import askdirectory, askopenfilename
from tkinter import messagebox


class Project:

    def __init__(self):
        self.dirname = None
        self.projectfilepath = None
        self.projectfilename = "sensitivity.project"
        self.logfilename = "data.log"

        self.statusPreprocessing = "none"
        self.statusSampling = "none"
        self.statusSolving = "none"
        self.statusPostprocessing = "none"


    def setupProject(self):
        index, __ = GUI_CreateOpen().getStatus()
        if index == 0:
            self.createProject()
            self.copy_simulation()
        if index == 1:
            self.openProject()


    def createProject(self):

        Messagebox("Choose a directory for the new project.")

        dirname =  GUI_ProjectDirectory().getProjectDirectory()
        timestemp = datetime.datetime.now().strftime("%Y-%m-%d_%H-%M")
        dirname = dirname + "/Sensitivity__" + timestemp
        os.mkdir( dirname )
        projectfilepath = os.path.join(dirname, self.projectfilename)

        f = open(projectfilepath, "w+")
        f.write( f"Project created: {str(timestemp) }\nUse this file to identify your sensitivity project. \n")
        f.write("Preprocessing: none\n")
        f.write("Sampling: none\n")
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

        for k in range(2,6):
            status[k] = status[k].split(": ")[1]

        self.statusPreprocessing = status[2]
        self.statusSampling = status[3]
        self.statusSolving = status[4]
        self.statusPostprocessing = status[5]

    def getDirectory(self):
        return self.dirname

    def getProjectStatus(self):
        return  self.statusPreprocessing, self.statusSampling, self.statusSolving, self.statusPostprocessing

    def copy_simulation(self):

        Messagebox("Copy simulation from directory")
        dirname = GUI_ProjectDirectory().getProjectDirectory()
        input_files = os.path.join(self.dirname, "basis_simulation")
        print("Copy folder " + dirname )
        print("to " + input_files )
        shutil.copytree(dirname, input_files)

    def extract_variables(self):

        basis_simulation = os.path.join(self.dirname, "basis_simulation")
        simulation = os.path.join(self.dirname, "simulation")
        shutil.copytree(basis_simulation, simulation)

        Messagebox("Extract variables from vadere.scenario file.")
        input_file = os.path.join(self.dirname, "basis_simulation/input/vadere.scenario" )
        new_input_file = os.path.join(self.dirname, "simulation/input/vadere.scenario" )
        GUI_VariableExtraction(input_file = input_file, new_input_file= new_input_file )

        Messagebox("Extract variables from omnetpp.ini file.")
        input_file = os.path.join(self.dirname, "basis_simulation/input/omnetpp.ini")
        new_input_file = os.path.join(self.dirname, "simulation/input/omnetpp.ini")
        GUI_VariableExtraction(input_file=input_file, new_input_file=new_input_file)

    def extract_variables_update(self,  omnett_index , vadere_index ):

        basis_simulation = os.path.join(self.dirname, "basis_simulation")
        simulation = os.path.join(self.dirname, "simulation")
        shutil.rmtree(simulation, ignore_errors=True)
        shutil.copytree(basis_simulation, simulation)

        Messagebox("Extract variables from vadere.scenario file.")
        input_file = os.path.join(self.dirname, "basis_simulation/input/vadere.scenario" )
        new_input_file = os.path.join(self.dirname, "simulation/input/vadere.scenario" )
        GUI_VariableExtraction(input_file = input_file, new_input_file= new_input_file, preselection = vadere_index)

        Messagebox("Extract variables from omnetpp.ini file.")
        input_file = os.path.join(self.dirname, "basis_simulation/input/omnetpp.ini")
        new_input_file = os.path.join(self.dirname, "simulation/input/omnetpp.ini")
        GUI_VariableExtraction(input_file=input_file, new_input_file=new_input_file,  preselection = omnett_index)

    def update_simulation_parameters(self):
        diff_omnettpp, diff_vadere = self.check_Difference()

        if diff_omnettpp == 0 and diff_vadere == 0 :
            print("No difference found.")
        else:
            omnett_index , vadere_index = self.findVariableIndex()
            self.extract_variables_update(omnett_index, vadere_index)





    def findVariableIndex(self):

        omnetpp_find_strings, vadere_find_strings = self.findOldVariablesInNewFile()

        basis_omnettpp, basis_vadere = self.getBasisFiles()
        basis_omnettpp = basis_omnettpp.splitlines()
        basis_vadere = basis_vadere.splitlines()

        omnett_index = []
        counter=0
        for item in omnetpp_find_strings:
            try:
                ind = basis_omnettpp.index(item)
                omnett_index.append(ind)
            except:
                counter += 1


        if counter > 0:
            Messagebox(str(counter) + " variables could not be found in new omnetpp.ini")

        vadere_index = []
        counter = 0
        for item in vadere_find_strings:
            try:
                ind = basis_vadere.index(item)
                vadere_index.append(ind)
            except:
                counter += 1


        if counter > 0:
            Messagebox(str(counter) + " variables could not be found in new vadere.scenario")

        return omnett_index , vadere_index





    def findOldVariablesInNewFile(self):

        # get lines to be found
        __,__, index_omnetpp, index_vadere = self.get_default_values()

        omnetpp, vadere = self.createSample()
        omnetpp = omnetpp.splitlines()
        vadere = vadere.splitlines()

        omnetpp_find_strings = []
        vadere_find_strings = []

        for k in index_omnetpp:
            omnetpp_find_strings.append( omnetpp[k] )

        for k in index_vadere:
            vadere_find_strings.append( vadere[k]  )

        return omnetpp_find_strings, vadere_find_strings





    def check_Difference(self):

        sample_omnettpp, sample_vadere = self.createSample()
        basis_omnettpp, basis_vadere = self.getBasisFiles()

        if sample_omnettpp == basis_omnettpp:
            #print("It's the same.")
            diff_omnettpp = 0
        else:
           # print("Difference between *.ini files.")
            diff_omnettpp = 1

        if sample_vadere == basis_vadere:
           # print("It's the same.")
            diff_vadere = 0
        else:
           # print("Difference between *.scenario files.")
            diff_vadere = 1

        return diff_omnettpp, diff_vadere


    def getBasisFiles(self):

        file = os.path.join(self.dirname, "basis_simulation/input/omnetpp.ini")

        with open(file) as f:
            sample_omnettpp = f.read()

        file = os.path.join(self.dirname, "basis_simulation/input/vadere.scenario")
        with open(file) as f:
            sample_vadere = f.read()

        return sample_omnettpp, sample_vadere



    def createSample(self, default_omnetpp = None, default_vadere = None):

        if default_omnetpp is None or default_vadere is None:
            default_omnetpp, default_vadere, __, __ = self.get_default_values()

        file = os.path.join(self.dirname, "simulation/input/omnetpp.ini")
        with open(file) as f:
            sample_omnettpp = f.read()

        for k in range(len(default_omnetpp) ):
            sample_omnettpp = sample_omnettpp.replace( "VARIABLE" + str(k) , default_omnetpp[k] )

        file = os.path.join(self.dirname, "simulation/input/vadere.scenario")
        with open(file) as f:
            sample_vadere = f.read()

        for k in range(len(default_vadere)):
            sample_vadere = sample_vadere.replace("VARIABLE" + str(k), default_vadere[k])

        return sample_omnettpp, sample_vadere

    def createSampling(self):

        default_omnetpp, default_vadere, __, __ = self.get_default_values()

        master = tkinter.Tk()
        line = dict()

        for row in range(len(default_omnetpp)):
            tkinter.Label(master, text=("Omnett - Variable" +str(row+1) + " =(" + default_omnetpp[row] + ")" )).grid(row=row)

            variable[row] = tkinter.StringVar(master)
            variable[row].set("continous")  # default value

            w[row] = tkinter.OptionMenu(master, variable[row], "continous", "discrete").grid(row=row, column =1 )
            e = tkinter.Entry(master)
            e.grid(row=row, column=2)

        print("grgregre")



    def get_default_values(self):

        #print("Check *.ini file.")
        file = os.path.join(self.dirname, "simulation/input/omnetpp.ini_default")
        default_omnetpp, index_omnetpp = self.__read_default_variable_values(file)

        #print("Check *.scenario file.")
        file = os.path.join(self.dirname, "simulation/input/vadere.scenario_default")
        default_vadere, index_vadere = self.__read_default_variable_values(file)

        return default_omnetpp, default_vadere, index_omnetpp, index_vadere


    def __read_default_variable_values(self, file):

        with open(file) as f:
            values = f.read().splitlines()

        default_values = []
        index = []
        for k in range( 1, len(values)):
            default_values.append(  values[k].split("= ")[1] )
            index.append(  values[k].split("\t")[0].split("Index: ")[1] )

        index = list(map(int, index))

        return default_values, index



class GUI_VariableExtraction:

    def __init__(self, input_file = None, new_input_file = None, preselection = None):

        if input_file is None or new_input_file is None:
            root = tkinter.Tk()
            root.withdraw()
            Messagebox("Choose an input file from which you want to extract variables.")
            FileName = askopenfilename(title="Open input file", filetypes=[("input file", "*.scenario"), ("input file", "*.ini")])
        else:
            FileName = input_file

        self.input_file = FileName
        self.preselection = preselection
        self.__start_gui(new_input_file)




    def __start_gui(self, new_input_file):

        self.oldSelection = None

        self.root = tkinter.Tk()
        self.root.title("Create variable extraction file")

        topframe = tkinter.Frame(self.root)
        topframe.pack()

        w = tkinter.Label(topframe, text="\n\n Basis input file: " + os.path.basename(self.input_file), width=100, anchor="w")
        w.pack(side="left", pady=0, )

        w = tkinter.Label(topframe, text=" ", width=10, height=5)
        w.pack(side="left", pady=0)


        greenbutton = tkinter.Button(topframe, text="Save", command= lambda: self.extract_data(new_input_file) , width=40, )
        greenbutton.pack(side="right")

        frame = tkinter.Frame(self.root)
        frame.pack()

        with open(self.input_file) as f:
            self.input_text = f.read().split("\n")

        self.listbox = tkinter.Listbox(frame, selectmode="multiple", height=70, width=100, selectbackground="green yellow")
        self.listbox.pack(side="left", expand=True, fill="both")

        for i in range(len(self.input_text)):
            self.listbox.insert(i, self.input_text[i])

        scrollbar = tkinter.Scrollbar(frame)
        scrollbar.pack(side="left", fill="y")
        self.listbox.config(yscrollcommand=scrollbar.set)
        scrollbar.config(command=self.listbox.yview)

        self.listbox.bind('<<ListboxSelect>>', self.onselect )

        self.choice = tkinter.Text(frame, height=70, width=100, )
        self.choice.pack(side="left", padx=20)

        self.extraction = ""

        if self.preselection is not None:
            self.oldSelection = self.preselection
            self.__update_selection(self.preselection)

        self.root.mainloop()


    def extract_data(self, new_input_file = None ):

        root = tkinter.Tk()
        root.withdraw()
        save_file = messagebox.askokcancel("Save variable extraction", "Save variable selection in file?")
        root.destroy()

        if save_file is True:
            if new_input_file is None:
                file = os.path.splitext(self.input_file)
                pathnewfile0 = file[0] + "2" + file[1]
            else:
                pathnewfile0 = new_input_file

            textwithVariables = self.listbox.get(0, "end")

            f = open(pathnewfile0, "w+")

            for item in textwithVariables[:-1]:
                f.write("%s\n" % item)
            f.write(textwithVariables[-1])
            f.close()


            pathnewfile =  pathnewfile0 + "_default"
            f = open(pathnewfile, "w+")
            f.write("Default values for VARIABLEx in file " + os.path.basename(pathnewfile0) + "\n")
            f.write(self.extraction)
            f.close()

            Messagebox("Exported variable selection to \n" + os.path.basename(pathnewfile))
            self.root.withdraw()
            self.root.quit()

    def onselect(self,event):
        w = event.widget
        selection = w.curselection()
        self.__update_selection(selection)

    def __update_selection(self,selection):

        if self.oldSelection is None:
            difference = None
        elif len(self.oldSelection) > len(selection):
            difference = list(set(self.oldSelection) ^ set(selection))[0]
            self.listbox.delete(difference)
            self.listbox.insert(difference, self.input_text[difference])
        self.oldSelection = selection

        self.choice.delete(1.0, "end")
        self.extraction = ""
        counter = 0
        for k in range(len(selection)):
            index = selection[k]
            mystr = self.input_text[index].replace(":", "=")
            mystr = mystr.replace(",", "")
            mystr = mystr.replace("#", "=")
            mystr = mystr.split(" = ")

            if len(mystr) >= 2 and (mystr[1].find("{") == -1):
                variableX = "VARIABLE" + str(counter)
                printout = self.input_text[index].replace(mystr[1], variableX)
                self.listbox.delete(index)
                self.listbox.insert(index, printout)
                self.listbox.selection_set(index)

                mystr2 = "Index: " + str(index) + " \t " + variableX + " = " + mystr[1] + "\n"
                self.choice.insert("end", mystr2)
                self.extraction = self.extraction + mystr2
            else:
                self.listbox.selection_clear(index)
                self.listbox.itemconfig(index, background="red")
            counter = counter + 1


class Messagebox():
    def __init__(self, title = "Message"):
        self.root = tkinter.Tk()
        self.root.withdraw()
        messagebox.showinfo("Create project", title)



class GUI_CreateOpen:

    status = "undefined"
    index = -1

    def __init__(self):
        self.__askcreateopen()

    def __askcreateopen(self):
        self.root = tkinter.Tk()
        self.root.withdraw()

        top = tkinter.Toplevel()
        top.title("Sensitivity analysis - Prep. ")
        top.geometry("300x130")

        msg = tkinter.Message(top, text="Project \n")
        msg.pack()

        button = tkinter.Button(top, text="Create ", command = lambda : self.__setStatus(0), width = 60)
        button.pack()

        button = tkinter.Button(top, text="Open", command = lambda : self.__setStatus(1), width = 60)
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

    def __init__(self, title = 'Please select a directory'):
        root = tkinter.Tk()
        root.withdraw()
        self.dirname = askdirectory(title=title)
        tkinter.filedialog.Directory()
        root.destroy()

    def getProjectDirectory(self):
        return self.dirname




if __name__ == "__main__":
    Project().setupProject()
