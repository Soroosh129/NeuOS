import pandas as pd
import numpy as np
import glob

files=glob.glob("*.log")
wr=open("result.txt","w")
total=[]
layers=[]

for i in range(24):
    new = []
    layers.append(new)

for file in files:
    fl=open(file)
    matrix=fl.readlines()[3:27]
    newmatrix=[]

    for index in range(len(matrix)):
	length=len(matrix[0])
        newrow=matrix[index].split(' ')
	if(newrow[2]=="0" or newrow[1]=="0"):
		ef=0
	else:
		ef=float(newrow[2])/float(newrow[1])
	newrow.append(ef)
        newrow.append(file[26:])
        newmatrix.append(newrow)
        layers[index].append(newrow)
    total.extend(newmatrix)



def MIN(array):

    minvalue=0
    minindex=0
    for index in range(len(array)):
        if(array[index]!=0):
              minvalue=array[index]
              minindex=index
              break;             
    for index in range(len(array)):
	if(float(array[index])<minvalue and float(array[index])!=0):
            minvalue=float(array[index])
            minindex=index
    return minvalue,minindex
    
result=[]

for index in range(len(layers)):
   layers[index]=np.array(layers[index])
   
   #Marray1=MIN(layers[index][:,1]);
   #Marray2=MIN(layers[index][:,2]);
   #Marray3=MIN(layers[index][:,-2]);

   result.append('layer'+str(index)+'  min latency:  '+str(Marray1[0])+"  "+str((layers[index])[Marray1[1],-1])+"  min energy: "+str(Marray2[0])+"  "+str((layers[index])[Marray2[1],-1])+"  min ef: "+str(Marray3[0])+"  "+str((layers[index])[Marray3[1],-1])+'\n')
for i in result:
  wr.write(i)

