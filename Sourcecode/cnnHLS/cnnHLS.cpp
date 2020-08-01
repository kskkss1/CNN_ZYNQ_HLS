#include "cnnHLS.h"
void  cnnHLS(int img[784], int indexoutput[1])
{
    const float kernal2[50][8] = {
    		#include "kernal2.dat"
    		};
    const float kernal3[160][8] = {
    		#include "kernal3.dat"
    		};
    const float fullweight[80][10] = {
    		#include "fullweight.dat"
    		};
    const float fullbias[10] = {
    		#include "fullbias.dat"
    		};
	float conv1d_1[735][8],pool_1[49][8],conv1d_3[40][16],pool_2[5][16],flatten[80],dense[10];
    int i,j,k,channel;

    int index[1];
   // int imgbuf[784];
    int loop5count;
    int loop7count;
    float sum=0;
	/************conv1d_1******************/
    /*****input:img output:conv1d_1[735][8]  kernal2[50][8]******/
    Loop5:
	for(k=0;k<8;k++) //channel
	{
		Loop51:
		for(i=0;i<735;i++) //height
		{
			loop5count = i;
			sum = 0.0;
			Loop511:
			for(j=0;j<50;j++) //kernal_width
			{
				if (img[loop5count] == 0)
				{
					sum = sum;
					loop5count ++ ;
				}
				else
				{
					sum=kernal2[j][k] + sum;
					loop5count ++;
				}
			//	sum=kernal2[j][k]*img[i+j]+sum;
			}
			if(sum > 0)
			{
				conv1d_1[i][k]=sum;
			}
			else
			{
				conv1d_1[i][k]=0;
			}
		}
	}

//	Loop6:
//	for(i=0;i<735;i++)
//	{
//		Loop61:
//		for(j=0;j<8;j++)
//		{
//			if(conv1d_1[i][j]>0)
//			{
//				conv1d_1[i][j]=conv1d_1[i][j];
//			}
//			else
//			{
//               conv1d_1[i][j]=0;
//			}
//		}
//	}

	/*******************pool_1**************/
	/***** output:pool_1[49][8]  maxpool_size:15******/
	Loop7:
	for(channel=0;channel<8;channel++)
	{

		loop7count = 0;
		Loop71:
		for(i=0;i<49;i++)
		{
			float max=conv1d_1[loop7count+1][channel];
			Loop711:
			for(j=0;j<14;j++)
			{
				loop7count ++;
				if(max>conv1d_1[loop7count][channel])
				{
					max = max;
				}
				else
				{
					max=conv1d_1[loop7count][channel];
				}
			}
			pool_1[i][channel]=max;
		}
	}


	/************conv1d_3******************/
	/*****input:pool_1[49][8] output:conv1d_3[40][16]  kernal3[160][8]******/
	Loop8:
	for(channel=0;channel<16;channel++) //channel:16
	{
		Loop81:
		for(i=0;i<40;i++) //height:
		{
		//	float sum=0;
			sum = 0.0;
			Loop811:
			for(j=0;j<10;j++) //kernal_width:10*8
			{
				Loop8111:
				for(k=0;k<8;k++)
				{
					sum=kernal3[j+channel*10][k]*pool_1[i+j][k]+sum;
				}

			}
			if(sum>0)
			{
				conv1d_3[i][channel]=sum;
			}
			else
			{
                conv1d_3[i][channel]=0;
			}
		//	conv1d_3[i][channel]=sum;
		}
	}



	/*******************pool_2**************/
	/*****input:conv1d_3[40][16] output:pool_2[5][16]  maxpool_size:8******/
	Loop10:
	for(channel=0;channel<16;channel++)
	{
		Loop101:
		for(i=0;i<5;i++)
		{
			float max=conv1d_3[i*8][channel];
			Loop102:
			for(j=0;j<7;j++)
			{
				if(max>conv1d_3[i*8+j+1][channel])
				{
					max=max;
				}
				else
				{
					max=conv1d_3[i*8+j+1][channel];
				}
			}
			pool_2[i][channel]=max;
		}
	}


	/*******************flatten**************/
	/*****input:pool_2[5][16] flatten:[80]******/
	Loop11:
	for(i=0;i<5;i++)
	{
		Loop111:
		for(j=0;j<16;j++)
		{
			flatten[i*16+j]=pool_2[i][j];
		}
	}
	/*******************fullconected**************/
	/*****input:flatten:[80] output:dense[10] *********/
	Loop12:
	for(i=0;i<10;i++)
	{
	//	float sum=0;
		sum = 0.0;
		Loop121:
		for(j=0;j<80;j++)
		{
			sum=flatten[j]*fullweight[j][i]+sum;

		}
        dense[i]=sum+fullbias[i];;
	}
	/******************output*******************/
	/*************output the number*********/
//	int index=0;
	float max=dense[0];
	index[0] = 0;
	Loop13:
	for(i=1;i<10;i++)
	{
		if(max>dense[i])
		{
			max=max;
			index[0]=index[0];
		}
		else
		{
            max=dense[i];
			index[0]=i;
		}
	}
	indexoutput[0] = index[0];
//		printf("%d\n",index);
}


