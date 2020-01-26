#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
//#include<conio.h>
int physicalMemoryIndex;
int pageTable[256];
int physicalMemory[128][256];
int TLB[16][2];
int TLBindex;
int pageindex;
int TLBHitCount;
int TLBMissCount;
int TLBsize;
void initialize()
{
	physicalMemoryIndex=0;
	TLBsize=0;
	TLBHitCount=0;
	TLBMissCount=0;
	pageindex=0;
	TLBindex=0;
	int i=0,j=0;
	for(i=0;i<256;i++)
	{
		pageTable[i]=-1;
	}
	for(i=0;i<16;i++)
	{
		TLB[i][0]=-1;
	}
	for(i=0;i<128;i++)
	{
		for(j=0;j<256;j++)
		{
			physicalMemory[i][j]=-1;
		}
	}
}

int charToInt(char n[])
{
	int i=0;
	int sum=0;
	while(n[i+1]!=0)
	{
		sum=sum*10+n[i]-48;
		i++;	
	}
return sum;
}

int readNumber(FILE *stream)
{
	char ch[2]={0,0};
	char number[10];
	int index=-1;
	while(ch[1]!=10&&ch[0]!=10&&!feof(stream))
	{
		index++;
		fread(ch,1,1,stream);
		number[index]=ch[0];	
	}
	
	if(index==0)
	{
		return -1;
	}
	index++;
	number[index]=0;
	return charToInt(number);
}

void split(int la,int *pt,int *os)
{
	int test1=la&255;
	*os=test1;
	int test2=la>>8;
	*pt=test2;
	
}

void pageFault(int pt)
{
	FILE* fp;
	int i=0;
	char v[2];
	int index=-1;
	fp=fopen("BACKING_STORE.bin","rb");
	fseek(fp,pt*256,0);
	for(i=0;i<256;i++)
	{
		fread(v,1,1,fp);
		physicalMemory[pageindex][i]=v[0];
	}	
	fclose(fp);
	pageTable[pt]=pageindex;
	pageindex++;
	physicalMemoryIndex++;
	if(physicalMemoryIndex>128)////look up for replaced frame in page table and TLB
	{
		pageindex=pageindex%128;
		for(i=0;i<256;i++)
		{
			if(pageTable[i]==pageindex)
			{
				index=i;
			}
		}
		for(i=0;i<16;i++)
		{
			if(TLB[i][0]==index)
			{
				TLB[i][0]=pt;
				TLBHitCount--;
			}
		}
		pageTable[index]=-1;
	}
}

int checkTLBFIFO(int pt)
{
	int i=0;
	for(i=0;i<16;i++)
	{
		if(TLB[i][0]==pt)
		{
			TLBHitCount++;
			return TLB[i][1];
		}
	}
	TLB[TLBindex][0]=pt;
	TLB[TLBindex][1]=pageTable[pt];
	TLBindex++;
	TLBindex=TLBindex%16;
	return -1;
}

int main(int argc, char *argv[])
{
	char filename[20];
	if(argc!=2)
	{
		printf("The number of arguments must be 1");	
	}
	strcpy(filename,argv[1]);

	FILE* fp;
	int pt;
	int os;
	int pfno=0;
	int tlbno=0;
	char page[256][2];
	int logicalAddress=0;
	int physicalAddress=0;
	int value=1;
	int frameNumber=-1;
	int n=0;
	//////////////With FIFO replacement///////////////
	printf("\n-----------------------With FIFO Replacement-----------------------\n\n");
	initialize();
	fp=fopen(filename,"r");
	while(!feof(fp))
	{
		n++;
		logicalAddress=readNumber(fp);
		if(logicalAddress!=-1)
		{		
			split(logicalAddress,&pt,&os);		
			if(pageTable[pt]==-1)//if pagefault
			{
				pageFault(pt);
				pfno++;
			}
			frameNumber=checkTLBFIFO(pt);
			if(frameNumber==-1)
			{
				frameNumber=pageTable[pt];			
			}
			printf("%d. Logical memory= %d\t",n,logicalAddress);
			physicalAddress=frameNumber*256+os;
			printf("Physical Address= %d\t",physicalAddress);
			value=physicalMemory[pageTable[pt]][os];
			printf("Value = %d\n",value);
		}
	}
	printf("Pafe Fault Count = %d\n",pfno);
	printf("Page Fault Rate = %.3f\n", pfno*100.0/1000.0);
	printf("TLB Hit Count(FIFO) = %d\n",TLBHitCount);
	printf("TLB hit rate = %.3f\n\n\n",TLBHitCount*100.0/1000.0);
	fclose(fp);
return 0;
}
