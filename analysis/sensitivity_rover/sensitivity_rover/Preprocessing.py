
import os ,sys ,tkinter, datetime, shutil, re, time, subprocess
from tkinter.filedialog import askdirectory, askopenfilename
from tkinter import messagebox
from pyDOE import *
import matplotlib.pyplot as plt




class Status:

    def __init__(self, projectfilepath):

        self.projectfilepath = projectfilepath

        self.Simulation = "none"
        self.Parameter = "none"
        self.Sampling = "none"
        self.Solving = "none"
        self.Postprocessing = "none"

    def read_status(self):

        f = open(self.projectfilepath, "r")
        status = f.read().splitlines()

        for k in range(2, len(status)):
            status[k] = status[k].split(": ")[1]

        self.Simulation = status[2]
        self.Parameter = status[3]
        self.Sampling = status[4]
        self.Solving = status[5]
        self.Postprocessing = status[6]

    def write_status(self, information = None):

        loc = "Original simulation was copied from: "

        if os.path.exists(self.projectfilepath):
            f = open(self.projectfilepath, "r+")
            header = f.read().splitlines()
            location = header[1]
        else:
            location = loc + "not set yet"

        f = open(self.projectfilepath, "w+")

        f.write("This is a sensitivity project file used for parameter studies. Use this file to identify your sensitivity project. \n")

        if information is None:
            f.write(location)
        else:
            f.write(loc + information)

        f.write("\n")


        f.write(f"Simulation: {self.Simulation}\n")
        f.write(f"Parameter: {self.Parameter}\n")
        f.write(f"Sampling: {self.Sampling}\n")
        f.write(f"Solving: {self.Solving}\n")
        f.write(f"Postprocessing: {self.Postprocessing}\n")
        f.close()



class Project:


    def __init__(self):
        self.dirname = None
        self.projectfilepath = None
        self.projectfilename = "sensitivity.project"
        self.logfilename = "data.log"


    def setupProject(self):
        index, __ = GUI_CreateOpen().getStatus()
        if index == 0:
            self.createProject()
            self.copy_root_simulation()

        if index == 1:
            self.openProject()


    def createProject(self):

        Messagebox("Choose a directory for the new project.")

        dirname =  GUI_ProjectDirectory().getProjectDirectory()
        timestemp = datetime.datetime.now().strftime("%Y-%m-%d_%H-%M")
        dirname = dirname + "/Sensitivity__" + timestemp
        os.mkdir( dirname )
        projectfilepath = os.path.join(dirname, self.projectfilename)

        self.status = Status(projectfilepath)
        self.status.write_status()

        self.dirname = dirname
        self.projectfilepath = projectfilepath






    def isempty_Project(self):
        basis_folder = os.path.join(self.dirname, "basis_simulation")
        sim_folder = os.path.join(self.dirname, "simulation")

        if os.path.isdir(basis_folder) == False:
            return_code = 0
        else:
            return_code = 1
            self.__find_input_files()
        return return_code






    def openProject(self):

        filename = askopenfilename(title="Select project file", filetypes= [("Project file", self.projectfilename)] )

        if os.path.basename(filename) != self.projectfilename:
            raise Exception('Error with sensitivity project file')
        else:
            print("Located sensitivity project in " + filename )

        self.status = Status(filename)
        self.status.read_status()

        self.dirname = os.path.dirname(filename)
        self.projectfilepath = filename

        self.getProjectStatus()



    def getDirectory(self):
        return self.dirname

    def getProjectStatus(self):
        self.status.read_status()
        return self.status


    def prepare_solver(self):

        self.copy_root_simulation()
        self.status.Simulation = "yes"
        self.status.write_status()


    def copy_root_simulation(self):

        Messagebox("Copy simulation from directory")
        dirname = GUI_ProjectDirectory().getProjectDirectory()
        self.simulation_name = os.path.basename(dirname)
        information = dirname.split("rover-main/")[1]
        information = os.path.join("rover-main", information)
        self.status.write_status(information)

        input_files = os.path.join(self.dirname, "basis_simulation")

        print("Copy folder " + dirname )
        print("to " + input_files )

        if os.path.exists(input_files):
            shutil.rmtree(input_files)

        shutil.copytree(dirname, input_files, ignore=shutil.ignore_patterns('*.out','output') )

    def copy_simulation(self, folder_name ):

        dirname = os.path.join(self.dirname, "simulation")
        input_files = os.path.join(self.dirname, folder_name)

        if os.path.exists(input_files):
            shutil.rmtree(input_files)

        shutil.copytree(dirname, input_files, ignore=shutil.ignore_patterns('*_default'))


    def  __get_corresponding_scenario_file_from_ini(self,ini_file):

        dirname = os.path.join(self.dirname,"basis_simulation")
        for root, dirs, files in os.walk(dirname):
            for file in files:
                if file.endswith(".sh"):
                    sh_file = os.path.join(root, file)

        with open(sh_file) as f:
            lines = f.read().splitlines()

        for line in lines:
            if "CONFIG" in line:
                config = line.split('"')[1]
                config = "[Config " + config + "]"
                break

        with open(ini_file) as f:
            lines = f.read().splitlines()

        right_config = False

        for k in range(len(lines)):
            if config in lines[k]:
                right_config = True

            if right_config == True:
                if "*.manager.vadereScenarioPath" in lines[k]:
                    correct_file = lines[k].split('"')[1]
                    break

        return os.path.join(dirname,correct_file)



    def __find_input_files(self):

        basis_simulation = os.path.join(self.dirname,"basis_simulation")

        for root, dirs, files in os.walk(basis_simulation):
            for file in files:
                if file.endswith(".ini"):
                    ini_file = os.path.join(root, file)


        scenario_files = list()

        for root, dirs, files in os.walk(basis_simulation):
            for file in files:
                if file.endswith(".scenario"):
                    scenario_files.append(os.path.join(root, file))

        if len(scenario_files)>1:
            scenario_file = self.__get_corresponding_scenario_file_from_ini(ini_file)
        else:
            scenario_file = scenario_files[0]

        self.scenario_file = scenario_file
        self.ini_file = ini_file








    def extract_variables(self):

        self.__find_input_files()

        basis_simulation = os.path.join(self.dirname, "basis_simulation")
        simulation = os.path.join(self.dirname, "simulation")
        shutil.copytree(basis_simulation, simulation)

        Messagebox("Extract variables from vadere.scenario file.")
        input_file = self.scenario_file
        new_input_file = self.__get_ini_files_in_simulation("simulation","scenario" )
        GUI_VariableExtraction(input_file = input_file, new_input_file= new_input_file )

        Messagebox("Extract variables from omnetpp.ini file.")
        input_file = self.ini_file
        new_input_file = self.__get_ini_files_in_simulation("simulation","ini" )
        GUI_VariableExtraction(input_file=input_file, new_input_file=new_input_file)

        self.status.Parameter = "yes"
        self.status.write_status()

    def extract_variables_update(self,  omnett_index , vadere_index ):

        basis_simulation = os.path.join(self.dirname, "basis_simulation")
        simulation = os.path.join(self.dirname, "simulation")
        shutil.rmtree(simulation, ignore_errors=True)
        shutil.copytree(basis_simulation, simulation)

        Messagebox("Extract variables from vadere.scenario file.")
        input_file = self.scenario_file
        new_input_file = self.__get_ini_files_in_simulation("simulation", "scenario")
        GUI_VariableExtraction(input_file = input_file, new_input_file= new_input_file, preselection = vadere_index)

        Messagebox("Extract variables from omnetpp.ini file.")
        input_file = self.ini_file
        new_input_file = self.__get_ini_files_in_simulation("simulation", "ini")
        GUI_VariableExtraction(input_file=input_file, new_input_file=new_input_file,  preselection = omnett_index)

        self.status.Parameter = "yes"
        self.status.write_status()

    def update_simulation_parameters(self):
        diff_omnettpp, diff_vadere = self.check_Difference()

        if diff_omnettpp == 0 and diff_vadere == 0 :
            print("No difference found.")
        else:

            root = tkinter.Tk()
            root.withdraw()
            update_file = messagebox.askokcancel("Project update possible", "Changes in input files detected. Update project?")
            root.destroy()

            if update_file is True:

                root = tkinter.Tk()
                root.withdraw()
                update_check = messagebox.askokcancel("Project update",
                                                     "All simulation results are deleted when updating. Continue?")
                root.destroy()

                if update_check is True:
                    omnett_index , vadere_index = self.findVariableIndex()
                    self.extract_variables_update(omnett_index, vadere_index)





    def findVariableIndex(self):

        omnetpp_find_strings, vadere_find_strings = self.findOldVariablesInNewFile()

        basis_omnettpp, basis_vadere = self.getBasisFiles()
        basis_omnettpp = basis_omnettpp.splitlines()
        basis_vadere = basis_vadere.splitlines()

        omnett_index = []
        counter =0
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
        default_omnetpp, default_vadere, index_omnetpp, index_vadere = self.get_default_values()

        omnetpp, vadere = self.createSample(default_omnetpp, default_vadere)
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

        default_omnetpp, default_vadere, index_omnetpp, index_vadere, __, __ = self.get_default_values()

        sample_omnettpp, sample_vadere = self.createSample(default_omnetpp, default_vadere)
        basis_omnettpp, basis_vadere = self.getBasisFiles()

        if sample_omnettpp == basis_omnettpp:
            # print("It's the same.")
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

        file = self.ini_file

        with open(file) as f:
            sample_omnettpp = f.read()

        file = self.scenario_file
        with open(file) as f:
            sample_vadere = f.read()

        return sample_omnettpp, sample_vadere



    def createSample(self, default_omnetpp, default_vadere, folder = "simulation" ):

       # file = os.path.join(self.dirname, folder + "/input/omnetpp.ini")
        file = self.__get_ini_files_in_simulation("simulation","ini")
        with open(file) as f:
            sample_omnettpp = f.read()

        for k in range(len(default_omnetpp) ):
            sample_omnettpp = sample_omnettpp.replace( "VARIABLE" + str(k) , default_omnetpp[k] )

        #file = os.path.join(self.dirname, folder + "/input/vadere.scenario")
        file = self.__get_ini_files_in_simulation("simulation", "scenario")
        with open(file) as f:
            sample_vadere = f.read()

        for k in range(len(default_vadere)):
            sample_vadere = sample_vadere.replace("VARIABLE" + str(k), default_vadere[k])

        return sample_omnettpp, sample_vadere

    def createSampling(self):

        default_omnetpp, default_vadere, __, __, var_omnetpp, var_vadere = self.get_default_values()
        file_sampling = os.path.join( self.dirname , "sampling.dat" )

        sampling = Sampling(default_omnetpp, default_vadere, var_omnetpp, var_vadere ,file_sampling  )

        if os.path.exists(file_sampling):
            sampling.load()
        else:
            sampling.create()

        state_omnett, ranges_omnett, state_vadere, ranges_vadere = sampling.get_parameter_ranges()

        self.createParametercombinations(state_omnett, ranges_omnett, state_vadere, ranges_vadere)

        self.status.Sampling = "yes"
        self.status.write_status()

    def __read_sampleNrs_Method(self):

        filename = os.path.join(self.dirname, "samples.dat")
        f = open(filename, "r+")
        text = f.read().splitlines()
        method = text[0].split(",")[0]
        samples = text[0].split(",")[1]
        samples = samples.split(" number of samples: ")[1]

        return method, int(samples)

    def createParametercombinations(self, state_omnett, ranges_omnett, state_vadere, ranges_vadere):

        samplingMethod = SamplingMethod(self.dirname)
        samplingMethod.create()
        method, samples = samplingMethod.read()
        samples = int(samples)


        if method == "Latin Hypercube":
            l  = lhs( len(state_omnett) + len(state_vadere) , samples )

        ranges_omnett.extend(ranges_vadere)
        ranges = ranges_omnett

        scaled_lhs = []

        for item in l:
            scaled_lhs.append( self.scale_to_parameter_space(item, ranges) )

        print("----------")

        filename = os.path.join( self.dirname, "samples.dat")
        f = open(filename, "w+")
        f.write(method + ", number of samples: " + str(samples) + "\n")



        k = 0
        for item in state_omnett:
            f.write( "Omnet" + str(k) + "," )
            k += 1

        k = 0
        for item in state_vadere:
            f.write("Vadere" + str(k) + ",")
            k += 1
        f.write("\n")

        for item in scaled_lhs:
            f.write("%s\n" % item)
        f.close()


    def runSimulation(self):
        samples_omnet, samples_vadere =  self.readParametercombinations()
        self.prepare_runs(samples_omnet, samples_vadere)
        self.run_runs( len(samples_omnet))


    def run_runs(self, number):

        # os.system("TRACI_GUI=false vadere")

        if print(os.getenv('ROVER_MAIN')) is None:
            # print("Please add variable ROVER_MAIN to your e.g. /etc/environment")

            # print("Alternative: add ROVER_MAIN with python")
            os.environ['ROVER_MAIN'] = '/home/christina/repos/rover-main'

        # print(os.getenv('ROVER_MAIN'))

        for k in range(number):

            run_id = "run__" + str(k)
            os.chdir(os.path.join(self.dirname, run_id))
            os.system('echo ""')
            os.system('echo "------------------- started -----------------------"')
            os.system("chmod +x startsim.sh")
            process = subprocess.Popen(["./startsim.sh"], env=os.environ)
            process.wait()
            os.chdir('..')
            os.system('echo "------------------ finished -----------------------"')
            os.system('echo ""')

        self.status.Solving = "yes"
        self.status.write_status()






    def prepare_runs(self ,samples_omnet, samples_vadere):

        for k in range( len(samples_omnet) ):

            folder_name = "run__" + str(k)

            self.copy_simulation(folder_name)
            sample_om, sample_va = self.createSample(samples_omnet[k], samples_vadere[k])

            vadere = self.__get_ini_files_in_simulation(folder_name,"scenario")
            omnet = self.__get_ini_files_in_simulation(folder_name,"ini")

            with open(omnet ,"w") as f:
                values = f.write(sample_om)

            with open(vadere ,"w") as f:
                values = f.write(sample_va)















    def readParametercombinations(self):
        filename = os.path.join(self.dirname, "samples.dat")

        with open(filename) as f:
            values = f.read().splitlines()

        variables = values[1].split(",")

        a = list()
        for var in variables:
            if "Omnet" in var:
                a.append(0)

            if "Vadere" in var:
                a.append(1)

        aa = a.index(1)


        samples_omnet = list()
        samples_vadere = list()

        for line in values[2:]:
            line = line.replace("[", "")
            line = line.replace("]", "")
            line = line.split(", ")

            samples_omnet.append(line[:aa])
            samples_vadere.append(line[aa:])

        print(samples_omnet)
        print(samples_vadere)



        return samples_omnet, samples_vadere



    def scale_to_parameter_space(self, sample, ranges):


        scaled_sample = []
        for k in range(len(sample)):
            range_s = ranges[k]
            value = range_s[0] + ( range_s[1] - range_s[0] ) * sample[k]
            scaled_sample.append( value )

        print(scaled_sample)
        return scaled_sample


    def get_default_values(self):

        scenario_path = self.__get_ini_files_in_simulation("simulation","scenario")
        ini_path = self.__get_ini_files_in_simulation("simulation", "ini")

        # print("Check *.ini file.")
        file = ini_path + "_default"
        default_omnetpp, index_omnetpp, var_omnetpp = self.__read_default_variable_values(file)

        # print("Check *.scenario file.")
        file = scenario_path + "_default"
        default_vadere, index_vadere, var_vadere = self.__read_default_variable_values(file)

        return default_omnetpp, default_vadere, index_omnetpp, index_vadere, var_omnetpp, var_vadere


    def __get_ini_files_in_simulation(self,folder, input_file_type):

        if "ini" in input_file_type:
            filepath = self.ini_file
        else:
            filepath = self.scenario_file

        relative_path = filepath.split("basis_simulation/")[1]
        newpath = os.path.join(self.dirname, folder)
        return os.path.join(newpath, relative_path)


    def __read_default_variable_values(self, file):

        with open(file) as f:
            values = f.read().splitlines()

        default_values = []
        index = []
        vars = []
        for k in range( 1, len(values)):
            val = values[k].split("= ")[1]
            val = val.split("\t")
            var = val[1].replace("(","")
            var = var.replace(")", "")
            var = var.replace(" ", "")
            vars.append(var)
            default_values.append( val[0] )
            index.append(  values[k].split("\t")[0].split("Index: ")[1] )

        index = list(map(int, index))

        return default_values, index, vars



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

    def onselect(self ,event):
        w = event.widget
        selection = w.curselection()
        self.__update_selection(selection)

    def __update_selection(self ,selection):

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

                mystr2 = "Index: " + str(index) + " \t " + variableX + " = " + mystr[1]
                mystr2 = mystr2 + "\t ("+ mystr[0].replace(" ","") +")\n"
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

    def __setStatus(self ,value):
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

class Sampling:

    def __init__(self, default_omnetpp, default_vadere, var_omnetpp, var_vadere , projefilepath ):

        self.projectfilepath = projefilepath
        self.default_omnetpp = default_omnetpp
        self.default_vadere = default_vadere
        self.var_omnetpp = var_omnetpp
        self.var_vadere = var_vadere

    def load(self):

        state_omnett, ranges_omnett, state_vadere, ranges_vadere  = self.get_parameter_ranges()
        self.create(state_omnett, ranges_omnett, state_vadere, ranges_vadere)


    def get_parameter_ranges(self):

        with open(self.projectfilepath) as f:
            text = f.read()
        text = text.splitlines()

        state_omnett = []
        ranges_omnett = []

        state_vadere = []
        ranges_vadere = []

        p = re.compile(r'(\d+(?:\.\d+)?)')  # Compile a pattern to capture float values

        for line in text:
            line = line.split(" : ")
            simulator = line[0]
            line = line[1].split("\t")
            floats = [float(i) for i in p.findall(line[2])]  # Convert strings to float

            if simulator == "Omnett":
                state_omnett.append( line[1] )
                ranges_omnett.append(floats)

            if simulator == "Vadere":
                state_vadere.append( line[1] )
                ranges_vadere.append(floats)


        return state_omnett, ranges_omnett, state_vadere, ranges_vadere



    def create(self, state_omnett = None ,ranges_omnett=None, state_vadere=None, ranges_vadere=None):

        self.master = tkinter.Tk()
        self.master.title("Parameter ranges")
        self.labels_omnetpp = []
        self.menus_omnetpp = []
        self.entries_omnetpp = []
        self.vars_omnetpp = []

        label = tkinter.Label(self.master, text="Omnet", ).grid(sticky="w", row=0, column=0)

        for row in range(len(self.default_omnetpp)):


            label = tkinter.Label(self.master, text=("VARIABLE" + str(row)  ),width=20).grid(sticky="w",row=row,column=1)
            label = tkinter.Label(self.master, text=( self.var_omnetpp[row] ),  padx = 15 ).grid(sticky="w", row=row, column=2)
            label = tkinter.Label(self.master, text=( "default = " + self.default_omnetpp[row] )).grid(sticky="w",row=row,column=3)


            self.labels_omnetpp.append(label)
            var = tkinter.StringVar(self.master)

            if state_omnett is None:
                var.set("continous")  # default value
            else:
                var.set(state_omnett[row])

            self.vars_omnetpp.append(var)
            menu = tkinter.OptionMenu(self.master, self.vars_omnetpp[row], "continous", "discrete").grid(row=row,
                                                                                                         column=4)
            self.menus_omnetpp.append(menu)
            entry = tkinter.Entry(self.master)
            if ranges_omnett is not None:
                entry.insert("end", str(ranges_omnett[row]))
            entry.grid(row=row, column=5)
            self.entries_omnetpp.append(entry)

        self.labels_vadere = []
        self.menus_vadere = []
        self.entries_vadere = []
        self.vars_vadere = []

        label = tkinter.Label(self.master, text="Vadere").grid(sticky="w", row=len(self.default_omnetpp), column=0)

        for k in range(len(self.default_vadere)):
            row = k + len(self.default_omnetpp)

            label = tkinter.Label(self.master,text=("VARIABLE" + str(k)  ),width=20).grid(sticky="w",row=row,column=1)
            label = tkinter.Label(self.master, text=( self.var_vadere[k]), padx = 15 ).grid(sticky="w", row=row, column=2)
            label = tkinter.Label(self.master, text=("default = " + self.default_vadere[k] )).grid(sticky="w", row=row, column=3)


            self.labels_vadere.append(label)
            var = tkinter.StringVar(self.master)

            if state_vadere is None:
                var.set("continous")  # default value
            else:
                var.set(state_vadere[k])

            self.vars_vadere.append(var)
            menu = tkinter.OptionMenu(self.master, self.vars_vadere[k], "continous", "discrete").grid(row=row, column=4)
            self.menus_vadere.append(menu)
            entry = tkinter.Entry(self.master)
            if ranges_vadere is not None:
                entry.insert("end", str(ranges_vadere[k]))
            entry.grid(row=row, column=5)
            self.entries_vadere.append(entry)

        greenbutton = tkinter.Button(self.master, text="Save", command=lambda: self.__write_sampling_file(),
                                     width=20, ).grid(row=len(self.default_omnetpp) + len(self.default_vadere),
                                                     column=5)

        self.master.mainloop()

    def __write_sampling_file(self):

        write = ""

        for k in range(len(self.vars_omnetpp)):
            printout = "Omnett : Variable" + str(k) + "\t" + self.vars_omnetpp[k].get() + "\t" + self.entries_omnetpp[
                k].get() + "\n"
            write = write + printout

        for k in range(len(self.vars_vadere)):
            printout = "Vadere : Variable" + str(k) + "\t" + self.vars_vadere[k].get() + "\t" + self.entries_vadere[
                k].get() + "\n"
            write = write + printout


        f = open(self.projectfilepath, "w+")
        f.write(write)
        f.close()

        self.master.quit()
        self.master.destroy()


class SamplingMethod():


    def __init__(self, dirname):
        self.dirname = dirname

    def read(self):

        filename = os.path.join(self.dirname, "samples.dat")
        f = open(filename, "r+")
        text = f.read().splitlines()
        method = text[0].split(",")[0]
        numbers = text[0].split(",")[1]
        numbers = numbers.split(" number of samples: ")[1]
        return numbers, method




    def create(self):

        if os.path.exists(os.path.join(self.dirname, "samples.dat")) == True:
            samples, method = self.read()
        else:
            samples = None
            method = None

        self.master = tkinter.Tk()
        self.master.title("Sampling method")

        label = tkinter.Label(self.master, text = "Number of samples").grid(sticky="w", row=0, column=0)
        self.entry = tkinter.Entry(self.master)
        if samples is not None:
            self.entry.insert("end", str(samples))
        self.entry.grid(row=0, column=1)


        self.method = tkinter.StringVar(self.master)
        if method is not None:
            self.method.set(method)
        label = tkinter.Label(self.master, text="Sampling method").grid(sticky="w", row=1, column=0)
        menu = tkinter.OptionMenu(self.master, self.method, "Latin Hypercube", "other").grid(row=1, column=1)


        greenbutton = tkinter.Button(self.master, text="Save", command=lambda: self.getMethod() ).grid(sticky="w", row=2, column=1)

    def getMethod(self):
        method = self.method.get()
        samples = self.entry.get()

        filename = os.path.join(self.dirname, "samples.dat")
        f = open(filename, "w+")
        f.write(method + ", number of samples: " + samples + "\n" )
        f.close()

        self.master.quit()
        self.master.destroy()






if __name__ == "__main__":
    Project().setupProject()
