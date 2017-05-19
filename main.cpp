
#include "DiskLib.h"
#include "MyApi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#pragma comment(lib,"DiskLib.lib")
using namespace std;

#ifdef OLDVERSION

FILE* fat12 = fopen("floppy.img", "rb");//��佫�ᱻ�滻
#endif // OLDVERSION
void printFilesNew(struct RootEntry* rootEntry_ptr);//��util��ʵ�ֵĺ���
struct BPB bpb;
struct BPB* bpb_ptr = &bpb;
struct RootEntry rootEntry;
struct RootEntry* rootEntry_ptr = &rootEntry;
const char* fs = "floppy.img";
int  getFATValue(FILE * fat12, int num);
void printFiles(FILE * fat12, struct RootEntry* rootEntry_ptr);  //��ӡ�ļ�������������ڴ�ӡĿ¼ʱ����������printChildren
void fillBPB(FILE * fat12, struct BPB* bpb_ptr);
void printChildren(FILE * fat12, char * directory, int startClus);  //��ӡĿ¼��Ŀ¼�����ļ���

int main()
{
	
	StartupDisk(fs);
	SetHeaderOffset(11, NULL, FILE_BEGIN);
	if (ReadFromDisk(bpb_ptr, 25, NULL) != 0) {
		BytsPerSec = bpb_ptr->BPB_BytsPerSec;
		SecPerClus = bpb_ptr->BPB_SecPerClus;
		RsvdSecCnt = bpb_ptr->BPB_RsvdSecCnt;
		NumFATs = bpb_ptr->BPB_NumFATs;
		RootEntCnt = bpb_ptr->BPB_RootEntCnt;
		if (bpb_ptr->BPB_FATSz16 != 0)
		{
			FATSz = bpb_ptr->BPB_FATSz16;
		}
		else
		{
			FATSz = bpb_ptr->BPB_TotSec32;
		}
	}
	else {
		cout << "[debug]:Read BPB fail!" << endl;
	}
	
	MyCreateFile("", "t32.txt");
	printFilesNew( rootEntry_ptr);
#ifdef OLDVERSION
	//����fat12�ļ���BPB.fat12������11~25�ֽڴ����BPB�е����ݣ����磺�����ÿ�����ֽ���=13
	fillBPB(fat12, bpb_ptr);
	//��ʼ������ȫ�ֱ���

	if (bpb_ptr->BPB_FATSz16 != 0)
	{
		FATSz = bpb_ptr->BPB_FATSz16;
	}
	else
	{
		FATSz = bpb_ptr->BPB_TotSec32;
	}

	//	struct RootEntry rootEntry;
	//	struct RootEntry* rootEntry_ptr = &rootEntry;
	MyCreateFile("", "tyc.txt");
	//��ӡ�ļ���
	printFiles(fat12, rootEntry_ptr);
	//	cout << "[output]" << bpb_ptr->BPB_FATSz16 << endl;
	//	cout << "[output]" << bpb_ptr->BPB_TotSec16 << endl;
	//	cout << "[output]" << bpb_ptr->BPB_TotSec32 << endl;
#endif // OLDVERSION
}

void fillBPB(FILE* fat12, struct BPB* bpb_ptr)
{
	int check;

	//BPB��ƫ��11���ֽڴ���ʼ
	check = fseek(fat12, 11, SEEK_SET); //����д������ָ���ƶ���ָ��λ��
	if (check == -1)
		printf("fseek in fillBPB failed!");

	//BPB����Ϊ25�ֽ�
	check = fread(bpb_ptr, 1, 25, fat12); //��ȡ25��1�ֽڵ�bpb_ptr
	if (check != 25)
		printf("fread in fillBPB failed!");
}

int  getFATValue(FILE * fat12, int num)   //NUM=��ʼ�غ�DIR_FstClus
{
	//FAT1��ƫ���ֽ�
	int fatBase = RsvdSecCnt * BytsPerSec;
	//FAT���ƫ���ֽ�
	int fatPos = fatBase + num * 3 / 2;
	//��żFAT���ʽ��ͬ��������д�����0��FAT�ʼ
	int type = 0;
	if (num % 2 == 0)
	{
		type = 0;
	}
	else
	{
		type = 1;
	}

	//�ȶ���FAT�����ڵ������ֽ�
	u16 bytes;
	u16* bytes_ptr = &bytes;
	int check;
	check = fseek(fat12, fatPos, SEEK_SET);
	if (check == -1)
		printf("fseek in getFATValue failed!");

	check = fread(bytes_ptr, 1, 2, fat12);
	if (check != 2)
		printf("fread in getFATValue failed!");

	//u16Ϊshort����ϴ洢��Сβ˳���FAT��ṹ���Եõ�
	//typeΪ0�Ļ���ȡbyte2�ĵ�4λ��byte1���ɵ�ֵ��typeΪ1�Ļ���ȡbyte2��byte1�ĸ�4λ���ɵ�ֵ
	if (type == 0)
	{
		return bytes << 4;
	}
	else
	{
		return bytes >> 4;
	}
}
void printChildren(FILE * fat12, char * directory, int startClus)
{
	//�������ĵ�һ���أ���2�Ŵأ���ƫ���ֽ�
	int dataBase = BytsPerSec * (RsvdSecCnt + FATSz * NumFATs + (RootEntCnt * 32 + BytsPerSec - 1) / BytsPerSec);
	char fullName[24];  //����ļ�·����ȫ��
	int strLength = strlen(directory);
	strcpy(fullName, directory);
	fullName[strLength] = '/';
	strLength++;
	fullName[strLength] = '\0';
	char* fileName = &fullName[strLength];

	int currentClus = startClus;
	int value = 0;//value��ʮ���M�Ɣ���ÿ�δ惦2/4���ֹ�
	int ifOnlyDirectory = 0;
	while (value < 0xFF8)
	{
		value = getFATValue(fat12, currentClus);
		if (value == 0xFF7)
		{
			printf("bad cluster!Reading fails!\n");
			break;
		}

		char* str = (char*)malloc(SecPerClus * BytsPerSec); //�ݴ�Ӵ��ж���������
		char* content = str;

		int startByte = dataBase + (currentClus - 2) * SecPerClus * BytsPerSec;
		int check;
		check = fseek(fat12, startByte, SEEK_SET);
		if (check == -1)
			printf("fseek in printChildren failed!");

		check = fread(content, 1, SecPerClus * BytsPerSec, fat12);
		if (check != SecPerClus * BytsPerSec)
			printf("fread in printChildren failed!");

		//����content�е�����,���δ��������Ŀ,Ŀ¼��ÿ����Ŀ�ṹ���Ŀ¼�µ�Ŀ¼�ṹ��ͬ
		int count = SecPerClus * BytsPerSec; //ÿ�ص��ֽ���
		int loop = 0;
		while (loop < count)
		{
			int i;
			char tempName[12];  //�ݴ��滻�ո�Ϊ�����ļ���
			if (content[loop] == '\0')
			{
				loop += 32;
				continue;
			}   //����Ŀ�����
				//���˷�Ŀ���ļ�
			int j;
			int boolean = 0;
			for (j = loop; j < loop + 11; j++)
			{
				if (!(((content[j] >= 48) && (content[j] <= 57)) ||
					((content[j] >= 65) && (content[j] <= 90)) ||
					((content[j] >= 97) && (content[j] <= 122)) ||
					(content[j] == ' ')))
				{
					boolean = 1;    //��Ӣ�ļ����֡��ո�
					break;
				}
			}
			if (boolean == 1)
			{
				loop += 32;
				continue;
			}   //��Ŀ���ļ������
			int k;
			int tempLong = -1;
			for (k = 0; k < 11; k++)
			{
				if (content[loop + k] != ' ')
				{
					tempLong++;
					tempName[tempLong] = content[loop + k];
				}
				else
				{
					tempLong++;
					tempName[tempLong] = '.';
					while (content[loop + k] == ' ') k++;
					k--;
				}
			}
			tempLong++;
			tempName[tempLong] = '\0';  //����Ϊֹ�����ļ�����ȡ�����ŵ�tempName��

			strcpy(fileName, tempName);
			printf("%s\n", fullName);
			ifOnlyDirectory = 1;
			loop += 32;
		}

		free(str);

		currentClus = value;
	};

	if (ifOnlyDirectory == 0)
		printf("%s\n", fullName);  //��Ŀ¼������£����Ŀ¼
}
void printFiles(FILE * fat12, struct RootEntry* rootEntry_ptr)
{
	int base = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec; //��Ŀ¼���ֽڵ�ƫ����
	int check;
	char realName[12];  //�ݴ潫�ո��滻�ɵ����ļ���

						//���δ����Ŀ¼�еĸ�����Ŀ
	int i;
	for (i = 0; i < RootEntCnt; i++)
	{

		check = fseek(fat12, base, SEEK_SET);
		if (check == -1)
			printf("fseek in printFiles failed!");

		check = fread(rootEntry_ptr, 1, 32, fat12);
		if (check != 32)
			printf("fread in printFiles failed!");

		base += 32;

		if (rootEntry_ptr->DIR_Name[0] == '\0') continue;     //����Ŀ�����

															  //���˷�Ŀ���ļ�
		int j;
		int boolean = 0;
		for (j = 0; j < 11; j++)
		{
			if (!(((rootEntry_ptr->DIR_Name[j] >= 48) && (rootEntry_ptr->DIR_Name[j] <= 57)) ||
				((rootEntry_ptr->DIR_Name[j] >= 65) && (rootEntry_ptr->DIR_Name[j] <= 90)) ||
				((rootEntry_ptr->DIR_Name[j] >= 97) && (rootEntry_ptr->DIR_Name[j] <= 122)) ||
				(rootEntry_ptr->DIR_Name[j] == ' ')))
			{
				boolean = 1;    //��Ӣ�ļ����֡��ո�
				break;
			}
		}
		if (boolean == 1) continue;  //��Ŀ���ļ������

		int k;
		if ((rootEntry_ptr->DIR_Attr & 0x10) == 0)
		{
			//�ļ�
			int tempLong = -1;
			for (k = 0; k < 11; k++)
			{
				if (rootEntry_ptr->DIR_Name[k] != ' ')
				{
					tempLong++;
					realName[tempLong] = rootEntry_ptr->DIR_Name[k];
				}
				else
				{
					tempLong++;
					realName[tempLong] = '.';
					while (rootEntry_ptr->DIR_Name[k] == ' ') k++;
					k--;
				}
			}
			tempLong++;
			realName[tempLong] = '\0';  //����Ϊֹ�����ļ�����ȡ�����ŵ���realName��

										//����ļ�
			printf("%s\n", realName);
		}
		else
		{
			//Ŀ¼
			int tempLong = -1;
			for (k = 0; k < 11; k++)
			{
				if (rootEntry_ptr->DIR_Name[k] != ' ')
				{
					tempLong++;
					realName[tempLong] = rootEntry_ptr->DIR_Name[k];
				}
				else
				{
					tempLong++;
					realName[tempLong] = '\0';
					break;
				}
			}   //����Ϊֹ����Ŀ¼����ȡ�����ŵ���realName

				//���Ŀ¼�����ļ�
			printChildren(fat12, realName, rootEntry_ptr->DIR_FstClus);
		}
	}
}
