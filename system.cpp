#include <system.h>
#include <sstream>
#include <tchar.h>
#include <shellapi.h>
#include <ShlObj.h>
#include "base64.h"

wstring GetExtW(wstring path){
	int index=path.find_last_of('.');
	if(index==-1){
		return L"";
	}
	else{
		wstring type=path.substr(index+1);
		return replace_allW(type,L"jpg",L"jpeg");
	}
}

string GetExt(string path){
	int index=path.find_last_of('.');
	if(index==-1){
		return "";
	}
	else{
		string type=path.substr(index+1);
		return replace_all(type,"jpg","jpeg");
	}
}

BOOL EnableShutdownPrivilege()
{
    HANDLE hProcess = NULL;
    HANDLE hToken = NULL;
    LUID uID = {0};
    TOKEN_PRIVILEGES stToken_Privileges = {0};

    hProcess = ::GetCurrentProcess();  //��ȡ��ǰӦ�ó�����̾��
    
    if(!::OpenProcessToken(hProcess,TOKEN_ADJUST_PRIVILEGES,&hToken))  //�򿪵�ǰ���̵ķ������ƾ��(OpenProcessToken��������ʧ�ܷ���ֵΪ��)
        return FALSE;

    if(!::LookupPrivilegeValue(NULL,SE_SHUTDOWN_NAME,&uID))  //��ȡȨ������Ϊ"SeShutdownPrivilege"��LUID(LookupPrivilegeValue��������ʧ�ܷ���ֵΪ��)
        return FALSE;

    stToken_Privileges.PrivilegeCount = 1;  //��������Ȩ�޸���
    stToken_Privileges.Privileges[0].Luid = uID;  //Ȩ�޵�LUID
    stToken_Privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;  //Ȩ�޵�����,SE_PRIVILEGE_ENABLEDΪʹ�ܸ�Ȩ��
    
    if(!::AdjustTokenPrivileges(hToken,FALSE,&stToken_Privileges,sizeof stToken_Privileges,NULL,NULL))  //���������������ָ��Ȩ��(AdjustTokenPrivileges��������ʧ�ܷ���ֵΪ��)
        return FALSE;

    if(::GetLastError() != ERROR_SUCCESS)  //�鿴Ȩ���Ƿ�����ɹ�
        return FALSE;

    ::CloseHandle(hToken);
    return TRUE;
}

//�ػ�����
BOOL Shutdown(BOOL bForce)
{
    EnableShutdownPrivilege();  //ʹ�ܹػ���Ȩ����
    if(bForce)
        return ::ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE,0);  //ǿ�ƹػ�
    else
        return ::ExitWindowsEx(EWX_SHUTDOWN,0);
}

//ע������
BOOL Logoff(BOOL bForce)
{
    if(bForce)
        return ::ExitWindowsEx(EWX_LOGOFF | EWX_FORCE,0);  //ǿ��ע��
    else
        return ::ExitWindowsEx(EWX_LOGOFF,0);
}

//��������
BOOL Reboot(BOOL bForce)
{
    EnableShutdownPrivilege();  //ʹ�ܹػ���Ȩ����
    if(bForce)
        return ::ExitWindowsEx(EWX_REBOOT | EWX_FORCE,0);  //ǿ������
    else
        return ::ExitWindowsEx(EWX_REBOOT,0);
}
int CDECL MessageBoxPrintf (TCHAR * szCaption, TCHAR * szFormat, ...)  
{      
  TCHAR  szBuffer [1024] ;  
  va_list pArgList ;  
  // The va_start macro (defined in STDARG.H) is usually equivalent to:  
  // pArgList = (char *) &szFormat + sizeof (szFormat) ;  
  va_start (pArgList, szFormat) ;  
  // The last argument to wvsprintf points to the arguments  
  _vsntprintf ( szBuffer, sizeof (szBuffer) / sizeof (TCHAR),  
    szFormat, pArgList) ;  
  // The va_end macro just zeroes out pArgList for no good reason  
  va_end (pArgList) ;  //Defoe.Tu  tyysoft@yahoo.com.cn Windows ������� PDF ������ 
  return MessageBox (NULL, szBuffer, szCaption, 0) ;  
}
BOOL  SaveFileDialog(HWND hWnd, const TCHAR* fileNameStr, TCHAR* szFile)
{
	//TCHAR szFile[2048];
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
// must !
	ofn.lpstrFile = szFile;
	ofn.lpstrTitle = TEXT("�����ļ�");
	ofn.lpstrFileTitle=(LPWSTR)fileNameStr;
	ofn.nMaxFile = 2048;//sizeof(szFile);
//
	ofn.lpstrFile[0] = '\0';
	wcscpy(ofn.lpstrFile, (LPWSTR)fileNameStr);
	ofn.Flags = OFN_OVERWRITEPROMPT;
//no extention file!    ofn.lpstrFilter="Any file(*.*)\0*.*\0ddfs\0ddfs*\0";
	return(GetSaveFileName((LPOPENFILENAME)&ofn));
}
BOOL  OpenFileDialog(HWND hWnd, const TCHAR* fileNameStr, TCHAR* szFile)
{
	//TCHAR szFile[2048];
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
// must !
	ofn.lpstrFile = szFile;
	ofn.lpstrTitle = TEXT("���ļ�");
	ofn.lpstrFileTitle=(LPWSTR)fileNameStr;
	ofn.nMaxFile = 2048;//sizeof(szFile);
//
	ofn.lpstrFile[0] = '\0';
	wcscpy(ofn.lpstrFile, (LPWSTR)fileNameStr);
	ofn.Flags = OFN_FILEMUSTEXIST;
//no extention file!    ofn.lpstrFilter="Any file(*.*)\0*.*\0ddfs\0ddfs*\0";
	return(GetOpenFileName((LPOPENFILENAME)&ofn));
}

//��ͬʱ����Ŀ¼���ļ�:path������·����Ҳ�������ļ����������ļ�ͨ���
wstring find(wstring path,bool cursive)
{ 
     //ȡ·�������һ��"//"֮ǰ�Ĳ���,����"//"
	replace_allW(path,L"\\",L"/");
	UINT index=path.find_last_of('/');
	if(index+1==path.length()){
		path.append(L"*.*");
	}
    wstring prefix=path.substr(0,path.find_last_of('/')+1);

    WIN32_FIND_DATA FindFileData;
    HANDLE hFind=::FindFirstFile(path.data(),&FindFileData);
	std::wstringstream ss;
    if(INVALID_HANDLE_VALUE == hFind)
    {
       ss<<L"[]";
	   FindClose(hFind);
	   return ss.str();
    }
	else{
		ss<<L"[";
	}
    while(TRUE)
    {
		bool flag=false;;
      //Ŀ¼
        if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            //���ǵ�ǰĿ¼��Ҳ���Ǹ�Ŀ¼
            if(cursive&&FindFileData.cFileName[0]!='.')
            {
				wstring temp=prefix+FindFileData.cFileName;
				ss<<L"{\"name\":\""<<FindFileData.cFileName<<L"\",\"list\":"<<find(temp+L"/*.*",cursive).data()<<L"}";
				flag=true;
            }
        }
        //�ļ�
        else
        {   
             ss<<L"\""<<FindFileData.cFileName<<L"\"";
			flag=true;
        }
        if(!FindNextFile(hFind,&FindFileData))
              break;
		else if(flag){
			ss<<L",";
		}
    }
	ss<<L"]";
    FindClose(hFind);
	return ss.str();
}

void SetDefaultBrowser(const TCHAR *strAppName){
	HKEY key;
	RegOpenKey(HKEY_CLASSES_ROOT,_T("http\\shell\\open\\command"),&key);
	RegSetValue(key,NULL,REG_SZ,strAppName,sizeof(strAppName));
	RegCloseKey(key);

	RegOpenKey(HKEY_CLASSES_ROOT,_T("HKEY_CLASSES_ROOT\\htmlfile\\shell\\open\\command"),&key);
	RegSetValue(key,NULL,REG_SZ,strAppName,sizeof(strAppName));
	RegCloseKey(key);

	RegOpenKey(HKEY_CLASSES_ROOT,_T("HKEY_CLASSES_ROOT\\mhtmlfile\\shell\\open\\command"),&key);
	RegSetValue(key,NULL,REG_SZ,strAppName,sizeof(strAppName));
	RegCloseKey(key);

	RegOpenKey(HKEY_CLASSES_ROOT,_T("HKEY_CLASSES_ROOT\\InternetShortcut\\shell\\open\\command"),&key);
	RegSetValue(key,NULL,REG_SZ,strAppName,sizeof(strAppName));
	RegCloseKey(key);
}
//---------------------------------------------------------------------------
// ����ļ��������
// strExt: Ҫ������չ��(����: ".txt")
// strAppKey: ExeName��չ����ע����еļ�ֵ(����: "txtfile")
// ����TRUE: ��ʾ�ѹ�����FALSE: ��ʾδ����
BOOL CheckFileRelation(const TCHAR *strExt, const TCHAR *strAppKey, TCHAR *strAppName, TCHAR *strDefaultIcon, TCHAR *strDescribe)
{
    int nRet=TRUE;
    HKEY hExtKey;
    TCHAR szPath[_MAX_PATH]; 
    DWORD dwSize=sizeof(szPath); 
    TCHAR strTemp[_MAX_PATH];
    if(RegOpenKey(HKEY_CLASSES_ROOT,strExt,&hExtKey)==ERROR_SUCCESS)
    {
        RegQueryValueEx(hExtKey,NULL,NULL,NULL,(LPBYTE)szPath,&dwSize);
        if(_wcsicmp(szPath,strAppKey)!=0)
        {
            nRet=FALSE;
			RegSetValue(hExtKey,L"",REG_SZ,strAppKey,wcslen(strAppKey)+1);
			//return nRet;
        }
        RegCloseKey(hExtKey);
    }
	dwSize=sizeof(szPath); 
    if(RegOpenKey(HKEY_CLASSES_ROOT,strAppKey,&hExtKey)==ERROR_SUCCESS)
    {
        RegQueryValueEx(hExtKey,NULL,NULL,NULL,(LPBYTE)szPath,&dwSize);
        if(_wcsicmp(szPath,strDescribe)!=0)
        {
            nRet=FALSE;
			RegSetValue(hExtKey,L"",REG_SZ,strDescribe,wcslen(strDescribe)+1);
        }
        RegCloseKey(hExtKey);
    }
	dwSize=sizeof(szPath); 
	wsprintf(strTemp,L"%s\\DefaultIcon",strAppKey);
    if(RegOpenKey(HKEY_CLASSES_ROOT,strTemp,&hExtKey)==ERROR_SUCCESS)
    {
        RegQueryValueEx(hExtKey,NULL,NULL,NULL,(LPBYTE)szPath,&dwSize);
        if(_wcsicmp(szPath,strDefaultIcon)!=0)
        {
            nRet=FALSE;
			RegSetValue(hExtKey,L"",REG_SZ,strDefaultIcon,wcslen(strDefaultIcon)+1);
        }
        RegCloseKey(hExtKey);
    }
	dwSize=sizeof(szPath); 
    wsprintf(strTemp,L"%s\\Shell",strAppKey);
    if(RegOpenKey(HKEY_CLASSES_ROOT,strTemp,&hExtKey)==ERROR_SUCCESS)
    {
        RegQueryValueEx(hExtKey,NULL,NULL,NULL,(LPBYTE)szPath,&dwSize);
        if(_wcsicmp(szPath,L"Open")!=0)
        {
            nRet=FALSE;
			RegSetValue(hExtKey,L"",REG_SZ,L"Open",wcslen(L"Open")+1);
        }
        RegCloseKey(hExtKey);
    }
	dwSize=sizeof(szPath); 
    wsprintf(strTemp,L"%s\\Shell\\Open\\Command",strAppKey);
    if(RegOpenKey(HKEY_CLASSES_ROOT,strTemp,&hExtKey)==ERROR_SUCCESS)
    {
		wsprintf(strTemp,L"%s \"%%1\"",strAppName);
        RegQueryValueEx(hExtKey,NULL,NULL,NULL,(LPBYTE)szPath,&dwSize);
        if(_wcsicmp(szPath,strTemp)!=0)
        {
            nRet=FALSE;
			RegSetValue(hExtKey,L"",REG_SZ,strTemp,wcslen(strTemp)+1);
        }
        RegCloseKey(hExtKey);
    }
	dwSize=sizeof(szPath); 
    if(RegOpenKey(HKEY_CURRENT_USER,strExt,&hExtKey)==ERROR_SUCCESS)
    {
        RegQueryValueEx(hExtKey,NULL,NULL,NULL,(LPBYTE)szPath,&dwSize);
        if(_wcsicmp(szPath,strAppKey)!=0)
        {
            nRet=FALSE;
			RegSetValue(hExtKey,L"",REG_SZ,strAppKey,wcslen(strAppKey)+1);
        }
        RegCloseKey(hExtKey);
    }
	dwSize=sizeof(szPath); 
    if(RegOpenKey(HKEY_CURRENT_USER,strAppKey,&hExtKey)==ERROR_SUCCESS)
    {
        RegQueryValueEx(hExtKey,NULL,NULL,NULL,(LPBYTE)szPath,&dwSize);
        if(_wcsicmp(szPath,strDescribe)!=0)
        {
            nRet=FALSE;
			RegSetValue(hExtKey,L"",REG_SZ,strDescribe,wcslen(strDescribe)+1);
        }
        RegCloseKey(hExtKey);
    }
	dwSize=sizeof(szPath); 
	wsprintf(strTemp,L"%s\\DefaultIcon",strAppKey);
    if(RegOpenKey(HKEY_CURRENT_USER,strTemp,&hExtKey)==ERROR_SUCCESS)
    {
        RegQueryValueEx(hExtKey,NULL,NULL,NULL,(LPBYTE)szPath,&dwSize);
        if(_wcsicmp(szPath,strDefaultIcon)!=0)
        {
            nRet=FALSE;
			RegSetValue(hExtKey,L"",REG_SZ,strDefaultIcon,wcslen(strDefaultIcon)+1);
        }
        RegCloseKey(hExtKey);
    }
	dwSize=sizeof(szPath); 
    wsprintf(strTemp,L"%s\\Shell",strAppKey);
    if(RegOpenKey(HKEY_CURRENT_USER,strTemp,&hExtKey)==ERROR_SUCCESS)
    {
        RegQueryValueEx(hExtKey,NULL,NULL,NULL,(LPBYTE)szPath,&dwSize);
        if(_wcsicmp(szPath,L"Open")!=0)
        {
            nRet=FALSE;
			RegSetValue(hExtKey,L"",REG_SZ,L"Open",wcslen(L"Open")+1);
        }
        RegCloseKey(hExtKey);
    }
	dwSize=sizeof(szPath); 
    wsprintf(strTemp,L"%s\\Shell\\Open\\Command",strAppKey);
    if(RegOpenKey(HKEY_CURRENT_USER,strTemp,&hExtKey)==ERROR_SUCCESS)
    {
		wsprintf(strTemp,L"%s \"%%1\"",strAppName);
        RegQueryValueEx(hExtKey,NULL,NULL,NULL,(LPBYTE)szPath,&dwSize);
        if(_wcsicmp(szPath,strTemp)!=0)
        {
            nRet=FALSE;
			RegSetValue(hExtKey,L"",REG_SZ,strTemp,wcslen(strTemp)+1);
        }
        RegCloseKey(hExtKey);
    }
	dwSize=sizeof(szPath); 
    wsprintf(strTemp,L"Software\\Classes\\%s_auto_file\\Shell\\Open\\Command",strExt+1);
    if(RegOpenKey(HKEY_CURRENT_USER,strTemp,&hExtKey)==ERROR_SUCCESS)
    {
		wsprintf(strTemp,L"%s \"%%1\"",strAppName);
        RegQueryValueEx(hExtKey,NULL,NULL,NULL,(LPBYTE)szPath,&dwSize);
        if(_wcsicmp(szPath,strTemp)!=0)
        {
            nRet=FALSE;
			RegSetValue(hExtKey,L"",REG_SZ,strTemp,wcslen(strTemp)+1);
        }
        RegCloseKey(hExtKey);
    }
    return nRet;
}
//---------------------------------------------------------------------------
// ע���ļ�����
// strExe: Ҫ������չ��(����: ".txt")
// strAppName: Ҫ������Ӧ�ó�����(����: "C:\MyApp\MyApp.exe")
// strAppKey: ExeName��չ����ע����еļ�ֵ(����: "txtfile")
// strDefaultIcon: ��չ��ΪstrAppName��ͼ���ļ�(����: "C:\MyApp\MyApp.exe,0")
// strDescribe: �ļ���������
void RegisterFileRelation(TCHAR *strExt, TCHAR *strAppName, TCHAR *strAppKey, TCHAR *strDefaultIcon, TCHAR *strDescribe)
{
    TCHAR strTemp[_MAX_PATH];
    HKEY hKey;
    
    RegCreateKey(HKEY_CLASSES_ROOT,strExt,&hKey);
    RegSetValue(hKey,L"",REG_SZ,strAppKey,wcslen(strAppKey)+1);
    RegCloseKey(hKey);
    
    RegCreateKey(HKEY_CLASSES_ROOT,strAppKey,&hKey);
    RegSetValue(hKey,L"",REG_SZ,strDescribe,wcslen(strDescribe)+1);
    RegCloseKey(hKey);
    
    wsprintf(strTemp,L"%s\\DefaultIcon",strAppKey);
    RegCreateKey(HKEY_CLASSES_ROOT,strTemp,&hKey);
    RegSetValue(hKey,L"",REG_SZ,strDefaultIcon,wcslen(strDefaultIcon)+1);
    RegCloseKey(hKey);
    
    wsprintf(strTemp,L"%s\\Shell",strAppKey);
    RegCreateKey(HKEY_CLASSES_ROOT,strTemp,&hKey);
    RegSetValue(hKey,L"",REG_SZ,L"Open",wcslen(L"Open")+1);
    RegCloseKey(hKey);
    
    wsprintf(strTemp,L"%s\\Shell\\Open\\Command",strAppKey);
    RegCreateKey(HKEY_CLASSES_ROOT,strTemp,&hKey);
    wsprintf(strTemp,L"%s \"%%1\"",strAppName);
    RegSetValue(hKey,L"",REG_SZ,strTemp,wcslen(strTemp)+1);
    RegCloseKey(hKey);
    
    wsprintf(strTemp,L"%s\\DefaultIcon",strAppKey);
    RegCreateKey(HKEY_CURRENT_USER,strTemp,&hKey);
    RegSetValue(hKey,L"",REG_SZ,strDefaultIcon,wcslen(strDefaultIcon)+1);
    RegCloseKey(hKey);
    
    wsprintf(strTemp,L"%s\\Shell",strAppKey);
    RegCreateKey(HKEY_CURRENT_USER,strTemp,&hKey);
    RegSetValue(hKey,L"",REG_SZ,L"Open",wcslen(L"Open")+1);
    RegCloseKey(hKey);
    
    wsprintf(strTemp,L"%s\\Shell\\Open\\Command",strAppKey);
    RegCreateKey(HKEY_CURRENT_USER,strTemp,&hKey);
    wsprintf(strTemp,L"%s \"%%1\"",strAppName);
    RegSetValue(hKey,L"",REG_SZ,strTemp,wcslen(strTemp)+1);
    RegCloseKey(hKey);
    
    wsprintf(strTemp,L"Software\\Classes\\%s_auto_file\\Shell\\Open\\Command",strExt+1);
    RegCreateKey(HKEY_CURRENT_USER,strTemp,&hKey);
    wsprintf(strTemp,L"%s \"%%1\"",strAppName);
    RegSetValue(hKey,L"",REG_SZ,strTemp,wcslen(strTemp)+1);
    RegCloseKey(hKey);
	SHChangeNotify(SHCNE_ASSOCCHANGED,SHCNF_FLUSHNOWAIT,0, 0); 
}

Bitmap* GetImageFromBase64(string encodedImage)
{
	int imageSize = int((encodedImage.length()/3)+1)*4;
	char* t=new char[imageSize];
	base64_decode(encodedImage.c_str(),encodedImage.length(), t, &imageSize); // using the base64 
	HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, imageSize);
	LPVOID pImage = ::GlobalLock(hMem);
	memcpy(pImage, t, imageSize);

	IStream* pStream = NULL;
	::CreateStreamOnHGlobal(hMem, FALSE, &pStream);

	Bitmap *img = new Bitmap(pStream);
	pStream->Release();
	GlobalUnlock(hMem);
	GlobalFree(hMem);
	delete []t;
    return img;
}

//TransparentWnd::connection.RegisterConnection(L"WebtopSever");
int   GetEncoderClsid(const   WCHAR*   format,   CLSID*   pClsid) 
{ 
      UINT     num   =   0;                     //   number   of   image   encoders 
      UINT     size   =   0;                   //   size   of   the   image   encoder   array   in   bytes 

      ImageCodecInfo*   pImageCodecInfo   =   NULL; 

      GetImageEncodersSize(&num,   &size); 
      if(size   ==   0) 
            return   -1;     //   Failure 

      pImageCodecInfo   =   (ImageCodecInfo*)(malloc(size)); 
      if(pImageCodecInfo   ==   NULL) 
            return   -1;     //   Failure 

      GetImageEncoders(num,   size,   pImageCodecInfo); 

      for(UINT   j   =   0;   j   <   num;   ++j) 
      { 
            if(   wcscmp(pImageCodecInfo[j].MimeType,   format)   ==   0   ) 
            { 
                  *pClsid   =   pImageCodecInfo[j].Clsid; 
                  free(pImageCodecInfo); 
                  return   j;     //   Success 
            }         
      } 

      free(pImageCodecInfo); 
      return   -1;     //   Failure 
} 
void SaveBitmap(Bitmap* pbm, wstring path){
	CLSID tiffClsid;
	GetEncoderClsid((L"image/"+ GetExtW(path)).data(), &tiffClsid);
	pbm->Save(path.data(), &tiffClsid);
}
