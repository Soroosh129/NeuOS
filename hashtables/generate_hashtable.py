import pandas as pd
import numpy as np
import glob
import copy


files=glob.glob("/home/nvidia/caffe-build/dvfs/med/*.log")

Configfile=open("freq.config","r")

output1=open("Latency.txt","w")
output2=open("TotalEnergy.txt","w")
output3=open("GPUEnergy.txt","w")
output4=open("CPUEnergy.txt","w")
output5=open("MemEnergy.txt","w")
output6=open("Uncertainty.txt","w")
output7=open("normLatency.txt","w")
output8=open("normTotalEnergy.txt","w")
output9=open("tmpUncertainty.txt","w")



#layers=["config"]
Latency=[]
normLatency=[]
TotalEnergy=[]
normTotalEnergy=[]
GPUEnergy=[]
CPUEnergy=[]
MemEnergy=[]
maxLatency=[0 for i in range(100)]
maxTotalEnergy=[0 for i in range(100)]

config=[]
for line in Configfile.readlines():
	config.append(line.split(' '))

def Combine(result,config):
        i=0
	for index in range(len(config)):
                tmp = int(result[i][0])
            	if(tmp==int(config[index][0])):
                    result[i].extend(config[index][1:])
                    i+=1
                if(i>=len(result)):
                    return


#result.append(layers)
def FormatWrite(list,file):
	for i in list:
		file.write(str(i)+'\t')
	file.write('\n')

for file in files:
	row=[]	
	fl=open(file)
	
	#insert configure name	

    	matrix=fl.readlines()[3:26]
	
	RLatency=[]
	RTotalEnergy=[]
	RGPUEnergy=[]
	RCPUEnergy=[]
	RMemEnergy=[]
	RLatency.append(int(file[41:-4]))
	RTotalEnergy.append(int(file[41:-4]))
	RGPUEnergy.append(int(file[41:-4]))
	RCPUEnergy.append(int(file[41:-4]))
	RMemEnergy.append(int(file[41:-4]))

	#for each layer,insert the corresponding value 
	for idx,layer in enumerate(matrix):
            list=layer.split(' ')
            if(int(list[1]) > maxLatency[idx]) : maxLatency[idx] = int(list[1]) 
            RLatency.append(float(list[1]))
            if(round(float(list[2]),2) > maxTotalEnergy[idx]) : maxTotalEnergy[idx] = round(float(list[2]),2) 
            RTotalEnergy.append(round(float(list[2]),2))
            RGPUEnergy.append(round(float(list[3]),2))
            RCPUEnergy.append(round(float(list[4]),2))
            RMemEnergy.append(round(float(list[5]),2))
            
	Latency.append(RLatency)
	TotalEnergy.append(RTotalEnergy)
	GPUEnergy.append(RGPUEnergy)
	CPUEnergy.append(RCPUEnergy)
	MemEnergy.append(RMemEnergy)
    	
Latency=sorted(Latency,key=lambda config:config[0])
TotalEnergy=sorted(TotalEnergy,key=lambda config:config[0])
GPUEnergy=sorted(GPUEnergy,key=lambda config:config[0])
CPUEnergy=sorted(CPUEnergy,key=lambda config:config[0])
MemEnergy=sorted(MemEnergy,key=lambda config:config[0])

normLatency=copy.deepcopy(Latency)
normTotalEnergy=copy.deepcopy(TotalEnergy)
tmpUncertainty=copy.deepcopy(Latency)
uncertainty=[]

for i in range(len(Latency)):
    for idx in range(len(Latency[i])-1):
        if(maxLatency[idx]!=0):
            normLatency[i][idx] = float(Latency[i][idx+1])/maxLatency[idx]
        else:
            normLatency[i][idx] = 0
        if(maxTotalEnergy[idx]!=0):
            normTotalEnergy[i][idx] = float(TotalEnergy[i][idx+1])/maxTotalEnergy[idx]
        else:
            normTotalEnergy[i][idx] = 0
        if(normLatency[i][idx]!=0):
            tmpUncertainty[i][idx] = normTotalEnergy[i][idx]/normLatency[i][idx]
        else :
            tmpUncertainty[i][idx] = 0
        

confCount = 0
for idx in range(len(tmpUncertainty[confCount])):
    maxUncertainty = 0
    for i in range(len(tmpUncertainty)):
        try:
            if(tmpUncertainty[i][idx] > maxUncertainty):
                maxUncertainty = tmpUncertainty[i][idx]
        except IndexError:
            maxUncertainty = maxUncertainty    
    uncertainty.append(maxUncertainty)
print(maxLatency)
print(maxTotalEnergy)
#print(normLatency)
#print(normTotalEnergy)
#print(uncertainty)



Combine(Latency,config)
Combine(TotalEnergy,config)
Combine(GPUEnergy,config)
Combine(CPUEnergy,config)
Combine(MemEnergy,config)

for item in uncertainty:
    output6.write(str(item)+'\t')
for index in range(len(Latency)):
	FormatWrite(Latency[index],output1)
	FormatWrite(normLatency[index],output7)
	FormatWrite(normTotalEnergy[index],output8)
	FormatWrite(TotalEnergy[index],output2)
	FormatWrite(GPUEnergy[index],output3)
	FormatWrite(CPUEnergy[index],output4)
	FormatWrite(MemEnergy[index],output5)
        #FormatWrite(uncertainty[index],output6)
	FormatWrite(tmpUncertainty[index],output9)

