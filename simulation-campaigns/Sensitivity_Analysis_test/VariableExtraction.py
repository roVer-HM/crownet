

from tkinter import *
from tkinter import messagebox
from tkinter.filedialog import askopenfilename
import os, re


def getLineNrFromIniPath():
    FileName = "basis_input_files/omnetpp.ini"
    with open(FileName) as f:
        newText = f.read()
    newText = newText.splitlines()

    FileName = "basis_input_files/test"
    with open(FileName) as f:
        inipaths = f.read().splitlines()

    pattern = "\[\w+.\w+\]\n+"

    indeces = list()
    errorstring = ""
    for item in inipaths:
        inipath = item.split("/")
        main_item = "[" + inipath[0] + "]"
        sub_item = inipath[1]

        main_item_index = newText.index(main_item)
        sub_item_index = newText.index(sub_item)

        if sub_item_index < main_item_index:
            errorstring = errorstring + "NOT found \t\t " + item + "\n"
        else:
            texttobechecked = [newText[index] for index in range(main_item_index + 1, sub_item_index + 1)]
            texttobechecked = "\n".join(texttobechecked)
            match = re.findall(pattern, texttobechecked)
            if len(match) == 0:
                print("Index:  " + str(sub_item_index) + "\t\t" + item)
                indeces.append(sub_item_index)
            else:
                errorstring = errorstring + "NOT found \t\t " + item + "\n"

    print("finished")
    return indeces, errorstring



def getIniPathFromLineNr():
    pass




def onselect(event):
    global frame, choice, extraction, listbox, newText, oldSelection
    w = event.widget
    selection = w.curselection()
    update_selection(selection)


def update_selection(selection):
    global frame, choice, extraction, listbox, newText, oldSelection

    if oldSelection is None:
        difference = None
    elif len(oldSelection)>len(selection):
        difference = list(set(oldSelection) ^ set(selection))[0]
        listbox.delete(difference)
        listbox.insert(difference, newText[difference])
    oldSelection = selection

    choice.delete(1.0, END)
    extraction = ""
    counter = 0
    for k in range ( len(selection) ):
        #value = w.get(selection[k])
        index = selection[k]
        mystr = newText[index].replace(":", "=")
        mystr = mystr.replace(",", "")
        mystr = mystr.replace("#", "=")
        mystr = mystr.split(" = ")

        if len(mystr) >= 2 and (mystr[1].find("{") == -1):
            variableX  = "VARIABLE" + str(counter)
            printout = newText[index].replace(mystr[1], variableX )
            listbox.delete(index)
            listbox.insert(index,printout)
            listbox.selection_set(index)

            mystr2 = "Index: " + str(index) + " \t " + variableX + " = " + mystr[1] + "\n"
            choice.insert(END, mystr2)
            extraction = extraction + mystr2
        else:
            listbox.selection_clear(index)
            listbox.itemconfig(index, background="red")
        counter = counter +1




def extract_data():
    global extraction,FileName,listbox

    save_file = messagebox.askokcancel("Save variable extraction","Save variable selection in file?")
    if save_file is True:

        textwithVariables = listbox.get(0, END)
        file = os.path.splitext(FileName)

        pathnewfile0 =  file[0] + "2" + file[1]
        f = open(pathnewfile0 , "w+")
        for item in textwithVariables:
            f.write("%s\n" % item)
        f.close()
        pathnewfile = file[0] + "2_defaultvalues"
        f = open(pathnewfile, "w+")
        f.write("Default values for VARIABLEx in file " + os.path.basename(pathnewfile0) + "\n" )
        f.write(extraction)
        f.close()

        messagebox.showinfo("Export file", "Exported variable selection to \n" + os.path.basename(pathnewfile))
        root.withdraw()
        root.quit()


def create_VariableExtraction(inputtype=None):

    global root, extraction, choice,frame, FileName, listbox, newText, oldSelection

    if inputtype is None:
        inputtype = [("input file", "*.scenario"), ("input file", "*.ini")]
    if "omnet" in inputtype or "ini" in inputtype :
        inputtype = [("input file", "*.ini")]

    if "vadere" in inputtype or "scenario" in inputtype:
        inputtype = [("input file", "*.scenario")]


    oldSelection = None

    root = Tk()
    root.withdraw()
    messagebox.showinfo("Input file", "Choose an input file from which you want to extract variables.")
    FileName = askopenfilename(title= "Open input file",filetypes = inputtype)

    with open(FileName) as f:
        newText = f.read().split("\n")

    ask_preselection = messagebox.askokcancel("Preselection", "Use preselection?")
    if ask_preselection is True:
        print("Chose preselection")
        #preselection = [3,8]
        preselection, errormessage = getLineNrFromIniPath()
        oldSelection = preselection
    else:
        preselection = None

    root = Tk()
    root.title("Create variable extraction file")

    topframe = Frame(root)
    topframe.pack()

    w = Label(topframe, text="\n\n Basis input file: " + os.path.basename(FileName),width= 100, anchor="w")
    w.pack(side=LEFT,pady=0,)

    w = Label(topframe, text=" ", width=10,height=5 )
    w.pack(side=LEFT, pady=0)

    greenbutton = Button(topframe, text="Save", command=extract_data,width= 40,)
    greenbutton.pack(side=RIGHT)

    frame = Frame(root)
    frame.pack()

    screen_width = int( 0.9* frame.winfo_screenwidth() /60 )
    screen_height = frame.winfo_screenmmheight()

    # create listbox
    listbox = Listbox(frame,selectmode="multiple", height= 70, width=100,selectbackground="green yellow")


    listbox.pack(side = LEFT,expand=TRUE,fill="both")


    for i in range(len(newText)):
        listbox.insert(i, newText[i])





    scrollbar = Scrollbar(frame)
    scrollbar.pack(side=LEFT, fill=Y)
    listbox.config(yscrollcommand=scrollbar.set)
    scrollbar.config(command=listbox.yview)

    listbox.bind('<<ListboxSelect>>', onselect)

    choice = Text(frame,  height=70, width = 100,)
    choice.pack( side = LEFT, padx=20 )

    extraction = ""
    if preselection is not None:
        update_selection(preselection)


    root.mainloop()

if __name__ == "__main__":
    create_VariableExtraction()