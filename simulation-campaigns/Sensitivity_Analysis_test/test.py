from string import Template, Formatter
import re

FileName = "basis_input_files/vadereXX.scenario"
#FileName = "basis_input_files/omnetpp22.ini"
with open(FileName) as f:
    newText = f.read()

matches = re.findall("\$var\d", newText)


newText = Template(newText)

newText.substitute(var1 = "444")

with open(FileName, "w") as f:
    f.write(newText)

print("gregre")
