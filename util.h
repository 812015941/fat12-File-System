#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <iostream>
#include <iomanip>
#include "DiskLib.h"
using namespace std;
extern struct RootEntry* rootEntry_ptr;
extern RootEntry* dwHandles[30];
void printRootEntryStruct(RootEntry* rootEntry_ptr);
void printBPB();
void printFilesNew(struct RootEntry* rootEntry_ptr);
void printChildrenNew(char * directory, int startClus);
int  getFATValueNew(int num);
DWORD createHandle(RootEntry* FileInfo);
BOOL isFileExist(char *pszFileName, u16 FstClus);
void printRootEntryStruct(RootEntry* rootEntry_ptr)
{
	cout << setw(22) << "[debug]DIR_Name:" << setw(14) << rootEntry_ptr->DIR_Name << endl;
	//	cout << setw(22) << "[debug]DIR_Attr:"  <<hex << rootEntry_ptr->DIR_Attr << endl;
	printf("      [debug]DIR_Attr:%x\n", rootEntry_ptr->DIR_Attr);
	//	cout << setw(22) << "[debug]reserved:" <<  setw(14) << rootEntry_ptr->reserved << endl;
	cout << setw(22) << "[debug]DIR_WrtTime:" << setw(14) << rootEntry_ptr->DIR_WrtTime << endl;
	cout << setw(22) << "[debug]DIR_WrtDate:" << setw(14) << rootEntry_ptr->DIR_WrtDate << endl;
	cout << setw(22) << "[debug]DIR_FstClus:" << setw(14) << rootEntry_ptr->DIR_FstClus << endl;
	cout << setw(22) << "[debug]DIR_FileSize:" << setw(14) << rootEntry_ptr->DIR_FileSize << endl;
	cout << setw(22) << "------------end-------------------" << endl;
}

void printBPB()
{
	cout << setw(22) << "[debug]BytsPerSec:" << setw(14) << BytsPerSec << endl;
	cout << setw(22) << "[debug]SecPerClus:" << setw(14) << SecPerClus << endl;
	cout << setw(22) << "[debug]RsvdSecCnt:" << setw(14) << RsvdSecCnt << endl;
	cout << setw(22) << "[debug]NumFATs:" << setw(14) << NumFATs << endl;
	cout << setw(22) << "[debug]RootEntCnt:" << setw(14) << RootEntCnt << endl;
	cout << setw(22) << "[debug]FATSz:" << setw(14) << FATSz << endl;
	cout << setw(22) << "------------end-------------------" << endl;
}
void fillTime(u16 &DIR_WrtDate, u16 &DIR_WrtTime)
{
	time_t tt = time(NULL);//��䷵�ص�ֻ��һ��ʱ��cuo
	tm* t = localtime(&tt);
	stringstream stream;
	stream << t->tm_year - 100 << t->tm_mon + 1 << t->tm_mday;
	string tmp = stream.str();
	DIR_WrtDate = atoi(tmp.c_str());
	stream.str("");
	stream << t->tm_hour << t->tm_min << t->tm_sec;
	tmp = stream.str();
	DIR_WrtTime = atoi(tmp.c_str());
}




void printFilesNew(struct RootEntry* rootEntry_ptr)
{
	int base = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec; //��Ŀ¼���ֽڵ�ƫ����
	int check;
	char realName[12];  //�ݴ潫�ո��滻�ɵ����ļ���

						//���δ����Ŀ¼�еĸ�����Ŀ
	int i;
	for (i = 0; i < RootEntCnt; i++)
	{

		SetHeaderOffset(base, NULL, FILE_BEGIN);
		ReadFromDisk(rootEntry_ptr, 32, NULL);
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
			printChildrenNew( realName, rootEntry_ptr->DIR_FstClus);
		}
	}
}

void printChildrenNew(char * directory, int startClus)
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
		value = getFATValueNew(currentClus);
		if (value == 0xFF7)
		{
			printf("bad cluster!Reading fails!\n");
			break;
		}

		char* str = (char*)malloc(SecPerClus * BytsPerSec); //�ݴ�Ӵ��ж���������
		char* content = str;

		int startByte = dataBase + (currentClus - 2) * SecPerClus * BytsPerSec;
		int check;
		//check = fseek(fat12, startByte, SEEK_SET);
		//if (check == -1)
		//	printf("fseek in printChildren failed!");

		//check = fread(content, 1, SecPerClus * BytsPerSec, fat12);
		//if (check != SecPerClus * BytsPerSec)
		//	printf("fread in printChildren failed!");
		SetHeaderOffset(startByte, NULL, FILE_BEGIN);
		check=ReadFromDisk(content, SecPerClus * BytsPerSec, NULL);
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
int  getFATValueNew( int num)
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
	//check = fseek(fat12, fatPos, SEEK_SET);
	//if (check == -1)
	//	printf("fseek in getFATValue failed!");

	//check = fread(bytes_ptr, 1, 2, fat12);
	//if (check != 2)
	//	printf("fread in getFATValue failed!");
	SetHeaderOffset(fatPos, NULL, FILE_BEGIN);
	check = ReadFromDisk(bytes_ptr, 2, NULL);
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

BOOL isFileExist(char *pszFileName, u16 FstClus)
{


	return true;
}
DWORD createHandle(RootEntry* FileInfo) {
	int i;
	for (i = 1; i < 30; i++) {
		if (dwHandles[i] == NULL) {
			dwHandles[i] = FileInfo;
			break;
		}
	}
	return i;
}


#endif // UTIL_H_INCLUDED
