from tkinter import *
import sensitivity_rover.Preprocessing as prep


class SensitivityAnalysisGui:
    preprocess = prep.Project()

    status = "undefined"

    def __init__(self, root):

        self.root = root
        self.root.title("Sensitivity analsis helper")
        self.root.geometry("1000x300")

        menu = Menu(self.root)
        self.root.config(menu=menu)

        filemenu = Menu(menu, tearoff=0)
        menu.add_cascade(label="Project", menu=filemenu)
        filemenu.add_command(label="New", command=lambda: self.__create())
        filemenu.add_command(label="Open", command=lambda: self.__open())
        filemenu.add_command(label="Close", command=self.root.destroy)

        helpmenu = Menu(menu)
        menu.add_cascade(label="Help", menu=helpmenu)
        helpmenu.add_command(label="About", )


    def __create(self):
        self.preprocess.createProject()
        directory = self.preprocess.getDirectory()
        directory = "Created project in: \t" + directory
        label = Label(self.root, height=5, width=100, text=directory)
        label.pack()
        self.__show("create")

    def __open(self):
        self.preprocess.openProject()
        directory = self.preprocess.getDirectory()
        directory = "Opened project: \t" + directory
        label = Label(self.root, height=5, width=120, text=directory)
        label.pack()
        self.__show("open")

    def __show(self, state):
        status = self.preprocess.getProjectStatus()
        isempty_Project = self.preprocess.isempty_Project()


        if status.Sampling == "none":
            color_Sampling = "red"
        else:
            color_Sampling = "green"

        if status.Solving == "none":
            color_Solving = "red"
        else:
            color_Solving = "green"

        if status.Postprocessing == "none":
            color_PostProcessing = "red"
        else:
            color_PostProcessing = "green"

        if state == "create" or isempty_Project == 0:

            if status.Simulation == "none":
                color_Simulation = "red"
            else:
                color_Simulation = "green"

            if status.Parameter == "none":
                color_Parameter = "red"
            else:
                color_Parameter = "green"

            self.button_prep = Button(self.root, text="Define simulation", relief=RIDGE, width=100, bg = color_Simulation,
                                               command=lambda: self.__prepareSolverNode())

            self.button_para = Button(self.root, text="Define parameters", relief=RIDGE, width=100, bg=color_Sampling, command=lambda: self.__define_parameters())

            self.button_prep.pack()
            self.button_para.pack()

        elif state == "open":


            color_Update = "red"

            self.button_update = Button(self.root, text="Update", relief=RIDGE, width=100, bg= color_Update,
                                      command=lambda: self.__update_preprocessing())
            self.button_update.pack()



        self.button_samp = Button(self.root, text="Define sampling", relief=RIDGE, width=100, bg=color_Sampling, state = "disabled",
                                  command=lambda: self.__prepareSampling())

        self.button_run = Button(self.root, text="Run simulations", relief=RIDGE, width=100, bg = color_Solving,
                              command=lambda: self.__runSimulations(), state = "disabled")

        self.button_post = Button(self.root, text="Postprocessing", relief=RIDGE, width=100, bg = color_PostProcessing,
                              command=lambda: self.__runPostProcessing(), state = "disabled" )


        self.button_samp.pack()
        self.button_run.pack()
        self.button_post.pack()

    def __update_preprocessing(self):
        print("Update simulation and parameters")
        self.preprocess.update_simulation_parameters()

    def __prepareSolverNode(self):
        print("Define simulation")
        self.preprocess.copy_simulation()

    def __define_parameters(self):
        print("Define parameters")
        self.preprocess.extract_variables()

    def __prepareSampling(self):
        print("Define sampling")

        self.preprocess.creaFteSampling()

        print(dir)


    def __runSimulations(self):
        print("Run simulations")

    def __runPostProcessing(self):
        print("Start postprocessing")

def run_SensitivityAnalysisGui():
    root = Tk()
    app = SensitivityAnalysisGui(root)
    root.mainloop()

if __name__ == '__main__':
    run_SensitivityAnalysisGui()