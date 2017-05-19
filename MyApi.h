#ifndef MYAPI_H_INCLUDED
#define MYAPI_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>
#include <sstream>
#include <iostream>

//#define OLDVERSION
using namespace std;
typedef unsigned char u8;   //1�ֽ�
typedef unsigned short u16; //2�ֽ�
typedef unsigned int u32;   //4�ֽ�
extern int  BytsPerSec;    //ÿ�����ֽ���
extern int  SecPerClus;    //ÿ��������
extern int  RsvdSecCnt;    //Boot��¼ռ�õ�������
extern int  NumFATs;   //FAT�����
extern int  RootEntCnt;    //��Ŀ¼����ļ���
extern int  FATSz;
#pragma pack (1) /*ָ����1�ֽڶ���*/
struct BPB {
    u16  BPB_BytsPerSec;    //ÿ�����ֽ���
    u8   BPB_SecPerClus;    //ÿ��������
    u16  BPB_RsvdSecCnt;    //Boot��¼ռ�õ�������
    u8   BPB_NumFATs=2;   //FAT�����
    u16  BPB_RootEntCnt;    //��Ŀ¼����ļ���
    u16  BPB_TotSec16;
    u8   BPB_Media;
    u16  BPB_FATSz16=9;   //FAT������
    u16  BPB_SecPerTrk;
    u16  BPB_NumHeads;
    u32  BPB_HiddSec;
    u32  BPB_TotSec32;  //���BPB_FATSz16Ϊ0����ֵΪFAT������
};
struct RootEntry {
    char DIR_Name[11]; //11byte
    u8   DIR_Attr;      //�ļ����� 1byte
    char reserved[10]; //10byte
    u16  DIR_WrtTime;//д�롢�޸�ʱ��
    u16  DIR_WrtDate;//
    u16  DIR_FstClus;   //��ʼ�غ�
    u32  DIR_FileSize;
};
/**
 Ҫ����ָ��Ŀ¼�´���ָ���ļ���������ļ������ڵĻ������Ŀ¼�����ڻ��ļ��Ѵ��ڣ�����ʧ�ܷ���0��
����ɹ��򷵻�һ����ʾ���ļ��ı�ʶ��������Window�ľ�����ڲ����ݽṹ��ӳ�䷽����������
pszFolderPath��Ŀ¼·������"C:\\Test\\Test01"�ȵ�
pszFileName���ļ�������"Test.txt"�ȵ�
 */
DWORD MyCreateFile(char *pszFolderPath, char *pszFileName);
/** \brief
Ҫ�󣺴�ָ��Ŀ¼�µ�ָ���ļ������Ŀ¼�����ڻ����ļ������ڣ��򷵻�0��ʾʧ�ܣ�
����ɹ��򷵻�һ����ʾ���ļ��ı�ʶ��������Window�ľ�����ڲ����ݽṹ��ӳ�䷽����������
pszFolderPath��Ŀ¼·������"C:\\Test\\Test01"�ȵ�
pszFileName���ļ�������"Test.txt"�ȵ�
 */
DWORD MyOpenFile(char *pszFolderPath, char *pszFileName);
/** \brief
 *
Ҫ�󣺹رո��ļ���
���������
dwHandle�������ʶ���ļ��ľ��������MyCreateFile���ص��Ǹ�
 *
 */
void MyCloseFile(DWORD dwHandle);
/** \brief
 *
 Ҫ��ɾ��ָ��Ŀ¼�µ�ָ���ļ������Ŀ¼�����ڻ����ļ������ڣ��򷵻�0��ʾʧ�ܣ����򷵻�TRUE��ʾ�ɹ���
���������
pszFolderPath��Ŀ¼·������"C:\\Test\\Test01"�ȵ�
pszFileName���ļ�������"Test.txt"�ȵ�
 *
 */
void MyDeleteFile(char *pszFolderPath, char *pszFileName);
/** \brief
 *
Ҫ�󣺽�pBuffer��dwBytesToWrite���ȵ�����д��ָ���ļ����ļ�ָ��λ�á�
���������
dwHandle��MyOpenFile���ص�ֵ���������������ԭ�ⲻ���Ĵ����㣬���ڲ����ݽṹ����������
pBuffer��ָ���д�����ݵĻ�����
dwBytesToWrite����д�����ݵĳ���
����ֵ���ɹ�д��ĳ��ȣ�-1��ʾʧ�ܡ�
 *
 */
DWORD MyWriteFile(DWORD dwHandle, LPVOID pBuffer, DWORD dwBytesToWrite);

/** \brief
 *
 Ҫ�󣺶�ȡָ���ļ��С�ָ�����ȵ����ݵ�����Ļ�������
���������
dwHandle��MyOpenFile���ص�ֵ���������������ԭ�ⲻ���Ĵ����㣬���ڲ����ݽṹ����������
pBuffer��ָ��������ݵĻ�����
dwBytesToRead������ȡ���ݵĳ���
����ֵ���ɹ���ȡ�ĳ��ȣ�-1��ʾʧ�ܡ�
 *
 */
DWORD MyReadFile(DWORD dwHandle, LPVOID pBuffer, DWORD dwBytesToRead);
/** \brief
 *
Ҫ����ָ��·���£�����ָ�����Ƶ��ļ��С����Ŀ¼�����ڻ���������ļ����Ѵ��ڣ��򷵻�FALSE��
�����ɹ�����TRUE��
���������
pszFolderPath��Ŀ¼·������"C:\\Test\\Test01"�ȵ�
pszFolderName���ļ������ƣ���"MyFolder"�ȵ�
����ֵ�����Ŀ¼�����ڻ���������ļ����Ѵ��ڣ��򷵻�FALSE�������ɹ�����TRUE��
 *
 */

BOOL MyCreateDirectory(char *pszFolderPath, char *pszFolderName);
/** \brief
 *
 Ҫ����ָ��·���£�ɾ��ָ�����Ƶ��ļ��С����Ŀ¼�����ڻ���������ļ��в����ڣ��򷵻�FALSE��
ɾ���ɹ�����TRUE��
���������
pszFolderPath��Ŀ¼·������"C:\\Test\\Test01"�ȵ�
pszFolderName���ļ������ƣ���"MyFolder"�ȵ�
����ֵ�����Ŀ¼�����ڻ���������ļ��в����ڣ��򷵻�FALSE��ɾ���ɹ�����TRUE��
 *
 */

BOOL MyDeleteDirectory(char *pszFolderPath, char *pszFolderName);
/** \brief
 *
��;���ƶ�ָ���ļ����ļ�ͷ����д��ͬλ�á�����ļ���������ڣ�����FALSE�����򷵻�TRUE
dwHandle��MyOpenFile���ص�ֵ���������������ԭ�ⲻ���Ĵ����㣬���ڲ����ݽṹ����������
nOffset��32λƫ�����������ɸ���Ϊ�㡣
dwMoveMethod��ƫ�Ƶ���ʼλ�ã��������������Ϳ�ѡ��
MY_FILE_BEGIN����ͷ����ʼ����ƫ��
MY_FILE_CURRENT���ӵ�ǰ��ͷλ�ÿ�ʼ�������
MY_FILE_END����ĩβ��ʼ����ƫ��
 *
 */

BOOL MySetFilePointer(DWORD dwFileHandle, int nOffset, DWORD dwMoveMethod);
#endif // MYAPI_H_INCLUDED
