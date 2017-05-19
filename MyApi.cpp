#include "MyApi.h"
#include "util.h"
#include "DiskLib.h"
#ifdef OLDVERSION


extern FILE* fat12;//��佫�ᱻ�滻
#endif // OLDVERSION

RootEntry* dwHandles[30] = { NULL };//�ļ������
extern struct BPB* bpb_ptr;
extern struct RootEntry* rootEntry_ptr;
//���涼Ϊ�̶�ֵ��
int  BytsPerSec;    //ÿ�����ֽ���
int  SecPerClus;    //ÿ��������
int  RsvdSecCnt;    //Boot��¼ռ�õ�������
int  NumFATs;   //FAT�����
int  RootEntCnt;    //��Ŀ¼����ļ���
int  FATSz; //FAT������
DWORD MyCreateFile(char *pszFolderPath, char *pszFileName)
{
	DWORD FileHandle = 0;
	printBPB();
	cout << "[output]trying to create file named " << pszFileName;
	if (strcmp(pszFolderPath, "") != 0)
		cout << " in folder " << pszFolderPath << endl;
	else cout << endl;
	//�ʂ䌤��
	int base = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
	cout << "[output]constant base:" << base << endl;
	int check;
	char realName[12];  //�ݴ潫�ո��滻�ɵ����ļ���
	//���δ����Ŀ¼�еĸ�����Ŀ
	int i;
	int max_FstClus = 0;//����Ѱ�������״�FstClus,���ļ��������֮��
	int max_FinalClus = 0;//��ֹ��=FstClus+m
	for (i = 0; i < RootEntCnt; i++)
	{
		
		SetHeaderOffset(base, NULL, FILE_BEGIN);
		ReadFromDisk(rootEntry_ptr, 32, NULL);
		printRootEntryStruct(rootEntry_ptr);
		if (max_FstClus < rootEntry_ptr->DIR_FstClus)
		{
			max_FstClus = rootEntry_ptr->DIR_FstClus;
			u32 cuSize = SecPerClus*BytsPerSec; //512
			u16 needCu = (rootEntry_ptr->DIR_FileSize + cuSize - 1) / cuSize;
			max_FinalClus = max_FstClus + needCu;
		}
		base += 32;
		//����Ҫ���ˣ�
		
		if (strcmp(pszFileName, rootEntry_ptr->DIR_Name)== 0)//�Ѿ����ڣ�������
			return 0;
		if (rootEntry_ptr->DIR_Name[0] == '\0') //continue;     //�ǿ���Ŀ��������д���ļ�
			//����Ŀ������д���ļ�

		{
			RootEntry *FileInfo_ptr = (RootEntry*)malloc(sizeof(RootEntry));
			cout << "[output]into loop2" << endl;
			//rootEntry_ptr->DIR_Name=pszFileName;
			strcpy(FileInfo_ptr->DIR_Name, pszFileName);
			for (int j = sizeof(pszFileName);j<11;j++)
			{
				FileInfo_ptr->DIR_Name[j] = pszFileName[j];
				//memcpy(&FileInfo_ptr->DIR_Name[j], &pszFileName[j],1);
			}
			
			//memcpy(&FileInfo_ptr->DIR_Name[8],&pszFileName[i],3);
			FileInfo_ptr->DIR_Attr = 0x20;
			fillTime(FileInfo_ptr->DIR_WrtDate, FileInfo_ptr->DIR_WrtTime);
			FileInfo_ptr->DIR_FileSize = 10000;//����ģ����޸�
			u32 cuSize = SecPerClus*BytsPerSec; //512
			u16 needCu = (FileInfo_ptr->DIR_FileSize + cuSize - 1) / cuSize;//����ȡ���������Ҫ�Ĵصĸ���
// TODO (812015941#1#): ��ҪѰ�ҷ����λ��
			FileInfo_ptr->DIR_FstClus = max_FinalClus +1;//�������ļ�ռ�õ����һ����+1�����ǵ�ǰ���ļ����״أ�
			cout << "[output]constant cuSize :" << cuSize << endl;
			cout << "[output]constant needCu :" << needCu << endl;
			cout << "[output]constant input file name:" << FileInfo_ptr->DIR_Name << endl;
			cout << "[output]constant input FileSize:" << FileInfo_ptr->DIR_FileSize << endl;
			printf("[output]constant input file attr:%x\n", FileInfo_ptr->DIR_Attr);
			cout << "[output]constant input file date:" << FileInfo_ptr->DIR_WrtDate << endl;
			cout << "[output]constant input file time:" << FileInfo_ptr->DIR_WrtTime << endl;
			cout << "[output]constant input file firstClus:" << FileInfo_ptr->DIR_FstClus << endl;
			// TODO (812015941#1#): д�������
			SetHeaderOffset(base-32, NULL, FILE_BEGIN);
			if (WriteToDisk(FileInfo_ptr, 32, NULL))
			{
				FileHandle=createHandle(FileInfo_ptr);
				break;
			}

			else cout << "[debug]Write to disk fail!" << endl;
		}

	}
	return FileHandle;
}

/** \brief
Ҫ�󣺴�ָ��Ŀ¼�µ�ָ���ļ������Ŀ¼�����ڻ����ļ������ڣ��򷵻�0��ʾʧ�ܣ�
����ɹ��򷵻�һ����ʾ���ļ��ı�ʶ��������Window�ľ�����ڲ����ݽṹ��ӳ�䷽����������
pszFolderPath��Ŀ¼·������"C:\\Test\\Test01"�ȵ�
pszFileName���ļ�������"Test.txt"�ȵ�
*/
DWORD MyOpenFile(char *pszFolderPath, char *pszFileName)
{
	return 0;
}