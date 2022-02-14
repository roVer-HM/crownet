import PyPDF2 as pdfreader
import glob
from PyPDF2.pdf import PdfFileReader
import reportlab
from reportlab.pdfgen import canvas
from reportlab.lib.units import mm, inch
import roveranalyzer.simulators.opp as OMNeT
from roveranalyzer.analysis import OppAnalysis
import io
from os.path import join
import roveranalyzer.simulators.crownet.dcd as DensityMap
from roveranalyzer.utils.general import Project

def add_page(i, data):

    par_var = [
        {"omnet": {"*.misc[*].nTable.maxAge": "3s"}},
        {"omnet": {"*.misc[*].nTable.maxAge": "5s"}},
        {"omnet": {"*.misc[*].nTable.maxAge": "8s"}},
        {"omnet": {"*.misc[*].nTable.maxAge": "10s"}},
    ]

def main():
    p = "../suqc/opp_stationary3_ymfPlus/simulation_runs/outputs"
    p2 = "subwayStationary_multiEnb_out/fig"
    imgs = ["count_over_time", "error_histogram", "neighborhood_table_size"]


    paths = []
    for i in range(0, 12):
        path = f"{p}/Sample_{i}_0/{p2}"
        for img in imgs:
            paths.append(f"{path}/{img}.pdf")
            print(paths[-1])
    

    pdfs = []
    for p in paths:
        pdf = pdfreader.PdfFileReader(p)
        pdfs.append(pdf)



    out = pdfreader.PdfFileWriter()
    packet = io.BytesIO()
    can = canvas.Canvas(packet, pagesize=(406*mm, 229*mm))
    can.drawString(406*0.3*mm, 110*mm, "Hello")
    can.save()
    packet.seek(0)
    _pdf = PdfFileReader(packet)
    # out.addPage(_pdf.getPage(0))

    for i, p in enumerate(pdfs):
        pp: pdfreader.PdfFileReader  = p
        out.addPage(pp.getPage(0))

    with open("./out_ymfplus.pdf", "wb") as o:
        out.write(o)
    
    print("done")



if __name__ == "__main__":
    main()