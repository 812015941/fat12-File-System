#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include "DiskLib.h"
#define MAX_NUM 1024
using namespace std;
/** \brief
�ļ��������
*/
vector<RootEntry> dwHandles;
extern struct RootEntry* rootEntry_ptr;
//extern RootEntry* dwHandles[MAX_NUM];
/** \brief
��ӡһ��Fat12���ļ�������Ϣ
*/
void printRootEntryStruct(RootEntry* rootEntry_ptr);

/** \brief
��ӡ�����ļ�ϵͳ�Ļ�����Ϣ
*/
void printBPB();
/** \brief
��ӡ���е��ļ��������Ŀ¼����������
*/
void printFilesNew(struct RootEntry* rootEntry_ptr);
/** \brief
��ӡ���е���Ŀ¼���ļ�������������
*/
void printChildrenNew(char * directory, int startClus);
/** \brief
��������
*/
int  getFATValueNew(int num);
template<typename T>
/** \brief
�����ã���ȡC���鳤��
*/
int getHandleLength(T &x);
/** \brief
�����ã��������
*/
DWORD createHandle(RootEntry* FileInfo);

/** \brief
�ļ��Ƿ���ڣ���ûд
*/
BOOL isFileExist(char *pszFileName, u16 FstClus);

void printRootEntryStruct(RootEntry* rootEntry_ptr)
{
	cout <<setw(22) << "[debug]DIR_Name:" << setw(14) << rootEntry_ptr->DIR_Name << endl;
	//	cout << setw(22) << "[debug]DIR_Attr:"  <<hex << rootEntry_ptr->DIR_Attr << endl;
	printf("      [debug]DIR_Attr:%x\n", rootEntry_ptr->DIR_Attr);
	//	cout << setw(22) << "[debug]reserved:" <<  setw(14) << rootEntry_ptr->reserved << endl;
	cout << setw(22) << "[debug]DIR_WrtTime:" << setw(14) << rootEntry_ptr->DIR_WrtTime << endl;
	cout << setw(22) << "[debug]DIR_WrtDate:" << setw(14) << rootEntry_ptr->DIR_WrtDate << endl;
	cout << setw(22) << "[debug]DIR_FstClus:" << setw(14) << rootEntry_ptr->DIR_FstClus << endl;
	cout << setw(22) << "[debug]DIR_FileSize:" << setw(14) << rootEntry_ptr->DIR_FileSize << endl;
	cout << setw(22) << "[debug]its FstClus postion (16λ):" << setw(14) <<hex
		<< (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (rootEntry_ptr->DIR_FstClus - 2) * BytsPerSec<< endl;
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
/** \brief
���ļ��������
*/
void fillHandles(int FstClusHJQ = 0x0)
{
	int base;
	if (FstClusHJQ == 0x0)
		base = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;//=9728=0x2600
	else
		base = BytsPerSec * (RsvdSecCnt + FATSz * NumFATs + (RootEntCnt * 32 + BytsPerSec - 1) / BytsPerSec) + (FstClusHJQ - 2) * SecPerClus * BytsPerSec;


	RootEntry *FileInfo_ptr = (RootEntry*)malloc(sizeof(RootEntry));
	for (int i = 0; i < RootEntCnt; i++)
	{
		SetHeaderOffset(base, NULL, FILE_BEGIN);
		ReadFromDisk(FileInfo_ptr, 32, NULL);
		if (FileInfo_ptr->DIR_Name[0] == '\0' || FileInfo_ptr->DIR_Name[0] == '.')
		{
			base += 32;
			continue;

		}
		if ((FileInfo_ptr->DIR_Attr == 0x10 || FileInfo_ptr->DIR_Attr == 0x20 || FileInfo_ptr->DIR_Attr == 0x28) && strlen(FileInfo_ptr->DIR_Name) <= 20)
		{
			//printRootEntryStruct(FileInfo_ptr);
			dwHandles.push_back(*FileInfo_ptr);
		}
		if (FileInfo_ptr->DIR_Attr == 0x10)
		{
			fillHandles(FileInfo_ptr->DIR_FstClus);
		}
		base += 32;
	}
}
/** \brief
Ѱ�ҿմأ����ؿմصĴغţ��������ļ���Ŀ¼��Ϣ��
*/
u16 findEmptyFat()
{
	int offset_fat = RsvdSecCnt*BytsPerSec + 1;//+3�Ǵӵڶ����ؿ�ʼѰ�ң�Ҳ�ǵ�һ����ŵ�fat
	//cout << "[debug]offset_fat: " << offset_fat << " (10 hexadecimal) " << hex << offset_fat << "(16 hexadecimal) in file!" << endl;
	bool _oven = true;//��ʼ��Ϊ2
	u16 result = 2;  //��ʼ��Ϊ2
	while (1)
	{
		if (_oven == true)
		{
			offset_fat += 2;
			SetHeaderOffset(offset_fat, NULL, FILE_BEGIN);
		}
		else
		{
			offset_fat += 1;
			SetHeaderOffset(offset_fat, NULL, FILE_BEGIN);
		}
		u16 bytes;
		u16 * bytes_ptr = &bytes;
		ReadFromDisk(bytes_ptr, 2, NULL);
		if (_oven == true) {
			bytes = bytes << 4;
			bytes = bytes >> 4;
		}
		else {
			bytes = bytes >> 4;
			bytes = bytes << 4;
		}
		//cout << "0x" << hex << bytes << "H" << endl;//�õ��˴�2��Ӧ��ֵ
		if (bytes == 0x0)//����ҵ��˿յ�fat
		{
			break;
		}
		result++;

		_oven = !_oven;
	}
	return result;
}
/** \brief
���ݴ��ݵĴغ�,�ҵ���һ���Ĵغţ�
*/
u16 findNextFat(u16 FstClus)
{
	int offset_fat = RsvdSecCnt*BytsPerSec;//+3�Ǵӵڶ����ؿ�ʼѰ�ң�Ҳ�ǵ�һ����ŵ�fat
											   //cout << "[debug]offset_fat: " << offset_fat << " (10 hexadecimal) " << hex << offset_fat << "(16 hexadecimal) in file!" << endl;
	bool _oven;
	if (FstClus % 2 == 0) {//ż��
		offset_fat += 1.5*FstClus + 1;
		_oven = true;
	}
	else
	{
		offset_fat += 1.5*FstClus + 0.5;
		_oven = false;
	}
	//cout << "[debug]Cluster " << FstClus << "'s offset in fat is " << offset_fat - RsvdSecCnt*BytsPerSec << endl;
	SetHeaderOffset(offset_fat - 1, NULL, FILE_BEGIN);
	u16 bytes;
	u16 * bytes_ptr = &bytes;

	ReadFromDisk(bytes_ptr, 2, NULL);
	if (_oven == true) {
		bytes = bytes << 4;
		bytes = bytes >> 4;
	}
	else {
		bytes = bytes >> 4;
	}
	//cout << "[debug]Cluster " << FstClus << "'s next cluster is 0x" << hex << bytes << "H" << endl;
	return bytes;
}
/** \brief
д��FAT��Ӧ�Ĵء�
*/
void writeFat(u16 FstClus, u16 bytes) 
{
	int offset_fat = RsvdSecCnt*BytsPerSec;
	bool _oven;
	if (FstClus % 2 == 0) {//ż��
		offset_fat += 1.5*FstClus + 1;
		_oven = true;
	}
	else
	{
		offset_fat += 1.5*FstClus + 0.5;
		_oven = false;
	}
	SetHeaderOffset(offset_fat - 1, NULL, FILE_BEGIN);
	u16 bytesOri;
	u16 * bytes_ptrOri = &bytesOri;
	u16 * bytes_ptr = &bytes;
	ReadFromDisk(bytes_ptrOri, 2, NULL);
//	cout << "[debug]writeFat() origin cluster is 0x" << hex << bytes_ptrOri << "H" << endl;
	if (_oven == true) {
		bytesOri = bytesOri >> 12;//ֻ����ǰ��λ
		bytesOri = bytesOri << 12;
		bytes = bytes << 4;
		bytes = bytes >> 4;

	}
	else {
		bytesOri = bytesOri << 12;//ֻ���º���λ
		bytesOri = bytesOri >> 12;
		bytes = bytes >> 4;
		bytes = bytes << 4;
	}
	bytes = bytes | bytesOri;
	SetHeaderOffset(offset_fat - 1, NULL, FILE_BEGIN);
	WriteToDisk(bytes_ptr,2,NULL);
}
/** \brief
���ѡ���Ĵص����ݣ�
FstClus:ָ��Ҫ�����Ĵص�ֵ��
*/
void clearCu(u16 FstClus)
{

	int base;
	base = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2)*BytsPerSec;
	char byte[512] = { 0x0};
	memset(byte, 0x0, sizeof(byte));

	int count= BytsPerSec*SecPerClus/512;//��Ҫ����Ĵ���
	for (int i=0;i<count;i++)
	{
		SetHeaderOffset(base, NULL, FILE_BEGIN);
		WriteToDisk(byte, 512, NULL);
	}

}
void printFilesNew(struct RootEntry* rootEntry_ptr)
{
	int base = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec; //��Ŀ¼���ֽڵ�ƫ����
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
			printChildrenNew(realName, rootEntry_ptr->DIR_FstClus);
		}
	}
}
/** \brief
��ӡ����Fat12���ļ�������Ϣ
*/
void printAllRootEntryStruct(int FstClusHJQ = 0x0)
{

	int base;
	if (FstClusHJQ ==0x0)
		base = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;//=9728=0x2600
	else
		base = BytsPerSec * (RsvdSecCnt + FATSz * NumFATs + (RootEntCnt * 32 + BytsPerSec - 1) / BytsPerSec)+(FstClusHJQ - 2) * SecPerClus * BytsPerSec;

	RootEntry *FileInfo_ptr = (RootEntry*)malloc(sizeof(RootEntry));
	for (int i = 0; i < RootEntCnt; i++)
	{

		SetHeaderOffset(base, NULL, FILE_BEGIN);
		ReadFromDisk(FileInfo_ptr, 32, NULL);
		if (FileInfo_ptr->DIR_Name[0] == '\0'||FileInfo_ptr->DIR_Name[0] == '.')
		{
			base += 32;
			continue;

		}
		if ((FileInfo_ptr->DIR_Attr == 0x10|| FileInfo_ptr->DIR_Attr == 0x20||FileInfo_ptr->DIR_Attr == 0x28)&& strlen(FileInfo_ptr->DIR_Name)<=20)
			printRootEntryStruct(FileInfo_ptr);
		if (FileInfo_ptr->DIR_Attr == 0x10)
		{
			printAllRootEntryStruct(FileInfo_ptr->DIR_FstClus);
		}
		base += 32;
	}
	return;
}
void printChildrenNew(char * directory, int startClus)
{
	//�������ĵ�һ���أ���2�Ŵأ���ƫ���ֽ�
	int dataBase = BytsPerSec * ( RsvdSecCnt + FATSz * NumFATs + (RootEntCnt * 32 + BytsPerSec - 1) / BytsPerSec);
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
		check = ReadFromDisk(content, SecPerClus * BytsPerSec, NULL);
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
int  getFATValueNew(int num)
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
template<typename T>
int getHandleLength(T &x)
{
	int s1 = sizeof(x);
	int s2 = sizeof(x[0]);
	int result = s1 / s2;
	return result;
}
BOOL isFileExist(char *pszFileName, u16 FstClus) {
	char filename[13];
	int dataBase;
	BOOL isExist = FALSE;
	// ������ǰĿ¼������Ŀ
	do {
		int loop;
		if (FstClus == 0) {
			// ��Ŀ¼��ƫ��
			dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
			loop = RootEntCnt;
		}
		else {
			// �������ļ���ַƫ��
			dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec;
			loop = BytsPerSec / 32;
		}
		for (int i = 0; i < loop; i++) {
			SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
			if (ReadFromDisk(rootEntry_ptr, 32, NULL) != 0) {
				// Ŀ¼0x10���ļ�0x20�����0x28
				int len_of_filename = 0;
				if (rootEntry_ptr->DIR_Attr == 0x20) {
					for (int j = 0; j < 11; j++) {
						if (rootEntry_ptr->DIR_Name[j] != ' ') {
							filename[len_of_filename++] = rootEntry_ptr->DIR_Name[j];
						}
						else {
							filename[len_of_filename++] = '.';
							while (rootEntry_ptr->DIR_Name[j] == ' ') j++;
							j--;
						}
					}
					filename[len_of_filename] = '\0';
					// ���Դ�Сд�Ƚ�
					if (_stricmp(filename, pszFileName) == 0) {
						isExist = TRUE;
						break;
					}
				}
			}
			dataBase += 32;
		}
		if (isExist) break;
	} while ((FstClus = findNextFat(FstClus)) != 0xFFF && FstClus != 0);
	return isExist;
}
/** \brief
�ж�Ŀ¼�Ƿ����
*/
u16 isDirectoryExist(char *FolderName, u16 FstClus) {
	char directory[12];
	int dataBase;
	u16 isExist = 0;
	// ������ǰĿ¼������Ŀ
	do {
		if (FstClus == 0) {
			// ��Ŀ¼��ƫ��
			dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
		}
		else {
			// �������ļ���ַƫ��
			dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec;
		}
		for (int i = 0; i < RootEntCnt; i++) {
			SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
			if (ReadFromDisk(rootEntry_ptr, 32, NULL) != 0) {
				// Ŀ¼0x10���ļ�0x20�����0x28
				if (rootEntry_ptr->DIR_Attr == 0x10) {
					for (int j = 0; j < 11; j++) {
						if (rootEntry_ptr->DIR_Name[j] != ' ') {
							directory[j] = rootEntry_ptr->DIR_Name[j];
							if (j == 10) {
								directory[11] = '\0';
								break;
							}
						}
						else {
							directory[j] = '\0';
							break;
						}
					}
					// ���Դ�Сд�Ƚ�
					if (_stricmp(directory, FolderName) == 0) {
						isExist = rootEntry_ptr->DIR_FstClus;
						break;
					}
				}
			}
			dataBase += 32;
		}
		if (isExist) break;
	} while ((FstClus = findNextFat(FstClus)) != 0xFFF && FstClus != 0);
	return isExist;
}
/** \brief
�ж�·���Ƿ����
*/
u16 isPathExist(char *pszFolderPath) {
	char directory[12]; // ���Ŀ¼��
	u16 FstClus = 0;
	/* ��3��ʼ�������̷�C:\\ */
	int i = 3, len = 0;
	while (pszFolderPath[i] != '\0') {
		if (pszFolderPath[i] == '\\') {
			directory[len] = '\0';
			//cout << directory << endl;
			if (FstClus = isDirectoryExist(directory, FstClus)) {
				len = 0;
			}
			else {
				len = 0;
				break;
			}
			i++;
		}
		else {
			directory[len++] = pszFolderPath[i++];
		}
	}
	if (len > 0) {
		directory[len] = '\0';
		//cout << directory << endl;
		FstClus = isDirectoryExist(directory, FstClus);
	}
	return FstClus;
}
char* getPathName(char *pszFolderPath)
{
	int i = 3;
	int len = 0;
	char directory[12]; // ���Ŀ¼��
	while (pszFolderPath[i] != '\0') {//ֱ��ĩβ
		if (pszFolderPath[i] != '\\')
		{
			directory[len++] = pszFolderPath[i++];
		}
		else break;//��ʱ�õ���directory
	}
	directory[len] = '\0';
	return directory;
}
DWORD createHandle(RootEntry* FileInfo) {
	int i;
	dwHandles.push_back(*FileInfo);
	i = dwHandles.size();
	return i;
}
/** \brief
10��������ת��Ϊ16����string
*/

string DecIntToHexStr(int num)
{
	string str;
	int Temp = num / 16;
	int left = num % 16;
	if (Temp > 0)
		str += DecIntToHexStr(Temp);
	if (left < 10)
		str += (left + '0');
	else
		str += ('A' + left - 10);
	return str;
}
/** \brief
16�����ַ���char*ת��Ϊ10����string
*/
void updateRootEntry(RootEntry *FileInfo_ptr)
{
	int dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;//д��fat
	for (int i = 0; i < RootEntCnt; i++)
	{
		SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
		ReadFromDisk(rootEntry_ptr, 32, NULL);
		dataBase += 32;
		if (strcmp(rootEntry_ptr->DIR_Name, FileInfo_ptr->DIR_Name) == 0)
		{
			SetHeaderOffset(dataBase - 32, NULL, FILE_BEGIN);
			WriteToDisk(FileInfo_ptr, 32, NULL);
			return;
		}
	}

}
void initFileInfo(RootEntry* FileInfo_ptr, char* FileName, u8 FileAttr, u32 FileSize, u16 FstClus) {
	FileInfo_ptr->DIR_Attr = FileAttr;
	memset(FileInfo_ptr->DIR_Name, 0, sizeof(FileInfo_ptr->DIR_Name));
	fillTime(FileInfo_ptr->DIR_WrtDate, FileInfo_ptr->DIR_WrtTime);
	if (FileAttr == 0x10) {
		FileInfo_ptr->DIR_FileSize = BytsPerSec;
		strcpy(FileInfo_ptr->DIR_Name, FileName);
	}
	else {
		FileInfo_ptr->DIR_FileSize = FileSize;
		int i = 0;
		while (FileName[i] != '\0') {
			if (FileName[i] == '.') {
				int j = i;
				while (j < 8) {
					FileInfo_ptr->DIR_Name[j] = 0x20;
					j++;
				}
				i++;
				break;
			}
			else {
				FileInfo_ptr->DIR_Name[i] = FileName[i];
				i++;
			}

		}
		memcpy(&FileInfo_ptr->DIR_Name[8], &FileName[i], 3);
	}
	int clusNum;
	if ((FileSize % BytsPerSec) == 0 && FileSize != 0) {
		clusNum = FileSize / BytsPerSec;
	}
	else {
		clusNum = FileSize / BytsPerSec + 1;
	}
	FileInfo_ptr->DIR_FstClus = FstClus;
}
#endif // UTIL_H_INCLUDED
