#include "myutil.h"
wstring&   replace_allW(wstring&   str,const   wstring&   old_value,const   wstring&   new_value)   
{   
    while(true)   {   
        wstring::size_type   pos(0);   
        if(   (pos=str.find(old_value))!=wstring::npos   )   
            str.replace(pos,old_value.length(),new_value);   
        else   break;   
    }   
    return   str;   
}   
string&   replace_all(string&   str,const   string&   old_value,const   string&   new_value)   
{   
    while(true)   {   
        string::size_type   pos(0);   
        if(   (pos=str.find(old_value))!=string::npos   )   
            str.replace(pos,old_value.length(),new_value);   
        else   break;   
    }   
    return   str;   
}   
wstring&   replace_all_distinct(wstring&   str,const   wstring&   old_value,const   wstring&   new_value)   
{   
    for(wstring::size_type   pos(0);   pos!=wstring::npos;   pos+=new_value.length())   {   
        if(   (pos=str.find(old_value,pos))!=wstring::npos   )   
            str.replace(pos,old_value.length(),new_value);   
        else   break;   
    }   
    return   str;   
}

bool isGB(const char*gb,int len){
	for(int i=0;i<len;++i){
		if(gb[i]<0){
			return true;
		}
	}
	return false;
}

int IsTextUTF8(const char* str,long length)
{
	int i;
	long nBytes=0;//UFT8����1-6���ֽڱ���,ASCII��һ���ֽ�
	unsigned char chr;
	bool bAllAscii=true; //���ȫ������ASCII, ˵������UTF-8
	for(i=0;i<length;i++)
	{
		chr= *(str+i);
		if( (chr&0x80) != 0 ) // �ж��Ƿ�ASCII����,�������,˵���п�����UTF-8,ASCII��7λ����,����һ���ֽڴ�,���λ���Ϊ0,o0xxxxxxx
			bAllAscii= false;
		if(nBytes==0) //�������ASCII��,Ӧ���Ƕ��ֽڷ�,�����ֽ���
		{
			if(chr>=0x80)
			{
				if(chr>=0xFC&&chr<=0xFD)
					nBytes=6;
				else if(chr>=0xF8)
					nBytes=5;
				else if(chr>=0xF0)
					nBytes=4;
				else if(chr>=0xE0)
					nBytes=3;
				else if(chr>=0xC0)
					nBytes=2;
				else
				{
					return false;
				}
				nBytes--;
			}
		}
		else //���ֽڷ��ķ����ֽ�,ӦΪ 10xxxxxx
		{
			if( (chr&0xC0) != 0x80 )
			{
				return false;
			}
			nBytes--;
		}
	}
	if( nBytes > 0 ) //Υ������
	{
		return false;
	}
	if( bAllAscii ) //���ȫ������ASCII, ˵������UTF-8
	{
		return false;
	}
	return true;
}

 