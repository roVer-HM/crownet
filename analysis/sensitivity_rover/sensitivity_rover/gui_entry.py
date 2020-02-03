from tkinter import *
import sensitivity_rover.Preprocessing as prep


class SensitivityAnalysisGui:
    preprocess = prep.Project()
    status = "undefined"

    def __init__(self):

        self.root = Tk()
        self.root.title("Sensitivity analsis helper")
        self.root.geometry("1000x300")

        self.menu = Menu(self.root)
        self.root.config(menu=self.menu)

        self.filemenu = Menu(self.menu, tearoff=0)
        self.menu.add_cascade(label="Project", menu=self.filemenu)
        self.filemenu.add_command(label="New", command=lambda: self.__create())
        self.filemenu.add_command(label="Open", command=lambda: self.__open())
        self.filemenu.add_command(label="Close", command=self.root.quit)

        self.helpmenu = Menu(self.menu)
        self.menu.add_cascade(label="Help", menu=self.helpmenu)
        self.helpmenu.add_command(label="About", )

        mainloop()

    def __create(self):
        self.preprocess.createProject()
        directory = self.preprocess.getDirectory()
        directory = "Created project in: \t" + directory
        Label(self.root, height=5, width=100, text=directory).grid(row=0, column=0, columnspan=2)
        self.__show()

    def __open(self):
        self.preprocess.openProject()
        directory = self.preprocess.getDirectory()
        directory = "Opened project: \t" + directory
        Label(self.root, height=5, width=120, text=directory).grid(row=0, column=0, columnspan=2)
        self.__show()

    def __show(self):
        statusPreprocessing, statusSolving, statusPostprocessing = self.preprocess.getProjectStatus()

        print([statusPreprocessing, statusSolving, statusPostprocessing])

        Button(self.root, text="Preprocessing", relief=RIDGE, width=100,
               command=lambda: self.__prepareSolverNode()).grid(row=1, column=0, padx=(20, 5))

        Button(self.root, text="Run simulations", relief=RIDGE, width=100,
               command=lambda: self.__runSimulations()).grid(row=2, column=0,
                                                             padx=(20, 5))

        Button(self.root, text="Postprocessing", relief=RIDGE, width=100,
               command=lambda: self.__runPostProcessing()).grid(row=3, column=0,
                                                                padx=(20, 5))

        if statusPreprocessing == "none":
            color1 = "red"
        else:
            color1 = "green"

        if statusSolving == "none":
            color2 = "red"
        else:
            color2 = "green"

        if statusPostprocessing == "none":
            color3 = "red"
        else:
            color3 = "green"

        Entry(self.root, bg=color1, relief=SUNKEN, width=4).grid(row=1, column=1)
        Entry(self.root, bg=color2, relief=SUNKEN, width=4).grid(row=2, column=1)
        Entry(self.root, bg=color3, relief=SUNKEN, width=4).grid(row=3, column=1)

    def __prepareSolverNode(self):
        print("Start preprocessing")

    def __runSimulations(self):
        print("Run simulations")

    def __runPostProcessing(self):
        print("Start postprocessing")
