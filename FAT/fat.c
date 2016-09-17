#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>

#define BPB_BytsPerSec 0x0b   
#define BPB_SecPerClus 0x0d   
#define BPB_RsvdSecCnt 0x0e   
#define BPB_NumFATs    0x10   
#define BPB_FATSz32    0x24   

int fd;
unsigned short int BytsPerSec, SecPerClus = 0, RsvdSecCnt, NumFATs = 0, file_cnt = 0, dir_cnt = 0;
unsigned int FirstDataSector, DataSector = 0, FATSz32, correction, FatStart, spaces = 0;

void function(unsigned int DataSector_)
{
	unsigned char c, ch, cht, *name = calloc(11, sizeof(char));
	unsigned short int offset = 0, clus_num = 0, i = 0;
	unsigned int address;

	lseek(fd, DataSector_, SEEK_SET);
	read(fd, &ch, 1);

	while (ch == 'A' || ch =='B' || ch == 0xe5 || ch == 0x2e || ch == 0x20)
	{
		DataSector_ += offset;
		lseek(fd, DataSector_, SEEK_SET);
		read(fd, &ch, 1);

		if (ch == 0xe5 || ch == 0x2e || ch == 0x20)
		{
			offset = 32;
		}
		else if (ch == 'A')
		{
			offset = 64;
			for(i = 0; i < spaces; i++)
			{
				printf(" ");
			}
			for(i = 0; i < 31; i++)
			{
				read(fd, &c, 1);
				if (isprint(c))
				{
					if (i == 12)
						continue;
					printf("%c", c);
				}
			}

			lseek(fd, DataSector_ + 32 + 11, SEEK_SET);
			read(fd, &cht, 1);

			if (cht == 0x10)
			{
				dir_cnt += 1;
				printf("\tdirectory\n");
				lseek(fd, DataSector_ + 32 + 16 + 8 + 2, SEEK_SET);
				read(fd, &clus_num, 2);

				address = ((clus_num - 2) * SecPerClus * BytsPerSec) + FirstDataSector;
				spaces += 3;
				function(address);
				spaces -= 3;
			}
			else
			{
				file_cnt += 1;
				printf("\tfile\n");
			}
		}
		else if (ch == 'B')
		{
			offset = 96;
			lseek(fd, DataSector_ + 32, SEEK_SET);

			for(i = 0; i < spaces; i++)
			{
				printf(" ");
			}
			for(i = 0; i < 31; i++)
			{
				read(fd, &c, 1);
				if (isprint(c))
				{
					if (i == 13)
						continue;
					printf("%c", c);
				}
			}
			lseek(fd, DataSector_ + 1, SEEK_SET);
			for(i = 0; i < 31; i++)
			{
				read(fd, &c, 1);
				if (isprint(c))
				{
					if (i == 12)
						continue;
					printf("%c", c);
				}
			}

			lseek(fd, DataSector_ + 64 + 11, SEEK_SET);
			read(fd, &cht, 1);

			if (cht == 0x10)
			{
				dir_cnt += 1;
				printf("\tdirectory\n");
				lseek(fd, DataSector_ + 64 + 16 + 8 + 2, SEEK_SET);
				read(fd, &clus_num, 2);

				address = ((clus_num - 2) * SecPerClus * BytsPerSec) + FirstDataSector;
				spaces += 3;
				function(address);
				spaces -= 3;
			}
			else
			{
				file_cnt += 1;
				printf("\tfile\n");
			}
		}
	}
}

int main(int argc, char *argv[])
{

	if ((fd = open (argv[1], O_RDONLY, S_IRWXU)) < 0)
	{
		perror("open");
	}

	lseek(fd, BPB_BytsPerSec, SEEK_SET);
	read(fd, &BytsPerSec, 2);
	printf("BPB_BytsPerSec=%x\n", BytsPerSec);

	lseek(fd, BPB_SecPerClus, SEEK_SET);
	read(fd, &SecPerClus, 1);
	printf("BPB_SecPerClus=%x\n", SecPerClus);

	lseek(fd, BPB_RsvdSecCnt, SEEK_SET);
	read(fd, &RsvdSecCnt, 2);
	printf("BPB_RsvdSecCnt=%x\n", RsvdSecCnt);

	lseek(fd, BPB_NumFATs, SEEK_SET);
	read(fd, &NumFATs, 1);
	printf("BPB_NumFATs=%x\n", NumFATs);

	lseek(fd, BPB_FATSz32, SEEK_SET);
	read(fd, &FATSz32, 4);
	printf("BPB_FATSz32=%x\n", FATSz32);

	FirstDataSector = (RsvdSecCnt + (NumFATs * FATSz32)) * BytsPerSec;
	printf("FirstDataSector=%x\n", FirstDataSector);

	DataSector = FirstDataSector;
	FatStart = RsvdSecCnt * BytsPerSec;
	printf("FatStart=%x\n", FatStart);

	function(DataSector);

	lseek(fd, FatStart + 8, SEEK_SET);
	read(fd, &correction, 4);
	if ((correction < 0x00ffffff))
	{
		correction = ((correction - 2) * BytsPerSec * SecPerClus) + FirstDataSector;
		function(correction);
	}

	printf("Number of files = %d\n", file_cnt);
	printf("Number of directories = %d\n", dir_cnt);
}
