# from string import Template, Formatter
# import re
#
# def getLineNrFromIniPath():
#     FileName = "basis_input_files/omnetpp.ini"
#     with open(FileName) as f:
#         newText = f.read()
#     newText = newText.splitlines()
#
#     FileName = "basis_input_files/test"
#     with open(FileName) as f:
#         inipaths = f.read().splitlines()
#
#     pattern = "\[\w+.\w+\]\n+"
#
#     indeces = list()
#     errorstring = ""
#     for item in inipaths:
#         inipath = item.split("/")
#         main_item = "[" + inipath[0] + "]"
#         sub_item = inipath[1]
#
#         main_item_index = newText.index(main_item)
#         sub_item_index = newText.index(sub_item)
#
#         if sub_item_index < main_item_index:
#             errorstring = errorstring + "NOT found \t\t " + item + "\n"
#             indeces.append("")
#         else:
#             texttobechecked = [newText[index] for index in range(main_item_index+1, sub_item_index + 1)]
#             texttobechecked = "\n".join(texttobechecked)
#             match = re.findall(pattern, texttobechecked)
#             if len(match)==0:
#                 print("Index:  " + str(sub_item_index) + "\t\t" + item)
#                 indeces.append(sub_item_index)
#             else:
#                 errorstring = errorstring + "NOT found \t\t " + item + "\n"
#                 indeces.append("")
#
#     return indeces, errorstring
#


import difflib
# from jsonpath_ng import jsonpath, parse
#
# def getKeysByValues(dictOfElements, listOfValues):
#     listOfKeys = list()
#     listOfItems = dictOfElements.items()
#     for item  in listOfItems:
#         if item[1] in listOfValues:
#             listOfKeys.append(item[0])
#     return listOfKeys
#
# # or
with open("basis_input_files/vadere.scenario") as f:
    f1 = f.read().splitlines()
with open("basis_input_files/vadere2.scenario") as f:
    f2 = f.read().splitlines()

aa = difflib.context_diff(f1, f2)

#bb = difflib.Differ(f1, f2)



for line in aa:
    l = line
    if l[0] == "+" or l[0] == "-":
        print(l)
    else:
        print("skipped")

#cc = "\n".join(difflib.context_diff(f1, f2))
#print(cc)
# listOfKeys = getKeysByValues(my_dict, ["VARIABLE0", [6, 7]])
#
# # Iterate over the list of values
# for key in listOfKeys:
print("finished")


