#include "MyApi.h"
#include "util.h"
#include "DiskLib.h"
#ifdef OLDVERSION


extern FILE* fat12;//��佫�ᱻ�滻
#endif // OLDVERSION
using namespace std;
extern struct BPB* bpb_ptr;
extern struct RootEntry* rootEntry_ptr;
//���涼Ϊ�̶�ֵ��
int  BytsPerSec;    //ÿ�����ֽ���
int  SecPerClus;    //ÿ��������
int  RsvdSecCnt;    //Boot��¼ռ�õ�������
int  NumFATs;   //FAT�����
int  RootEntCnt;    //��Ŀ¼����ļ���
int  FATSz; //FAT������
BOOL MyCreateDirectory(char *pszFolderPath, char *pszFolderName)//path��Ҫ�У� ����folder��Ҫ��
{
	u16 FstClus = findEmptyFat();//������дDIR_FstClus
	u16 FstClusHJQ;//�����洢��Ŀ¼��DIR_FstClus
	int base;
	cout << "[debug]findEmptyFat():" << FstClus << endl;
	cout << "[output]trying to create Directory named " << pszFolderName;
	if (strcmp(pszFolderPath, "") != 0)
	{
		cout << " in folder " << pszFolderPath << endl;
		if ((FstClusHJQ = isPathExist(pszFolderPath)) || strlen(pszFolderPath) == 3)//���ڲ��ܼ���,�Ժ���ӵ��ļ�Ӧ
		{
			if (isDirectoryExist(pszFolderName, FstClusHJQ))
			{
				cout << "[output]" << pszFolderPath << '\\' << pszFolderName << " folder has existed!" << endl;
				return false;//���pszFolderName�Ĵغţ�
			}
			if (FstClusHJQ == 0) {
				// ��Ŀ¼��ƫ��
				base = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
			}
			else {
				// �������ļ���ַƫ��
				base = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClusHJQ - 2) * BytsPerSec;//=
				cout << "[debug]" << pszFolderPath << "'s FstClus value is:" << FstClusHJQ << ";postion (16λ): is " << hex << base << endl;
			}
		}
		else {
			cout << "[output]" << pszFolderPath << '\\' << pszFolderName << " path does not exist!" << endl;

			return false;
		}
	}//---------------------------------------------������������·���������
	else {
		cout << endl;
		base = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
	}//---------------------------------------------û��·���������ڸ�Ŀ¼�¡�
	cout << "[output]" << pszFolderName << "'s RootEntry is ready to write in position:" << hex << base << " (16 hexadecimal)" << endl;

	for (int i = 0; i < RootEntCnt; i++)//Ѱ�ҿյ�Ŀ¼��
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
			strcpy(DirInfo_ptr->DIR_Name, pszFolderName);
			DirInfo_ptr->DIR_Attr = 0x10;
			fillTime(DirInfo_ptr->DIR_WrtDate, DirInfo_ptr->DIR_WrtTime);
			DirInfo_ptr->DIR_FileSize = 0;
			DirInfo_ptr->DIR_FstClus = FstClus;
			if (strcmp(pszFolderPath, "") != 0)//����и�·��
			{
				SetHeaderOffset(base - 32, NULL, FILE_BEGIN);
				WriteToDisk(DirInfo_ptr, 32, NULL);
				//TODO:׼�����'.' �� '..'����.���ǵ�ǰĿ¼�ı�������..���״ؾ͸�ָ���ϼ�Ŀ¼�ļ����״ء�
				//��ӵ�������Ĵء�
				base = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (DirInfo_ptr->DIR_FstClus - 2) * BytsPerSec;//���¼���base�����Լ�����������д"."��
				int selfFstClus = DirInfo_ptr->DIR_FstClus;//�ݴ��Լ����״أ�������"."
				cout << "[debug]���ļ��еĵ�ַ���и���·�� :" << hex << base << endl;
				//TODO:�Ѷ�Ӧ������F6��0��!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!------------------------------------------
				clearCu(DirInfo_ptr->DIR_FstClus);
				SetHeaderOffset(base, NULL, FILE_BEGIN);
				initFileInfo(DirInfo_ptr, ".", 0x10, 0, DirInfo_ptr->DIR_FstClus);
				WriteToDisk(DirInfo_ptr, 32, NULL);
				initFileInfo(DirInfo_ptr, "..", 0x10, 0, FstClusHJQ);//����һ����Ŀ¼���״�
				SetHeaderOffset(base + 32, NULL, FILE_BEGIN);
				WriteToDisk(DirInfo_ptr, 32, NULL);
				writeFat(FstClus, 0xffff);
				return true;
			}
			if (strcmp(pszFolderPath, "") == 0)//����޸�·�����ڸ��ļ�Ŀ¼д������Ӧ��������������Ӧ��λ�����.��..�Ŷԡ�
			{

				SetHeaderOffset(base - 32, NULL, FILE_BEGIN);//д�ڸ�Ŀ¼
				WriteToDisk(DirInfo_ptr, 32, NULL);
				FstClusHJQ = DirInfo_ptr->DIR_FstClus;//�ݴ��Լ����״أ�������"."
				base = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32;//10176
				base += (FstClusHJQ - 2) * BytsPerSec;//д�ڶ�Ӧ��������
				cout << "[debug]���ļ��еĵ�ַ���޸��� :" << hex << base << endl;
				//TODO:�Ѷ�Ӧ������F6��0��!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!------------------------------------------
				clearCu(DirInfo_ptr->DIR_FstClus);
				SetHeaderOffset(base, NULL, FILE_BEGIN);
				WriteToDisk(DirInfo_ptr, 32, NULL);
				initFileInfo(DirInfo_ptr, ".", 0x10, 0, DirInfo_ptr->DIR_FstClus);//�ؾ����Լ���
				SetHeaderOffset(base, NULL, FILE_BEGIN);
				WriteToDisk(DirInfo_ptr, 32, NULL);
				initFileInfo(DirInfo_ptr, "..", 0x10, 0, 0x00);//��һ���ؾ��Ǹ�Ŀ¼��0��
				SetHeaderOffset(base + 32, NULL, FILE_BEGIN);
				WriteToDisk(DirInfo_ptr, 32, NULL);
				writeFat(FstClus, 0xffff);
				return true;
			}
		}

	}
	return false;
}
DWORD MyCreateFile(char *pszFolderPath, char *pszFileName)
{
	fillHandles();
	int dwHandle = 0;
	u16 FstClus = findEmptyFat();//������дDIR_FstClus
	u16 FstClusHJQ;//�����洢��Ŀ¼��DIR_FstClus
	int base;
	cout << "[debug]findEmptyFat():" << FstClus << endl;
	cout << "[output]trying to create Directory named " << pszFileName;
	if (strcmp(pszFolderPath, "") != 0)
	{
		cout << " in folder " << pszFolderPath << endl;
		if ((FstClusHJQ = isPathExist(pszFolderPath)) || strlen(pszFolderPath) == 3)//���ڲ��ܼ���,�Ժ���ӵ��ļ�Ӧ
		{
			if (isFileExist(pszFileName, FstClusHJQ))
			{
				cout << "[output]" << pszFolderPath << '\\' << pszFileName << " file has existed!" << endl;
				return false;//���pszFolderName�Ĵغţ�
			}
			if (FstClusHJQ == 0) {
				// ��Ŀ¼��ƫ��
				base = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
			}
			else {
				// �������ļ���ַƫ��
				base = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClusHJQ - 2) * BytsPerSec;//=
				cout << "[debug]" << pszFolderPath << "'s FstClus value is:" << FstClusHJQ << ";postion (16λ): is " << hex << base << endl;
			}
		}
		else {
			cout << "[output]" << pszFolderPath << '\\' << pszFileName << " path does not exist!" << endl;

			return false;
		}
	}//---------------------------------------------������������·���������
	else {
		cout << endl;
		base = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
	}//---------------------------------------------û��·���������ڸ�Ŀ¼�¡�
	cout << "[output]" << pszFileName << "'s RootEntry is ready to write in position:" << hex << base << " (16 hexadecimal)" << endl;
	for (int i = 0; i < RootEntCnt; i++)//Ѱ�ҿյ�Ŀ¼��
	{
		SetHeaderOffset(base, NULL, FILE_BEGIN);
		ReadFromDisk(rootEntry_ptr, 32, NULL);
		base += 32;
		if (strcmp(pszFileName, rootEntry_ptr->DIR_Name) == 0)//�Ѿ����ڣ�������
			return false;
		if (rootEntry_ptr->DIR_Name[0] == '\0') //continue;     //�ǿ���Ŀ��������д���ļ�
												//����Ŀ������д���ļ�
		{
			RootEntry *DirInfo_ptr = (RootEntry*)malloc(sizeof(RootEntry));
			strcpy(DirInfo_ptr->DIR_Name, pszFileName);
			DirInfo_ptr->DIR_Attr = 0x20;
			fillTime(DirInfo_ptr->DIR_WrtDate, DirInfo_ptr->DIR_WrtTime);
			DirInfo_ptr->DIR_FileSize = 0;
			DirInfo_ptr->DIR_FstClus = FstClus;
			if (strcmp(pszFolderPath, "") != 0)//����и�·��
			{
				FileHandle fileHandle;
				fileHandle.fileInfo = *DirInfo_ptr;
				dwHandles.push_back(fileHandle);
				dwHandle = dwHandles.size();
				SetHeaderOffset(base - 32, NULL, FILE_BEGIN);
				WriteToDisk(DirInfo_ptr, 32, NULL);
				//TODO:׼�����'.' �� '..'����.���ǵ�ǰĿ¼�ı�������..���״ؾ͸�ָ���ϼ�Ŀ¼�ļ����״ء�
				//��ӵ�������Ĵء�
				base = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (DirInfo_ptr->DIR_FstClus - 2) * BytsPerSec;//���¼���base,�õ�����ļ������ڵ������ַ
				int selfFstClus = DirInfo_ptr->DIR_FstClus;//�ݴ��Լ����״أ�������"."
				cout << "[debug]���ļ��ĵ�ַ���и���·�� :" << hex << base << endl;
				//TODO:�Ѷ�Ӧ������F6��0��!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!------------------------------------------
				clearCu(DirInfo_ptr->DIR_FstClus);
				writeFat(FstClus, 0xffff);
				return dwHandle;
			}
			else if (strcmp(pszFolderPath, "") == 0)//����޸�·�����ڸ��ļ�Ŀ¼д������Ӧ��������������Ӧ��λ�����.��..�Ŷԡ�
			{
				FileHandle fileHandle;
				fileHandle.fileInfo = *DirInfo_ptr;
				dwHandles.push_back(fileHandle);
				dwHandle = dwHandles.size();
				SetHeaderOffset(base - 32, NULL, FILE_BEGIN);//д�ڸ�Ŀ¼
				WriteToDisk(DirInfo_ptr, 32, NULL);
				FstClusHJQ = DirInfo_ptr->DIR_FstClus;//�ݴ��Լ����״أ�������"."
				base = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32;//10176
				base += (FstClusHJQ - 2) * BytsPerSec;//д�ڶ�Ӧ��������
				cout << "[debug]���ļ��ĵ�ַ���޸��� :" << hex << base << endl;
				//TODO:�Ѷ�Ӧ������F6��0��!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!------------------------------------------
				clearCu(DirInfo_ptr->DIR_FstClus);
				writeFat(FstClus, 0xffff);
				return dwHandle;
			}
		}
	}
	return dwHandle;



	/*
	for (int i = 0; i < RootEntCnt; i++)
	{

		SetHeaderOffset(base, NULL, FILE_BEGIN);
		ReadFromDisk(rootEntry_ptr, 32, NULL);
		base += 32;
		//����Ҫ���ˣ�

		if (strcmp(pszFileName, rootEntry_ptr->DIR_Name) == 0)//�Ѿ����ڣ�������
			return 0;
		if (rootEntry_ptr->DIR_Name[0] == '\0'|| rootEntry_ptr->DIR_Name[0] == '\0') //continue;     //�ǿ���Ŀ��������д���ļ�
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
	*/
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
	bool trick = true;//trick:Ϊ����ܷ��ء�
	int HandleBack = 0;
	//printBPB();
	cout << "[output]trying to open file named " << pszFileName;
	if (strcmp(pszFolderPath, "") != 0)
	{
		//trick:���û��Ŀ¼����һ�γ��־�ȡ�������û��Ŀ¼����ô�ڶ���ȡ��
		trick = !trick;
		cout << " in folder " << pszFolderPath << endl;
	}
	else cout << endl;
	//�ʂ䌤��
	int size = dwHandles.size();
	for (int i = 0; i < size; i++)
	{
		if (strcmp(pszFileName, dwHandles[i].fileInfo.DIR_Name) == 0)
		{
			FileHandle = i;
			if (trick) {
				return FileHandle;//Ϊ�棬���̷��أ�
			}
			else
			{
				trick = !trick;
				HandleBack = FileHandle;
			}
		}
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
	return HandleBack;
}
void MyCloseFile(DWORD dwHandle) {
	dwHandles.clear();
}
BOOL MyDeleteFile(char *pszFolderPath, char *pszFileName)
{
	u16 FstClusHJQ;//�����洢��Ŀ¼��DIR_FstClus
	int base;
	cout << "[output]trying to delete file named " << pszFileName;
	if (strcmp(pszFolderPath, "") != 0)
	{
		cout << " in folder " << pszFolderPath << endl;
		if ((FstClusHJQ = isPathExist(pszFolderPath)) || strlen(pszFolderPath) == 3)//���ڲ��ܼ���,�Ժ���ӵ��ļ�Ӧ
		{
			if (isFileExist(pszFileName, FstClusHJQ))
			{
				cout << "[output]" << pszFolderPath << '\\' << pszFileName << " file existed! Continuing..." << endl;
			}
			if (FstClusHJQ == 0) {
				// ��Ŀ¼��ƫ��
				base = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
			}
			else {
				// �������ļ���ַƫ��
				base = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClusHJQ - 2) * BytsPerSec;//=
				cout << "[debug]" << pszFolderPath << "'s FstClus value is:" << FstClusHJQ << ";postion (16λ): is " << hex << base << endl;
			}
		}
		else {
			cout << "[output]" << pszFolderPath << '\\' << pszFileName << " path does not exist!" << endl;
			return false;
		}
	}//---------------------------------------------������������·���������
	else {
		cout << endl;
		base = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
	}//---------------------------------------------û��·�����ļ��ڸ�Ŀ¼�¡�

	cout << "[output]" << pszFileName << "'s RootEntry is ready to be cleared in position:" << hex << base << " (16 hexadecimal)" << endl;
	for (int i = 0; i < RootEntCnt; i++)//�ҵ�Ҫɾ�����ļ���
	{
		SetHeaderOffset(base, NULL, FILE_BEGIN);
		ReadFromDisk(rootEntry_ptr, 32, NULL);
		base += 32;
		if (strcmp(pszFileName, rootEntry_ptr->DIR_Name) == 0)//�ҵ���
		{
			RootEntry *DirInfo_ptr = (RootEntry*)malloc(sizeof(RootEntry));
			SetHeaderOffset(base - 32, NULL, FILE_BEGIN);//д�ڸ�Ŀ¼
			initFileInfo(DirInfo_ptr, "", 0x0, 0, 0);
			WriteToDisk(DirInfo_ptr, 32, NULL);
			FstClusHJQ = rootEntry_ptr->DIR_FstClus;//�ݴ��Լ����״�
			int nextClus = 0;
			do {
				if (nextClus != 0)//����֮�ʣ���������ɾ��������
				{
					FstClusHJQ = nextClus;
				}
				clearCu(FstClusHJQ);
				nextClus = findNextFat(FstClusHJQ);
				writeFat(FstClusHJQ, 0x0000);
			} while (nextClus != 0xFFF && nextClus != 0);
			return true;
		}
	}
	return false;


	/*
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
				if (FstClus == 0xfff)//�����滻0x200H�ı�����
					writeFat(FstClus, 0x0000);
				break;
			}

			else cout << "[debug]Write to disk fail!" << endl;
		}

	}
	return;
	*/
}
BOOL MyDeleteDirectory(char *pszFolderPath, char *pszFolderName) {
	u16 FstClusHJQ;//�����洢��Ŀ¼��DIR_FstClus
	int base;
	cout << "[output]trying to delete directory named " << pszFolderName;
	if (strcmp(pszFolderPath, "") != 0)
	{
		cout << " in folder " << pszFolderPath << endl;
		if ((FstClusHJQ = isPathExist(pszFolderPath)) || strlen(pszFolderPath) == 3)//���ڲ��ܼ���,�Ժ���ӵ��ļ�Ӧ
		{
			if (isDirectoryExist(pszFolderName, FstClusHJQ))
			{
				cout << "[output]" << pszFolderPath << '\\' << pszFolderName << "folder existed! Continuing..." << endl;
			}
			if (FstClusHJQ == 0) {
				// ��Ŀ¼��ƫ��
				base = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
			}
			else {
				// �������ļ���ַƫ��
				base = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClusHJQ - 2) * BytsPerSec;//=
				cout << "[debug]" << pszFolderPath << "'s FstClus value is:" << FstClusHJQ << ";postion (16λ): is " << hex << base << endl;
			}
		}
		else {
			cout << "[output]" << pszFolderPath << '\\' << pszFolderName << " path does not exist!" << endl;
			return false;
		}
	}//---------------------------------------------������������·���������
	else {
		cout << endl;
		base = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
	}//---------------------------------------------û��·�����ļ��ڸ�Ŀ¼�¡�
	cout << "[output]" << pszFolderName << "'s RootEntry is ready to be cleared in position:" << hex << base << " (16 hexadecimal)" << endl;
	for (int i = 0; i < RootEntCnt; i++)//�ҵ�Ҫɾ�����ļ���
	{
		SetHeaderOffset(base, NULL, FILE_BEGIN);
		ReadFromDisk(rootEntry_ptr, 32, NULL);
		base += 32;
		int base_back = base;
		if (strcmp(pszFolderName, rootEntry_ptr->DIR_Name) == 0)//�ҵ���
		{
			RootEntry *DirInfo_ptr = (RootEntry*)malloc(sizeof(RootEntry));

			FstClusHJQ = rootEntry_ptr->DIR_FstClus;//�ݴ��Լ����״�
//TODO:ɾ��Ŀ¼�µ��ļ���Ŀ¼���ļ���ɾ����¼���ɣ���Ҫ�����أ����Ե��ú�������
			base = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClusHJQ - 2) * BytsPerSec;//1 ��ȥɾ�����ռ���ļ�
			for (int i = 0; i < RootEntCnt; i++)//�ҵ�Ҫɾ�����ļ���
			{
				SetHeaderOffset(base, NULL, FILE_BEGIN);
				ReadFromDisk(rootEntry_ptr, 32, NULL);
				base += 32;
				if (rootEntry_ptr->DIR_Attr == 0x20 && rootEntry_ptr->DIR_Name[0] != '.')//2�ҵ����ļ�
				{


					char tmpPath1[sizeof(pszFolderName) + 3] = { 0 };
					strcat(tmpPath1, "c:\\");
					strcat(tmpPath1, pszFolderName);
					char tmpName[sizeof(rootEntry_ptr->DIR_Name)];
					strcpy(tmpName, rootEntry_ptr->DIR_Name);
					MyDeleteFile(tmpPath1, tmpName);
					continue;
				}
				if (rootEntry_ptr->DIR_Attr == 0x10 && rootEntry_ptr->DIR_Name[0] != '.')//3�ҵ���Ŀ¼
				{
					char tmpPath1[sizeof(pszFolderName) + 3] = { 0 };
					strcat(tmpPath1, "c:\\");
					strcat(tmpPath1, pszFolderName);
					MyDeleteDirectory(tmpPath1, rootEntry_ptr->DIR_Name);
					continue;
				}
			}
			SetHeaderOffset(base_back - 32, NULL, FILE_BEGIN);//д�ڸ�Ŀ¼//ע�⣺�������Ҫ�ã������Ȳ��ż������
			initFileInfo(DirInfo_ptr, "", 0x0, 0, 0);
			WriteToDisk(DirInfo_ptr, 32, NULL);
			clearCu(FstClusHJQ);
			writeFat(FstClusHJQ, 0x0000);
			//int nextClus = 0;
			//do {
			//	if (nextClus != 0)//����֮�ʣ���������ɾ��������
			//	{
			//		FstClusHJQ = nextClus;
			//	}
			//	clearCu(FstClusHJQ);
			//	nextClus = findNextFat(FstClusHJQ);
			//	writeFat(FstClusHJQ, 0x0000);
			//} while (nextClus != 0xFFF && nextClus != 0);
			return true;
		}
	}
	return false;
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
	int lenOfBuffer = dwBytesToWrite; // ��������д�볤��
	char* cBuffer = (char*)malloc(sizeof(u8)*lenOfBuffer);
	memcpy(cBuffer, pBuffer, lenOfBuffer); // ���ƹ���
	struct RootEntry* FileInfo_ptr = &dwHandles[dwHandle].fileInfo;
	u32 updatedFileSize = dwBytesToWrite;
	int dataBase = 0, dataOffset = 0;
	//TODO:����headerȥ��
	u16 FstClus = FileInfo_ptr->DIR_FstClus;
	u16 nextEmptyClus = findEmptyFat();
	LONG offset = dwHandles[dwHandle].offset;//��ȡ��¼�ĵ�ǰƫ��
	int curClusNum = offset / (BytsPerSec*SecPerClus); // ��ǰָ���ڵڼ�������,���Ϊ0�����״أ�Ϊ1��˵�������ӵĵ�һ����
	int curClusOffset = (offset )% (BytsPerSec*SecPerClus); // ��ǰ��������ƫ��
	u16 needClu = (curClusOffset + dwBytesToWrite + (BytsPerSec*SecPerClus) - 1) / (BytsPerSec*SecPerClus);//��Ҫ�Ĵ���������һ������������1,2,3��
	//--------------------------------�����Ǹ���FAT��
	if (needClu == 1)//������Ҫһ���أ����ȥ������
	{
	}
	else if (needClu > 1)
	{
		u16 nextClus;
		u16 FstClusUse = FstClus;//��֤FstClus���䡣
		u16 needClusBack = needClu;//��֤FstClus���䡣
		int originClus = 1;//�ļ�ӵ�еĴصĸ���
		//int curClusNum_PLUS_needClu = curClusNum + needClu;
		while (1)//���ѭ������ͳ��ԭ�����ļ������˶��ٸ���
		{
			if (findNextFat(FstClusUse) == 0xfff)
			{
				//��������ļ���ֻ��һ���أ���ôoriginClus=1
				break;
			}
			else {
				FstClusUse = findNextFat(FstClusUse);
				originClus++;
			}
		}
#define Empty_File 1
		//�Ѿ�ȷ�������Ǹ����ļ���С�ļ������ļ���
		if (originClus >= needClusBack+curClusNum)//���ļ���������ǰ�����������ˡ���ʽ�ұ����ֵ��MY_FILE_END�������
		{

		}
		else if (originClus == Empty_File)//���ļ���������µ��㹻����������
		{
			needClu = needClusBack;
			while (needClu > 1)//���ض�����룩�����ѭ������������fat
			{
				nextClus = findEmptyFat();
				writeFat(FstClusUse, nextClus);//���Լ���0XFFF���³���һ�ء�
				needClu--;
				FstClusUse = nextClus;
				continue;
			}
			writeFat(FstClusUse, 0xffff);//��ĩβ��д��0xfff
		}
		else if (originClus > Empty_File && originClus < needClusBack+curClusNum)//һ���С�ļ�����Ҫ��չ������Ĵء�������ǰ��2�أ���Ҫ3�أ���ô����Ҫ����һ�ء�
		{

			needClu = needClusBack;
			FstClusUse = FstClus;
			while (findNextFat(FstClusUse) != 0xFFF && findNextFat(FstClusUse) != 0)//Ѱ���ļ��Ѿ���������һ���ء�
			{
				FstClusUse = findNextFat(FstClusUse);
			}
			while (needClu > 1)//���ض�����룩�����ѭ������������fat
			{
				nextClus = findEmptyFat();
				writeFat(FstClusUse, nextClus);//���Լ���0XFFF���³���һ�ء�
				needClu--;
				FstClusUse = nextClus;
				continue;
			}
			writeFat(FstClusUse, 0xffff);//��ĩβ��д��0xfff
		}




	}

	else {
		cout << "[debug]��Ҫ�Ĵؼ����������ԡ�needClu:" << needClu << endl;
		return -1;
	}
	dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (curClusNum + FstClus - 2) * BytsPerSec;//�趨�״�ƫ��ֵ
	dataOffset = dataBase + curClusOffset;//������dataOffset��
	//------------------�����Ǹ���RootENTRY
	fillTime(FileInfo_ptr->DIR_WrtDate, FileInfo_ptr->DIR_WrtTime);
	if (FileInfo_ptr->DIR_FileSize >= (lenOfBuffer + offset))//���д��ĳ���+ƫ��<=ԭ�ļ���С,��ô�ļ���С���䡣
	{
	}
	else {
		FileInfo_ptr->DIR_FileSize = offset + lenOfBuffer;
	}
	updateRootEntry(dwHandles[dwHandle].parentClus, FileInfo_ptr);
	//------------------������д�롣
	int hasWritten = 0;
	if (lenOfBuffer < 512 - curClusOffset)//��С����дһ���ء�����ʣ�µ��㹻���롣
	{
		SetHeaderOffset(dataOffset, NULL, FILE_BEGIN);
		WriteToDisk(cBuffer, lenOfBuffer, NULL);
		return lenOfBuffer;
	}
	do {//Ҫд�����
		dataOffset = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (curClusNum + FstClus - 2) * BytsPerSec + curClusOffset;//dataOffset���¡�
		//��һ��д�Ժ�curClusOffset�ض����0��
		SetHeaderOffset(dataOffset, NULL, FILE_BEGIN);

		if (hasWritten + 512-curClusOffset <= lenOfBuffer)//��һ��д�룬���ݵ�ǰ��ƫ��д�����ʣ�µĿռ��Ѿ��㹻д�룬��ôôд�뼴��
		{
			WriteToDisk(&cBuffer[hasWritten], 512 - curClusOffset, NULL);
		}
		else {
			WriteToDisk(&cBuffer[hasWritten], lenOfBuffer - hasWritten, NULL);//д��ʣ�µ��ַ���
		}
		hasWritten += 512 - curClusOffset;
		curClusOffset = 0;//д�˵�һ���Ժ����ͱ��0���Ժ�ÿ�ζ�д512�ֽڡ�
	} while ((FstClus = findNextFat(FstClus)) != 0xfff && FstClus != 0);
		return lenOfBuffer;


	//int FstClus = dwHandles[dwHandle].fileInfo.DIR_FstClus;
	//int dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2)*BytsPerSec;
	//cout << "[debug] " << dwHandles[dwHandle].fileInfo.DIR_Name << " locates at " << dataBase << " (10 hexadecimal) 0x" << DecIntToHexStr(dataBase) << "H (16 hexadecimal) in file!" << endl;
	////��ȡ�ˣ�Ҫд�ĵ�ַ
	////��Ŀ¼������ʼ��ַΪ��1+9+9��*512=0x2600h��
	////�ٶ���Ŀ¼����СΪ0x1c00������ȱʡֵ224*32Bytes��������������ʼƫ��Ϊ0x2600+0x1c00=0x4200��
	//SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
	//if (WriteToDisk(pBuffer, dwBytesToWrite, NULL))
	//{
	//	dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;//д��fat
	//	dwHandles[dwHandle].fileInfo.DIR_FileSize = dwBytesToWrite;
	//	struct RootEntry* FileInfo_ptr = &dwHandles[dwHandle].fileInfo;
	//	updateRootEntry(FileInfo_ptr);
	//	return dwBytesToWrite;
	//}
	return -1;
}
DWORD MyReadFile(DWORD dwHandle, LPVOID pBuffer, DWORD dwBytesToRead)
{
	struct RootEntry* FileInfo_ptr = &dwHandles[dwHandle].fileInfo;
	//TODO:����headerȥ��
	u16 FstClus = FileInfo_ptr->DIR_FstClus;
	LONG offset = dwHandles[dwHandle].offset;//��ȡ��¼�ĵ�ǰƫ��
	int curClusNum = offset / BytsPerSec; // ��ǰָ���ڵڼ�������
	int curClusOffset = offset % BytsPerSec; // ��ǰ��������ƫ��
	while (curClusNum) {
		if (findNextFat(FstClus) == 0xFFF) {
			break;
		}
		FstClus = findNextFat(FstClus);
		curClusNum--;
	}// ��ȡ��ǰָ����ָ����


	//memset(cBuffer, 0, lenOfBuffer);
	int hasRead = 0;
	int base = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2)*BytsPerSec;
	int dataOffset = base + curClusOffset;
	int lenOfBuffer = dwBytesToRead;// �����������볤��
	if (FileInfo_ptr->DIR_FileSize - offset < lenOfBuffer) {
		lenOfBuffer = FileInfo_ptr->DIR_FileSize - offset;
	}
	char* cBuffer = (char*)malloc(sizeof(u8)*lenOfBuffer);// ����һ��������
	memset(cBuffer, 0, lenOfBuffer);
	cout << "[output]" << dwHandles[dwHandle].fileInfo.DIR_Name << "'s FstClus value is:" << FstClus << ";postion (16λ): is " << hex << base << endl;
	int count = BytsPerSec*SecPerClus / 512;//ÿһ������Ҫ��ȡ�Ĵ���
	char byte[512];
	do {
		base = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2)*BytsPerSec;
		for (int i = 0; i < count; i++)
		{
			SetHeaderOffset(base, NULL, FILE_BEGIN);
			ReadFromDisk(byte, 512, NULL);
		}
		//cout << byte ;
		FstClus = findNextFat(FstClus);
		if (hasRead > lenOfBuffer)//���������һ��512�ֽ��Ѿ�������Ҫ����ֽڶ���
		{
			memcpy(&cBuffer[hasRead], byte, lenOfBuffer - hasRead + 512);
			break;
		}
		memcpy(&cBuffer[hasRead], byte, 512);
		hasRead += 512;
	} while (FstClus != 0xfff && FstClus != 0);
	//cout << endl;
	cout << "[output]File contend:" << endl << cBuffer << endl;
	memcpy(pBuffer, cBuffer, 512);



	/*int FstClus = dwHandles[dwHandle].DIR_FstClus;
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
	*/
	return -1;
}
BOOL MySetFilePointer(DWORD dwFileHandle, int nOffset, DWORD dwMoveMethod) {
	FileHandle *hd = &dwHandles[dwFileHandle];
	//if (hd == NULL ) return FALSE; // ���������
	LONG curOffset = nOffset + hd->offset; // currentģʽ��ƫ�ƺ��λ��
	u16 currentClus = hd->fileInfo.DIR_FstClus; // �״�
	int fileSize = hd->fileInfo.DIR_FileSize; // �ļ���С
	switch (dwMoveMethod) {
	case MY_FILE_BEGIN:
		if (nOffset < 0) {
			hd->offset = 0; // С��0����Ϊ0
		}
		else if (nOffset > fileSize) {
			hd->offset = fileSize;
		}
		else {
			hd->offset = nOffset;
		}
		break;
	case MY_FILE_CURRENT:
		if (curOffset < 0) {
			hd->offset = 0;
		}
		else if (curOffset > fileSize) {
			hd->offset = fileSize;
		}
		else {
			hd->offset = curOffset;
		}
		break;
	case MY_FILE_END:
		if (nOffset > 0) {
			hd->offset = fileSize;
		}
		else if (nOffset < -fileSize) {
			hd->offset = 0;
		}
		else {
			hd->offset = fileSize + nOffset;
		}
		break;
	}
	return TRUE;
}
