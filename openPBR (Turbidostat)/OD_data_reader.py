import re


f = open("log_OD.txt", "r")
csv = open("OD_log.csv","w")

OD680 = []
OD720 = []

csv.write("OD680_Read (mV),OD720_Read (mV)")

for line in f:
    
    if line.startswith("OD680 ="):
        search = re.search("OD[0-9]{1,3} *= *([0-9]{3}).*OD[0-9]{1,3} *= *([0-9]{1,3})",line)
        csv.write(search.group(1))
        csv.write(",")
        csv.write(search.group(2))
        csv.write("\n")
        
f.close()
csv.close()
        
    
        
    
