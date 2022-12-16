/* testpf.c */
#include <stdio.h>
#include <time.h>
#include "pf.h"
#include "pftypes.h"

#define FILE1	"file1"
#define FILE2	"file2"
#define FILE3	"data.csv"
 

void delay(int number_of_seconds) 
{ 
 
    int milli_seconds = 1000000 * number_of_seconds; 
	clock_t start_time = clock();  
	int count = 10;
	while (clock() < start_time + milli_seconds)
	{
		count--;
	}
} 

long int data[10];
int no_of_records = 0;
int no_of_pages = 0;
//algo=1;
int seek_num = 0;
double seek_time = 0;
main()
{
lb:;
int error;
int i;
int pagenum,*buf;
int *buf1,*buf2;
int fd1,fd2;

	FILE *f = fopen("test_pf.csv","r+");

	printf("DATA:\n");
	char ff[1024];
	while(fgets(ff,1024,f))
	{
		printf("%s",ff);

		long long int content = 0;
		
		int x = 0;
		while(ff[x] != 10)
		{
			x++;
		}
		
		for(int i=x-1;i>=0;i--)
		{
			int c = ff[i];
			content = content*1000 + c;
		}
			
		data[no_of_records] = content;
		no_of_records++;
	
	}
	printf("\n");
	fclose(f);
	/* create a few files */
	if ((error=PF_CreateFile(FILE1))!= PFE_OK){
		PF_PrintError("file1");
		exit(1);
	}
	printf("file1 created\n");

	if ((error=PF_CreateFile(FILE2))!= PFE_OK){
		PF_PrintError("file2");
		exit(1);
	}
	printf("file2 created\n");

	/* write to file1 */
	writefile(FILE1);

	/* print it out */
	readfile(FILE1);
	double backup_time1 = (double) clock();
	//printf("bt1 %f\n", backup_time1);
	backup(FILE1, FILE2);
	double backup_time2 = (double) clock();
	//printf("bt2 %f\n", backup_time2);
	double backup_time = backup_time2 - backup_time1;
	printf("Total Backup time from RAID 1 disk to RAID 0 disk: %f ms\n", backup_time);
	backup_time = backup_time/seek_num;
	printf("Average Backup time: %f ms\n", backup_time);

	/* write to file2 */
	//writefile(FILE2);

	/* print it out */
	//readfile(FILE2);

	/* print the files */
	readfile(FILE1);
	readfile(FILE2);

	double ch_time1 = (double) clock();
	int ch_time = 0;		
	char ch;
	printf("\n");
	while(ch_time < 10000000)
	{
		printf("Waiting for %d seconds\n",(10 - ch_time/1000000));
		delay(1);
		double ch_time2 = (double) clock();
		ch_time = ch_time2 - ch_time1;
	}
	printf("\n");
	//scanf(" %c%*c",&ch);
	ch = 'y';
	if(ch == 'y')
	{
		ch = NULL;
		for(int i=0;i<no_of_records;i++)
			data[i] = 0;
		no_of_records = 0;
		no_of_pages = 0;
		seek_num = 0;
		seek_time = 0;

		/* destroy the two files */
		if ((error=PF_DestroyFile(FILE1))!= PFE_OK){
			PF_PrintError("destroy file1");
			exit(1);
		}
		if ((error=PF_DestroyFile(FILE2))!= PFE_OK){
			PF_PrintError("destroy file2");
			exit(1);
		}

		goto lb;
	}

	/* print the buffer */
//	printf("\n\n\n*********************************Lastly buffer:*******************\n\n");
//	PFbufPrint();

	/* print the hash table */
//	printf("\n********************************************hash table:\n");

//	PFhashPrint();
}



/************************************************************
Open the File.
allocate as many pages in the file as the buffer
manager would allow, and write the page number
into the data.
then, close file.
******************************************************************/

writefile(fname)
char *fname;
{
int i;
int fd,pagenum;
long long int *buf;
int error;
double time1 = 0;

	/* open file1, and allocate a few pages in there */
	if ((fd=PF_OpenFile(fname))<0){
		PF_PrintError("open file1");
		exit(1);
	}
	printf("opened %s\n",fname);
	
	time1 = (double) clock();
	//printf("time1 %f\n",time1);
	
	for (i=0; i < no_of_records; i++){
		if ((error=PF_AllocPage(fd,&pagenum,&buf))!= PFE_OK){
			PF_PrintError("first buffer\n");
			exit(1);
		}
		*((long long int *)buf) = data[i];
		//PFbufPrint();
		printf("allocated page %d\n",pagenum);
		no_of_pages++;

		long long int content = (long long int) data[i];
		char ccc[1024];
		int i=0;
		while(content != 0)
		{
			int c = content%1000;
			char cc = c;
			ccc[i] = cc;
			i++;
			content = content/1000;
		}
		printf("Record Write: %s\n",ccc);

		if ((error=PF_UnfixPage(fd,pagenum,TRUE))!= PFE_OK){
			PF_PrintError("unfix buffer\n");
			exit(1);
		}
	}
	
	/*Data Mirroring in RAID 1*/
	for (i=0; i < no_of_records; i++){
		if ((error=PF_AllocPage(fd,&pagenum,&buf))!= PFE_OK){
			PF_PrintError("first buffer\n");
			exit(1);
		}
		*((long long int *)buf) = data[i];
		//PFbufPrint();
		printf("allocated page %d\n",pagenum);
		no_of_pages++;
	
		long long int content = (long long int) data[i];
		char ccc[1024];
		int i=0;
		while(content != 0)
		{
			int c = content%1000;
			char cc = c;
			ccc[i] = cc;
			i++;
			content = content/1000;
		}
		printf("Record Write: %s\n",ccc);

		if ((error=PF_UnfixPage(fd,pagenum,TRUE))!= PFE_OK){
			PF_PrintError("unfix buffer\n");
			exit(1);
		}
	}

	printf("Total Number of Pages %d\n",no_of_pages);
	
	double time2 = (double) clock();
	//printf("time2 %f\n",time2);
	seek_time = time2 - time1;
	printf("Total Time to write in the disk with RAID 1 is %f ms\n",seek_time);
	seek_time = seek_time/no_of_pages;
	printf("Average Time to write in the disk with RAID 1 is %f ms\n",seek_time);

	/* close the file */
	if ((error=PF_CloseFile(fd))!= PFE_OK){
		PF_PrintError("close file1\n");
		exit(1);
	}

}

/**************************************************************
print the content of file
*************************************************************/
readfile(fname)
char *fname;
{
int error;
long long int *buf;
int pagenum;
int fd;

	printf("opening %s\n",fname);
	if ((fd=PF_OpenFile(fname))<0){
		PF_PrintError("open file");
		exit(1);
	}
	printfile(fd,fname);
	if ((error=PF_CloseFile(fd))!= PFE_OK){
		PF_PrintError("close file");
		exit(1);
	}
}

printfile(fd,fname)
int fd;
char *fname;
{
int error;
long long int *buf;
int pagenum;
double time1 = 0;
int count = 0;

	printf("reading file: %d\n",fd);
	pagenum = -1;
	time1 = (double) clock();
	//printf("time1 %f\n",time1);
	while (count < no_of_pages/2 && (error=PF_GetNextPage(fd,&pagenum,&buf))== PFE_OK){
		printf("got page %d, %d\n",pagenum,*buf);
		//delay(1);
		count++;
		
		long long int content = (long long int) *buf;
		char ccc[1024];
		int i=0;
		while(content != 0)
		{
			int c = content%1000;
			char cc = c;
			ccc[i] = cc;
			i++;
			content = content/1000;
		}
		printf("Record read: %s\n",ccc);


		if ((error=PF_UnfixPage(fd,pagenum,FALSE))!= PFE_OK){
			PF_PrintError("unfix");
			exit(1);
		}
	}

	printf("Total Number of Page Read is %d\n",count);

	double time2 = (double) clock();
	int RAID;
	if(fname == "file1")
		RAID = 1;
	else
		RAID = 0;
	//printf("time2 %f\n",time2);
	seek_time = time2 - time1;
	printf("Total Time to read in the disk with RAID %d: %f ms\n",RAID,seek_time);
	seek_time = seek_time/count;
	printf("Average Time to read in the disk with RAID %d: %f ms\n",RAID,seek_time);
		
	if (error != PFE_EOF && count != no_of_pages/2){
		PF_PrintError("not eof\n");
		exit(1);
	}
	printf("eof reached\n");

}

backup(fname)
char *fname;
{
int error;
long long int *buf;
int pagenum;
int fd;

	printf("opening %s\n",fname);
	if ((fd=PF_OpenFile(fname))<0){
		PF_PrintError("open file");
		exit(1);
	}
	printf("%s\n",fname);
	//printf("%s\n",fname1);
	backupfile(fd,FILE2);
	if ((error=PF_CloseFile(fd))!= PFE_OK){
		PF_PrintError("close file");
		exit(1);
	}
}

backupfile(fd,fname)
char *fname;
int fd;
{
int error;
long long int *buf;
int pagenum;
double time1 = 0;
double per_page_seek_time = 0;
double total_per_page_seek_time = 0;

	printf("reading file: %d\n",fd);
	pagenum = -1;
	time1 = (double) clock();
	while (seek_num < no_of_pages/2 && (error=PF_GetNextPage(fd,&pagenum,&buf))== PFE_OK){
		printf("got page %d, %d\n",pagenum,*buf);
		seek_num++;
		
		double time2 = (double) clock();
		per_page_seek_time = time2 - time1;
		printf("Per Page Time to read in the disk with RAID 1 for backup: %f\n",per_page_seek_time);
		
		if(seek_num == 1)
		{
			FILE *f = fopen(FILE3, "w+");
			
			fprintf(f,"%f \t",per_page_seek_time);

			fclose(f);
		
		}		
		else
		{ 
			FILE *f = fopen(FILE3, "a");

			fprintf(f,"%f \t",per_page_seek_time);

			fclose(f);
	
		}


		writebackup(fname,pagenum,buf);

		double time3 = (double) clock();
		total_per_page_seek_time = time3 - time1;
		printf("Total Per Page Time for backup: %f\n",total_per_page_seek_time);

		FILE *f1 = fopen(FILE3, "a"); 

		fprintf(f1,"%f\n",total_per_page_seek_time);

		fclose(f1);		
		if ((error=PF_UnfixPage(fd,pagenum,FALSE))!= PFE_OK){
			PF_PrintError("unfix");
			exit(1);
		}

		time1 = (double) clock();
	
	}

	printf("Number of Seek: %d\n",seek_num);

	if (error != PFE_EOF && seek_num != no_of_pages/2){
		PF_PrintError("not eof\n");
		exit(1);
	}
	printf("eof reached\n");

}

writebackup(fname,pagenum,buf1)
char *fname;
long long int *buf1;
int pagenum;
{
int i;
int fd;
int error;
long long int *buf;
double time1 = 0;
double per_page_write_seek_time = 0;

	printf("writing in file: %s\n",fname);
	
	/* open file1, and allocate a few pages in there */
	if ((fd=PF_OpenFile(fname))<0){
		PF_PrintError("open file1");
		exit(1);
	}
	printf("opened %s\n",fname);
	
	time1 = (double) clock();
	
	if ((error=PF_AllocPage(fd,&pagenum,&buf))!= PFE_OK){
		PF_PrintError("first buffer\n");
		exit(1);
	}
	*((long long int *)buf) =(long long int) *buf1;
	printf("allocated page %d\n",pagenum);
	
	double time2 = (double) clock();
	per_page_write_seek_time = time2 - time1;
	printf("Per Page Write Time in the disk with RAID 0 for backup: %f\n",per_page_write_seek_time);
	
	FILE *f = fopen(FILE3, "a"); 

	fprintf(f,"%f \t",per_page_write_seek_time);

	fclose(f);		
	/* unfix these pages */
	if ((error=PF_UnfixPage(fd,pagenum,FALSE))!= PFE_OK){
		PF_PrintError("unfix buffer\n");
		exit(1);
	}

	/* close the file */
	if ((error=PF_CloseFile(fd))!= PFE_OK){
		PF_PrintError("close file1\n");
		exit(1);
	}

}
