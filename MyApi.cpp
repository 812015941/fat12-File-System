#include "MyApi.h"
#include "util.h"
#include "DiskLib.h"
#ifdef OLDVERSION


extern FILE* fat12;//��佫�ᱻ�滻
#endif // OLDVERSION
//RootEntry* dwHandles[MAX_NUM] = { NULL };//�ļ������
extern struct BPB* bpb_ptr;
extern struct RootEntry* rootEntry_ptr;
//���涼Ϊ�̶�ֵ��
int  BytsPerSec;    //ÿ�����ֽ���
int  SecPerClus;    //ÿ��������
int  RsvdSecCnt;    //Boot��¼ռ�õ�������
int  NumFATs;   //FAT�����
int  RootEntCnt;    //��Ŀ¼����ļ���
int  FATSz; //FAT������
BOOL MyCreateDirectory(char *pszFolderPath, char *pszFolderName)
{
	u16 FstClus = findEmptyFat();
	cout << "[debug]findEmptyFat():" << FstClus << endl;
	DWORD FileHandle = 0;
	cout << "[output]trying to create Directory named " << pszFolderName;
	if (strcmp(pszFolderPath, "") != 0)
		cout << " in folder " << pszFolderPath << endl;
	else cout << endl;
	//�ʂ䌤��
	int base = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
	cout << "[output]constant base:" << base << endl;
	int check;
	//���δ����Ŀ¼�еĸ�����Ŀ
	for (int i = 0; i < RootEntCnt; i++)
	{

		SetHeaderOffset(base, NULL, FILE_BEGIN);
		ReadFromDisk(rootEntry_ptr, 32, NULL);
		base += 32;
		if (strcmp(pszFolderName, rootEntry_ptr->DIR_Name) == 0)//�Ѿ����ڣ�������
			return false;
		if (rootEntry_ptr->DIR_Name[0] == '\0') //continue;     //�ǿ���Ŀ��������д���ļ�
												//����Ŀ������д���ļ�
		{
			RootEntry *DirInfo_ptr = (RootEntry*)malloc(sizeof(RootEntry));
			cout << "[output]into loop2" << endl;
			strcpy(DirInfo_ptr->DIR_Name, pszFolderName);
			DirInfo_ptr->DIR_Attr = 0x10;
			fillTime(DirInfo_ptr->DIR_WrtDate, DirInfo_ptr->DIR_WrtTime);
			DirInfo_ptr->DIR_FileSize = 0;
			DirInfo_ptr->DIR_FstClus = FstClus;
			cout << "[output]constant input dir name:" << DirInfo_ptr->DIR_Name << endl;
			printf("[output]constant input file attr:%x\n", DirInfo_ptr->DIR_Attr);
			cout << "[output]constant input file firstClus:" << DirInfo_ptr->DIR_FstClus << endl;
			SetHeaderOffset(base - 32, NULL, FILE_BEGIN);
			if (WriteToDisk(DirInfo_ptr, 32, NULL))
			{
				dwHandles.push_back(*DirInfo_ptr);
				FileHandle = dwHandles.size();
				writeFat(FstClus, 0xffff);
				return true;
			}

			else cout << "[debug]Write to disk fail!" << endl;
		}

	}
	return false;
}
DWORD MyCreateFile(char *pszFolderPath, char *pszFileName)
{	
	u16 FstClus = findEmptyFat();
	cout << "[debug]findEmptyFat():" << FstClus << endl;
	
	DWORD FileHandle = 0;
	//printBPB();
	cout << "[output]trying to create file named " << pszFileName;
	if (strcmp(pszFolderPath, "") != 0)
		cout << " in folder " << pszFolderPath << endl;
	else cout << endl;
	//�ʂ䌤��
	int base = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
	cout << "[output]constant base:" << base << endl;
	int check;
	//���δ����Ŀ¼�еĸ�����Ŀ
	int i;

	int max_FstClus = 0;//����Ѱ�������״�FstClus,���ļ��������֮��
	int max_FinalClus = 0;//��ֹ��=FstClus+m
	for (i = 0; i < RootEntCnt; i++)
	{

		SetHeaderOffset(base, NULL, FILE_BEGIN);
		ReadFromDisk(rootEntry_ptr, 32, NULL);
		if (max_FstClus < rootEntry_ptr->DIR_FstClus&&rootEntry_ptr->DIR_Name[0] != '\0')
		{
			max_FstClus = rootEntry_ptr->DIR_FstClus;
			u32 cuSize = SecPerClus*BytsPerSec; //512
			u16 needCu = (rootEntry_ptr->DIR_FileSize + cuSize - 1) / cuSize;
			max_FinalClus = max_FstClus + needCu;
		}
		base += 32;
		//����Ҫ���ˣ�

		if (strcmp(pszFileName, rootEntry_ptr->DIR_Name) == 0)//�Ѿ����ڣ�������
			return 0;
		if (rootEntry_ptr->DIR_Name[0] == '\0') //continue;     //�ǿ���Ŀ��������д���ļ�
			//����Ŀ������д���ļ�

		{
			RootEntry *FileInfo_ptr = (RootEntry*)malloc(sizeof(RootEntry));
			cout << "[output]into loop2" << endl;
			strcpy(FileInfo_ptr->DIR_Name, pszFileName);
			FileInfo_ptr->DIR_Attr = 0x20;
			fillTime(FileInfo_ptr->DIR_WrtDate, FileInfo_ptr->DIR_WrtTime);
			FileInfo_ptr->DIR_FileSize = 0;
			u32 cuSize = SecPerClus*BytsPerSec; //512
			u16 needCu = (FileInfo_ptr->DIR_FileSize + cuSize - 1) / cuSize;//����ȡ���������Ҫ�Ĵصĸ���
//TODO����ʼ����Ҫ��д��
			FileInfo_ptr->DIR_FstClus = FstClus;
			cout << "[output]constant cuSize :" << cuSize << endl;
			cout << "[output]constant needCu :" << needCu << endl;
			cout << "[output]constant input file name:" << FileInfo_ptr->DIR_Name << endl;
			cout << "[output]constant input FileSize:" << FileInfo_ptr->DIR_FileSize << endl;
			printf("[output]constant input file attr:%x\n", FileInfo_ptr->DIR_Attr);
			cout << "[output]constant input file date:" << FileInfo_ptr->DIR_WrtDate << endl;
			cout << "[output]constant input file time:" << FileInfo_ptr->DIR_WrtTime << endl;
			cout << "[output]constant input file firstClus:" << FileInfo_ptr->DIR_FstClus << endl;
			// TODO (812015941#1#): д�������
			SetHeaderOffset(base - 32, NULL, FILE_BEGIN);
			if (WriteToDisk(FileInfo_ptr, 32, NULL))
			{
				dwHandles.push_back(*FileInfo_ptr);
				FileHandle = dwHandles.size();
				writeFat(FstClus, 0xffff);
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
	fillHandles();
	DWORD FileHandle = 0;
	//printBPB();
	cout << "[output]trying to delete file named " << pszFileName;
	if (strcmp(pszFolderPath, "") != 0)
		cout << " in folder " << pszFolderPath << endl;
	else cout << endl;
	//�ʂ䌤��
	int i = 0;
	vector<RootEntry>::iterator it;// = dwHandles.begin();
	for (it = dwHandles.begin(); it != dwHandles.end(); it++)
	{
		if (strcmp(pszFileName, dwHandles[i].DIR_Name) == 0)
		{
			FileHandle = i;
			return FileHandle;
		}
		i++;
	}
	/*for (int i = 1; i < dwHandles.size(); i++)
	{
		if (strcmp(pszFileName, dwHandles[i]->DIR_Name) == 0)
		{
			FileHandle = i;
			return FileHandle;
		}
		else iter++;
	}*/
	return FileHandle;
}

void MyDeleteFile(char *pszFolderPath, char *pszFileName)//û��Ҫɾ�����ݴص����ݣ�?
{
	DWORD FileHandle = 0;
	//printBPB();
	cout << "[output]trying to delete file named " << pszFileName;
	if (strcmp(pszFolderPath, "") != 0)
		cout << " in folder " << pszFolderPath << endl;
	else cout << endl;
	//�ʂ䌤��
	int base = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
	int i;
	for (i = 0; i < RootEntCnt; i++)
	{

		SetHeaderOffset(base, NULL, FILE_BEGIN);
		ReadFromDisk(rootEntry_ptr, 32, NULL);
		base += 32;
		//����Ҫ���ˣ�

		if (strcmp(pszFileName, rootEntry_ptr->DIR_Name) == 0)//�ҵ�
		{
			RootEntry *FileInfo_ptr = (RootEntry*)malloc(sizeof(RootEntry));
			for (int j = 0; j < 11; j++)
			{
				FileInfo_ptr->DIR_Name[j] = '\0';
			}

			FileInfo_ptr->DIR_Attr = 0x00;
			fillTime(FileInfo_ptr->DIR_WrtDate, FileInfo_ptr->DIR_WrtTime);
			FileInfo_ptr->DIR_FileSize = 0;
			u32 cuSize = SecPerClus*BytsPerSec; //512
// TODO (812015941#1#): ��ҪѰ�ҷ����λ��
			SetHeaderOffset(base - 32, NULL, FILE_BEGIN);
			if (WriteToDisk(FileInfo_ptr, 32, NULL))
			{
				FileHandle = createHandle(FileInfo_ptr);
				//ɾ��fat�ж�Ӧ�����ݡ�
				u16 FstClus = rootEntry_ptr->DIR_FstClus;
				while (FstClus != 0xfff)
				{
					u16 tmpClus = findNextFat(FstClus);
					writeFat(FstClus, 0x0000);
					FstClus = tmpClus;
				}
				if(FstClus==0xfff)//�����滻0x200H�ı�����
					writeFat(FstClus, 0x0000);
				break;
			}

			else cout << "[debug]Write to disk fail!" << endl;
		}

	}
	return;
}
//TODO: д�ļ�
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
DWORD MyWriteFile(DWORD dwHandle, LPVOID pBuffer, DWORD dwBytesToWrite) {
	int FstClus = dwHandles[dwHandle].DIR_FstClus;
	int dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2)*BytsPerSec;
	cout << "[debug] " << dwHandles[dwHandle].DIR_Name << " locates at " << dataBase << " (10 hexadecimal) 0x" << DecIntToHexStr(dataBase) << "H (16 hexadecimal) in file!" << endl;
	//��ȡ�ˣ�Ҫд�ĵ�ַ
	//��Ŀ¼������ʼ��ַΪ��1+9+9��*512=0x2600h��
	//�ٶ���Ŀ¼����СΪ0x1c00������ȱʡֵ224*32Bytes��������������ʼƫ��Ϊ0x2600+0x1c00=0x4200��
	SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
	if (WriteToDisk(pBuffer, dwBytesToWrite, NULL))
	{
		dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;//д��fat
		dwHandles[dwHandle].DIR_FileSize = dwBytesToWrite;
		struct RootEntry* FileInfo_ptr = &dwHandles[dwHandle];
		updateRootEntry(FileInfo_ptr);


		return dwBytesToWrite;
	}
	return -1;
}
DWORD MyReadFile(DWORD dwHandle, LPVOID pBuffer, DWORD dwBytesToRead)
{
	int FstClus = dwHandles[dwHandle].DIR_FstClus;
	int dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2)*BytsPerSec;
	cout << "[debug] " << dwHandles[dwHandle].DIR_Name << " locates at " << dataBase << " (10 hexadecimal) 0x" << DecIntToHexStr(dataBase) << "H (16 hexadecimal) in file!" << endl;
	//��ȡ�ˣ�Ҫд�ĵ�ַ
	//��Ŀ¼������ʼ��ַΪ��1+9+9��*512=0x2600h��
	//�ٶ���Ŀ¼����СΪ0x1c00������ȱʡֵ224*32Bytes��������������ʼƫ��Ϊ0x2600+0x1c00=0x4200��
	SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
	if (ReadFromDisk(pBuffer, dwHandles[dwHandle].DIR_FileSize, NULL) != 0)
	{
		u8 ptr[15];
		//u8 *ptr= (u8*)malloc(dwBytesToRead);
//TODO:��취���
		//hex_to_str(ptr, pBuffer, sizeof(pBuffer));

		///printf("[output]Read file:%s\ncontents: %s \n", dwHandles[dwHandle].DIR_Name, get_raw_string(pBuffer));
		return dwBytesToRead;
	}
	return -1;
}