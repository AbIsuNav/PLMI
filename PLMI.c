
/*
 * PLMI code file
 * PLMI.c
 * Kennet Garcia, Abgeiba Isunza
 *
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "PLMI.h"


//---------------------------METHODS FOR PLMI----------------------------------------
float linInterp(float Input[numDims]){
	//Local variables
    float interpValue=0, prod;
    float res[numDimsIt];
    float tracks[numDims][2][2];
    unsigned int ids[numDims]={0,0,0};
    int count[numDims]={0,0,0};

    for(unsigned char i=0; i<numDims; i++){
        if (i==0) bisection(BX, Input[i]);
        if (i==1) bisection(BY, Input[i]);
        if (i==2) bisection(BZ, Input[i]);
        tracks[i][0][0]=bisValue[0];
        tracks[i][0][1]=bisValue[1];
        tracks[i][1][0]=bisValue[0]+1;
        tracks[i][1][1]=1-bisValue[1];
    }

    for(unsigned char i=0; i<numDimsIt; i++){
        prod=1;
        for (unsigned char j=0;j<numDims; j++){
            ids[j]=tracks[j][count[j]][0];
            prod=prod*tracks[j][count[j]][1];
        }
		res[i]=pgm_read_float(&(Values[ids[0]][ids[1]][ids[2]]))*prod;
		
        //Update Count
        count[0]=count[0]+1;
        for(unsigned char i=0; i<numDims; i++){
            if(count[i]==2 && i!=(numDims-1)){
                count[i]=0;
                count[i+1]=count[i+1]+1;
            }
        }
    }

    //Get final result
    for(unsigned char i=0; i<numDimsIt; i++){
        interpValue=interpValue+res[i];
    }
    return interpValue;
}

void bisection(float BarsN[numBars], float desiredValue){
    unsigned char k=round(numBars/2), i=k-1;
    unsigned char flag=1;
    float id=0, s=0;

    while (flag){
        //Get the next half of the vector
        k=k/2;
        //Compare desiredValue with specific index
        if(desiredValue>BarsN[i]){
            //Update index and compare
            i=i+round(k);
            if(i>numBars-2) i=numBars-2;
            //Check the number of remaining elements
            if(k<1){
                //Update ID
                id=i;
                //Get the similitude
                s=(BarsN[i+1]-desiredValue)/(BarsN[i+1]-BarsN[i]);
                //End the while loop
                flag=0;
            }
        }else{
            //Update index and compare
            i=i-round(k);
            if(i<1) i=1;
            //Check the number of remaining elements
            if(k<1){                //Update ID
                id=i-1;
                //Get the similitude
                s=(BarsN[i]-desiredValue)/(BarsN[i]-BarsN[i-1]);
                //End the while loop
                flag=0;
            }
        }
    }
    //Create output
    bisValue[0]=id; bisValue[1]=s;
}