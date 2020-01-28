

from tkinter import *
from tkinter import messagebox
from tkinter.filedialog import askopenfilename
import os

def onselect(event):
    global frame, choice, extraction
    w = event.widget
    selection=w.curselection()
    choice.delete(1.0, END)
    extraction = ""
    for k in range ( len(selection) ):
        value = w.get(selection[k])
        mystr = "Index: " +str(selection[k]  ) + " \t " + value + "\n"
        choice.insert(END, mystr)
        extraction = extraction + mystr

def extract_data():
    global extraction,FileName
    save_file = messagebox.askokcancel("Save variable extraction","Save variable selection in file?")
    if save_file is True:
        file = os.path.splitext(FileName)
        pathnewfile =  file[0] + ".mapping"
        f = open(pathnewfile , "w+")
        f.write(extraction)
        f.close()
        messagebox.showinfo("Export file", "Exported variable selection to \n" + os.path.basename(pathnewfile))
        root.withdraw()
        root.quit()


def create_VariableExtraction(inputtype=None):

    global root, extraction, choice,frame, FileName

    if inputtype is None:
        inputtype = [("input file", "*.scenario"), ("input file", "*.ini")]
    if "omnet" in inputtype or "ini" in inputtype :
        inputtype = [("input file", "*.ini")]

    if "vadere" in inputtype or "scenario" in inputtype:
        inputtype = [("input file", "*.scenario")]


    root = Tk()
    root.withdraw()
    messagebox.showinfo("Input file", "Choose an input file from which you want to extract variables.")
    FileName = askopenfilename(title= "Open input file",filetypes = inputtype)

    with open(FileName) as f:
        newText = f.read().split("\n")

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
    listbox = Listbox(frame,selectmode="multiple", height= 70, width=100)
    listbox.pack(side = LEFT,expand=TRUE,fill="both")


    for i in range(len(newText)):
        listbox.insert(i, newText[i])

    scrollbar = Scrollbar(frame)
    scrollbar.pack(side=LEFT, fill=Y)
    listbox.config(yscrollcommand=scrollbar.set)
    scrollbar.config(command=listbox.yview)

    listbox.bind('<<ListboxSelect>>', onselect)

    choice = Text(frame,  height=70, width = 100)
    choice.pack( side = LEFT, padx=20 )

    extraction = ""

    root.mainloop()

if __name__ == "__main__":
    create_VariableExtraction()