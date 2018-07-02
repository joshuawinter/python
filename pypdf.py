#!/usr/bin/python



import pyPdf 




def getPDFContent(path):
    content = ""
    # Load PDF into pyPDF
    pdf = pyPdf.PdfFileReader(file(path, "rb"))
    # Iterate pages
    for i in range(0, pdf.getNumPages()):
        # Extract text from page and add to content
        content += pdf.getPage(i).extractText() + "\n"
    # Collapse whitespace
    content = " ".join(content.replace("\0xa0", " ").strip().split())
    return content

#c = getPDFContent("/home/n0083510/Downloads/telematicsreview.pdf")
txt = open('/home/n0083510/Downloads/txt.txt','w')
cnt = getPDFContent("/home/n0083510/Downloads/CDH4_Installation_Guide_4.1.pdf")
txt.write("\n".join(cnt))
txt.close()



