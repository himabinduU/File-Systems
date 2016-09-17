#include <stdio.h>
#include <fcntl.h>

#define SUPER_BLOCK_OFFSET 0x400

int fd;
unsigned short int file_cnt = 0, dir_cnt = 0;
unsigned int GroupDescOffset = 0, AddrFrstInodeTable, FrstDisketteOffset, FrstDataBlockAddr, InodesPerGroup, BlockSize, InodeSize, BlockNumFrstInodeTable, PtrFrstDataBlock;

void function(unsigned int DataBlockAddr)
{
	unsigned char ch;
	static unsigned short int offset = 0;
	unsigned short int i, add, type = 0, len = 0;
	unsigned int InodeNum, BlockNumInodeTable, AddrInodeTable;

	lseek(fd, DataBlockAddr, SEEK_SET);
	read(fd, &InodeNum, 4);

	while (InodeNum != 0x00)
	{
		DataBlockAddr = DataBlockAddr + offset;
		lseek(fd, DataBlockAddr, SEEK_SET);
		read(fd, &InodeNum, 4);

		lseek(fd, 2, SEEK_CUR);
		read(fd, &len, 1);
		read(fd, &type, 1);
		read(fd, &ch, 1);

		if (len % 4 == 0)
		{
			add = 0;
		}
		else
		{
			add = 1;
		}	

		offset = 4 * ((len / 4) + add + 2);

		if (ch == 0x2e || ch == 0)
		{
			continue;
		}

		lseek(fd, DataBlockAddr + 6 + 2, SEEK_SET);

		for (i = 0; i < len; i++)
		{
			read(fd, &ch, 1);
			printf("%c", ch);
		}

		if (type == 0x02)
		{
			dir_cnt += 1;
			printf("\t\tDirectory\n");

			if (InodeNum >= InodesPerGroup)
			{
				lseek(fd, GroupDescOffset + ((InodeNum / InodesPerGroup) * 32) + 8, SEEK_SET);
				read(fd, &BlockNumInodeTable, 4);
				AddrInodeTable = BlockNumInodeTable * BlockSize;
				lseek(fd, AddrInodeTable + 40, SEEK_SET);
				read(fd, &PtrFrstDataBlock, 4);
				InodeNum = 0;
				DataBlockAddr = PtrFrstDataBlock * BlockSize;
				function(DataBlockAddr);
			}
		}
		else
		{
			file_cnt += 1;
			printf("\t\tFile\n");
		}
	}
}

int main(int argc, char *argv[])
{

	if ((fd = open (argv[1], O_RDONLY, S_IRWXU)) < 0)
	{
		perror("open");
	}

	lseek(fd, SUPER_BLOCK_OFFSET + 24, SEEK_SET);
	read(fd, &BlockSize, 4);
	BlockSize = 1024 << BlockSize;
	printf("BlockSize = %x\n", BlockSize);

	lseek(fd, SUPER_BLOCK_OFFSET + 40, SEEK_SET);
	read(fd, &InodesPerGroup, 4);
	printf("InodesPerGroup = %x\n", InodesPerGroup);

	lseek(fd, SUPER_BLOCK_OFFSET + 88, SEEK_SET);
	read(fd, &InodeSize, 2);
	printf("InodeSize = %x\n", InodeSize);

	if (BlockSize == 0x1000)
	{
		GroupDescOffset = 0x1000;
	}
	else
	{
		GroupDescOffset = SUPER_BLOCK_OFFSET + BlockSize;
	}

	lseek(fd, GroupDescOffset + 8, SEEK_SET);
	read(fd, &BlockNumFrstInodeTable, 4);
	printf("BlockNumFrstInodeTable = %x\n", BlockNumFrstInodeTable);

	AddrFrstInodeTable = BlockNumFrstInodeTable * BlockSize;
	printf("AddrFrstInodeTable = %x\n", AddrFrstInodeTable);

	FrstDisketteOffset = AddrFrstInodeTable + InodeSize;
	printf("FrstDisketteOffset = %x\n", FrstDisketteOffset);

	lseek(fd, FrstDisketteOffset + 40, SEEK_SET);
	read(fd, &PtrFrstDataBlock, 4);
	printf("PtrFrstDataBlock = %x\n", PtrFrstDataBlock);

	FrstDataBlockAddr = PtrFrstDataBlock * BlockSize;
	printf("FrstDataBlockAddr = %x\n\n", FrstDataBlockAddr);

	function(FrstDataBlockAddr);

	printf("Number of files = %d\n", file_cnt);
	printf("Number of Directories = %d\n", dir_cnt);
}
