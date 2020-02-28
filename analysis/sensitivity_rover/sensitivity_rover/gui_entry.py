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
        self.__show()

    def __open(self):
        self.preprocess.openProject()
        directory = self.preprocess.getDirectory()
        directory = "Opened project: \t" + directory
        label = Label(self.root, height=5, width=120, text=directory)
        label.pack()

        isempty_Project = self.preprocess.isempty_Project()
        if isempty_Project == 1:
            self.preprocess.update_simulation_parameters()
        self.__show()


    def __show(self):

        self.button_Simulation = Button(self.root, text="Define simulation", relief=RIDGE, width=100,
                                            command=lambda: self.__prepareSolverNode())

        self.button_Parameter = Button(self.root, text="Define parameters", relief=RIDGE, width=100, command=lambda: self.__define_parameters())

        self.button_Sampling = Button(self.root, text="Define sampling", relief=RIDGE, width=100, state ="disabled",
                                      command=lambda: self.__prepareSampling())

        self.button_Solving = Button(self.root, text="Run simulations", relief=RIDGE, width=100,
                                     command=lambda: self.__runSimulations(), state = "disabled")

        self.button_PostProcessing = Button(self.root, text="Postprocessing", relief=RIDGE, width=100,
                                            command=lambda: self.__runPostProcessing(), state = "disabled")


        self.button_array = list()
        self.button_array.append(self.button_Simulation)
        self.button_array.append(self.button_Parameter)
        self.button_array.append(self.button_Sampling)
        self.button_array.append(self.button_Solving)
        self.button_array.append(self.button_PostProcessing)

        for button in self.button_array:
            button.pack()

        self.update_buttons()

    def update_buttons(self):
        self.__update_color_scheme()
        self.__enable_disable_buttons()


    def __update_preprocessing(self):
        print("Update simulation and parameters")
        self.preprocess.update_simulation_parameters()
        self.update_buttons()


    def __prepareSolverNode(self):
        print("Define simulation")
        self.preprocess.prepare_solver()
        self.update_buttons()


    def __define_parameters(self):
        print("Define parameters")
        self.preprocess.extract_variables()
        self.update_buttons()


    def __prepareSampling(self):
        print("Define sampling")

        self.preprocess.createSampling()
        self.update_buttons()


    def __runSimulations(self):

        print("Run simulations")
        self.preprocess.runSimulation()
        self.update_buttons()

    def __runPostProcessing(self):
        print("Start postprocessing")



    def __enable_disable_buttons(self):
        status = self.preprocess.getProjectStatus()

        activity = list()
        activity.append(status.Simulation)
        activity.append(status.Parameter)
        activity.append(status.Sampling)
        activity.append(status.Solving)
        activity.append(status.Postprocessing)

        index_none = activity.index("none")

        k = 0
        for button in self.button_array:
            if k <= index_none:
                button.config(state="normal")
            else:
                button.config(state="disabled")
            k += 1




    def __update_color_scheme(self):

        status = self.preprocess.getProjectStatus()

        if status.Simulation == "none":
            color_Simulation = "red"
        else:
            color_Simulation = "green"

        if status.Parameter == "none":
            color_Parameter = "red"
        else:
            color_Parameter = "green"

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



        self.button_Simulation.config(bg =  color_Simulation)
        self.button_Parameter.config(bg =  color_Parameter)
        self.button_Sampling.config(bg =  color_Sampling)
        self.button_Solving.config(bg =  color_Solving)
        self.button_PostProcessing.config(bg =  color_PostProcessing)




def run_SensitivityAnalysisGui():
    root = Tk()
    app = SensitivityAnalysisGui(root)
    root.mainloop()

if __name__ == '__main__':
    run_SensitivityAnalysisGui()