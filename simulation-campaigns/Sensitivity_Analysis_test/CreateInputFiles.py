
FileName = "basis_input_files/omnetpp.mapping"

with open(FileName) as f:
    newText = f.read()

newText = newText.splitlines()

indeces = list()
values = list()

for item in newText:
    index, value = item.split("\t")
    index = int(index.split("Index: ")[1])

    value = value.replace(":","=")
    value = value.replace(",", "")
    value = float(value.split("=")[1])


    indeces.append(index)
    values.append(value)
    print(index)


FileName = "basis_input/vadere.scenario"

with open(FileName) as f:
    newText2 = f.read()






print("finished")