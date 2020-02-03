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
        statusPreprocessing, statusSampling, statusSolving, statusPostprocessing = self.preprocess.getProjectStatus()


        if statusPreprocessing == "none":
            color_prep = "red"
        else:
            color_prep = "green"

        statusParameter = "none"

        if statusParameter == "none":
            color_para = "red"
            status_para = "normal"
        else:
            color_para = "green"
            status_para = "normal"

        if statusSampling == "none":
            color_samp = "red"
            status_samp = "normal"
        else:
            color_samp = "green"
            status_samp = "normal"

        if statusSolving == "none":
            color_run = "red"
            status_run = "normal"
        else:
            color_run = "green"
            status_run = "normal"

        if statusPostprocessing == "none":
            color_post = "red"
            status_post = "disabled"
        else:
            color_post = "green"
            status_post = "normal"

        if state == "create":
            self.button_prep = Button(self.root, text="Define simulation", relief=RIDGE, width=100, bg = color_prep,
                                               command=lambda: self.__prepareSolverNode())

            self.button_para = Button(self.root, text="Define parameters", relief=RIDGE, width=100, bg=color_samp,
                                      state = status_para, command=lambda: self.__define_parameters())

            self.button_prep.pack()
            self.button_para.pack()

        elif state == "open":
            self.button_update = Button(self.root, text="Update", relief=RIDGE, width=100, bg=color_prep,
                                      command=lambda: self.__update_preprocessing())
            self.button_update.pack()



        self.button_samp = Button(self.root, text="Define sampling", relief=RIDGE, width=100, bg=color_samp, state = status_samp,
                                  command=lambda: self.__prepareSampling())

        self.button_run = Button(self.root, text="Run simulations", relief=RIDGE, width=100, bg = color_run,
                              command=lambda: self.__runSimulations(), state = status_run)

        self.button_post = Button(self.root, text="Postprocessing", relief=RIDGE, width=100, bg = color_post,
                              command=lambda: self.__runPostProcessing(), state = status_post )


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

        self.preprocess.createSampling()

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