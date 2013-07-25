// StringDLL.cpp : Defines the exported functions for the DLL application.

#include "stdafx.h"
#include <stdio.h>
#include <assert.h>
#include <vcclr.h>
#include <cstdlib>
#include "windows.h"
#include "math.h"
#include <ctime>

#include "stdafx.h"
#include <stdio.h>
#include <assert.h>
#include <Math.h>

#define DEBUG

#if defined(DEBUG)
	#define WARN(message)				\
	{									\
		MessageBox(						\
			NULL,						\
			TEXT(message),				\
			TEXT("������������ �����"),	\
			MB_OK | MB_ICONERROR		\
		);								\
		exit(EXIT_FAILURE);				\
	}

	#define WARN_IF(condition, message)		\
	{										\
		if (condition)						\
		{									\
			MessageBox(						\
				NULL,						\
				TEXT(message),				\
				TEXT(#condition),			\
				MB_OK | MB_ICONERROR		\
			);								\
			exit(EXIT_FAILURE);				\
		}									\
	}
#else
	#define WARN(message)
	#define WARN_IF(condition, message)
#endif

using namespace System;
using namespace System::Reflection;
using namespace System::Runtime::CompilerServices;
using namespace System::Runtime::InteropServices;
using namespace System::Security::Permissions;
using namespace std;



extern "C"
{

	struct FString
	{
		wchar_t* Data;
		int ArrayNum;
		int ArrayMax;

		void UpdateArrayNum()
		{
			ArrayNum = wcslen(Data)+1;
			assert(ArrayNum <= ArrayMax);
		}
	};

	struct Var{//��������� ����������, ������� ������ ���� �������
		gcroot<String^> Name;//��� ����������
		gcroot<String^> Value;//� �� ��������
	};

	enum iifOp{//������������ ���������� ������� iif
		r,//�����
		nr,//�� �����
		b,//������
		m,//������
		br,//������ ���� �����
		mr,//������ ���� �����
		null//������
	};

	// ���������� ����������

	// ����������, ������������ � ��������
	Var *Vars[100];
	
	int NumVars=0;//���-�� ���������� � ������


	//���������� �������

	//������� ����� ����� � ������ ��������
	String^ ParsingCode(String^ Str);
	int inStr(String^ Str, int start, char* ch) ;
	String^ mid(String^ Str, int start, int close);
	array<String^>^ Split(String^ Str, char* ch);
	String^ RemoveSpaces(String^ Str);
	String^ FunctionDefiction(String^ Str);
	String^ iif(String^ Str);
	iifOp iifOperator(String^ Str);
	String^ RetVar(String^ Str);
	String^ randF(String^ Str);
	//����� ���������� �������

//������� �������
	String^ StartDefinerd(String^ str)
	{
		String^ str2=str;//�������� ������, ������� � ���������� � ��������
		String^ code;//����������, � ������� ��������� ���, ����������� � {...}
		int startInStr=0,//����������, � ������� ���������� �����, � �������� ������ ��������� ������ "{"
			start=0,//����������, � ������� ���������� ��������� ������ "{"
			close;//����������, � ������� ���������� ��������� ������ "}"
		while (start!=-1)//���� ������ ������ {...}
		{
			start=inStr(str,startInStr,"{");//���������� ����� �������, � ������� ���������� "{", ������� � startInStr
			close=inStr(str,startInStr+1,"}");//���������� ����� ������ � ������� ���������� "}", ������� � startInStr
			if (start==-1) break;//���� ������� InStr ���������� -1, �� ��� ������ ��� ������ "{" ������ �� ����������� � ������� �� �����
			code=mid(str,start,close);//����� �������� ��� ���, ������������ {...}
			startInStr=close;//���������� �����, � �������� � ��������� ��� ����� ������
			str2=str2->Replace("{"+code+"}",ParsingCode(code));//�������� {...} �� ����������� ��� ...
		}
		//��� � ����� ��� ������� =)
		return str2;
	}


//���������� ������� ������� ���� ���� {... & ... & ...}
	String^ ParsingCode(String^ Str)
	{
		if (String::IsNullOrEmpty(Str)) return "";//�������� �� Null � ������
		array <String ^> ^Blocks = gcnew array<String ^> (100);//������ ������ ����, ����������� ��������� "&"
		String^ Temp="";//������, ������� �� ��������, �������� ���
		//P.S. ��� ����� ����������� ����������� � ������ �������, ��� ��� ������ �� �������������� �� ����

		Str=RemoveSpaces(Str);//������� ������� � ������ � � ����� ������
		Blocks=Split(Str,"+");//��������� ������ �������� "&", ��������� �� ������, ������� ���������� ������ ������ � �������
		for (int n=0;n<100;n++)//���� �������� ������, ����������� ��������� "&"
		{
			String^ Func=FunctionDefiction(Blocks[n]);//����������� ������� �����
			if (Func=="iif"){//���� ������� iif
				Temp+=iif(mid(Blocks[n],4,Blocks[n]->Length));//��������� � ���������� Temp ����������� ��� � ������� ������� iif
			}	else if (Func=="rand")	{//���� ������� rand
				Temp+=randF(mid(Blocks[n],5,Blocks[n]->Length));//��������� � ���������� Temp ����������� ��� � ������� ������� rand
			}	else	{//�����, ���� ������ ���������� ��� ���������, ��
				Temp+=RetVar(Blocks[n]);//��������� � ���������� Temp ����������� ��� � ������� ������� RetVar
			}
		}
		return Temp;
	}

//������ ����������� �������

	String^ FunctionDefiction(String^ Str){
		if (String::IsNullOrEmpty(Str)) return "";//�������� �� Null
		Str=RemoveSpaces(Str);
		String^ Temp=mid(Str->Replace(" ",""),0,5);
		if (mid(Str,0,4)=="iif" && mid(Str->Replace(" ",""),0,5)=="iif(") return "iif";
		if (mid(Str,0,5)=="rand" && mid(Str->Replace(" ",""),0,6)=="rand(") return "rand";
		return Str;//����� ���� ��� �� ���� �� �������, �� ���������� ����������� ������
	}


//������� ������� ��������� iif
	String^ iif(String^ Str)
	{
		iifOp Operator;//����������, � ������� ��������� �������� �������
		array<String^>^ Blocks = Split(Str,",");//����� ������� iif
		//iif(�������, ��� ����������, ���� ������� �������, ��� ����������, ���� ������� �����)
		String^ A=ParsingCode(Blocks[1]);//����, ����� ������� �������
		String^ B=ParsingCode(Blocks[2]);//����, ����� ������� �����
		//��� ��� ����� �������������� ����������� � ������� ������� ParsingCode, ��� ��������� ������������ ������ ������� � �������
		//�����, ��� ������� ��������

		array<String^>^ C;//���� �������, �� ����������� ��������
		String^ A1;//����, ����������� ����� ���������� �������
		String^ A2;//����, ����������� ����� ��������� �������
		Operator=iifOperator(Blocks[0]);//����������� ��������� �������

		if(iifOperator(Blocks[0])==r){//���� �������� "�����"
			C=Split(Blocks[0],"=");//���������� ����� ������� �� ������� ������� ��������� �������
			A1=ParsingCode(C[0]);//������ ����� �� ��������� �������
			A2=ParsingCode(C[1]);//������ ����� ����� ��������� �������
			if (A1==A2) return A; else return B;	//���������� ���� A ��� ���� B � ����������� �� �������


		}	else if(iifOperator(Blocks[0])==nr) {//���� �������� "�� �����"
			C=Split(Blocks[0],"<");//���������� ����� ������� �� ������� ������� ��������� �������
			A1=ParsingCode(C[0]);//������ ����� �� ��������� �������
			A2=ParsingCode(mid(C[1],1,C[1]->Length+1));//������ ����� ����� ��������� �������
			//������� mid �����, �.�. �������� ������� ������� �� 2-� ��������
			if (A1!=A2) return A; else return B;		//���������� ���� A ��� ���� B � ����������� �� �������


		}	else if(iifOperator(Blocks[0])==br) {//���� �������� "������, ���� �����"
			C=Split(Blocks[0],">");//���������� ����� ������� �� ������� ������� ��������� �������
			A1=ParsingCode(C[0]);//������ ����� �� ��������� �������
			A2=ParsingCode(mid(C[1],1,C[1]->Length+1));//������ ����� ����� ��������� �������
			if (Convert::ToInt32(A1)>=Convert::ToInt32(A2)) return A; else return B;	//���������� ���� A ��� ���� B � ����������� �� �������


		}	else if(iifOperator(Blocks[0])==mr) {//���� �������� "������, ���� �����"
			C=Split(Blocks[0],"<");//���������� ����� ������� �� ������� ������� ��������� �������
			A1=ParsingCode(C[0]);//������ ����� �� ��������� �������
			A2=ParsingCode(mid(C[1],1,C[1]->Length+1));//������ ����� ����� ��������� �������
			if (Convert::ToInt32(A1)<=Convert::ToInt32(A2)) return A; else return B;	//���������� ���� A ��� ���� B � ����������� �� �������


		}	else if(iifOperator(Blocks[0])==b) {//���� �������� "������"
			C=Split(Blocks[0],">");//���������� ����� ������� �� ������� ������� ��������� �������
			A1=ParsingCode(C[0]);//������ ����� �� ��������� �������
			A2=ParsingCode(C[1]);//������ ����� ����� ��������� �������
			if (Convert::ToInt32(A1)>Convert::ToInt32(A2)) return A; else return B;	//���������� ���� A ��� ���� B � ����������� �� �������


		}	else if(iifOperator(Blocks[0])==m) {//���� �������� "������"
			C=Split(Blocks[0],"<");//���������� ����� ������� �� ������� ������� ��������� �������
			A1=ParsingCode(C[0]);//������ ����� �� ��������� �������
			A2=ParsingCode(C[1]);//������ ����� ����� ��������� �������
			if (Convert::ToInt32(A1)<Convert::ToInt32(A2)) return A; else return B;	//���������� ���� A ��� ���� B � ����������� �� �������


		}	else	{//���� �������� "������"
			return "";
		}
		return "";
	}


//������ ����������� ��������� �������
	iifOp iifOperator(String^ Str)
	{
		//������������ �� �������� ������� ����������� �������
		if (Str->Replace("<>","")!=Str) return nr;
		if (Str->Replace("<=","")!=Str) return mr;
		if (Str->Replace(">=","")!=Str) return br;
		if (Str->Replace("<","")!=Str) return m;
		if (Str->Replace(">","")!=Str) return b;
		if (Str->Replace("=","")!=Str) return r;
		return null;//���� �� ���� �������� �� ��������, �� ���������� "������"
	}

//������� ������� ��������� rand
	String^ randF(String^ Str)
	{
		srand((int)time(0));//�����������
		int n;//���������� ������, �� ������� ���������� ��������� ����
		array<String^>^ Temp=Split(Str,",");//������ ������, �� ������� ���������� ���������
		for (n=0;n<100;n++)//����������� ���-�� ������, �� ������� ���������� ���������
		{
			if (String::IsNullOrEmpty(Temp[n])) break; //���� ������ ������ ��� ����� NULL, �� ������� �� �����
		}
		return ParsingCode(Temp[rand() %n ]);//���������� ����������� ������
	}

//������� ������� ��������� '...', ��� ����������
	String^ RetVar(String^ Str)
	{
		if (String::IsNullOrEmpty(Str)) return "";//���� ������ ������ ��� ����� NULL, �� ���������� ������
		Str=RemoveSpaces(Str);//������� ��� ������� � ������ � � ����� ������
		try{//�����������, �������� �� ������ ������ ���� 80
			int ch=Convert::ToInt32(Str);//������� ������������������ ������ � �����
										 //, ���� ������ ����� ����� �� "�����" ������, �� ���������� ����������, ������� ���������������
			return Str;//���� ������ ���� �����, �� ������ ���������� �������� ������
		}catch(FormatException^){//�������� ����������
		}
		if ((int)(Str[0])==(int)("'"[0]) && (int)(Str[Str->Length-1])==(int)("'"[0])) {//����������, ��������� ��  �������� ������ � �������
			return mid(RemoveSpaces(Str),1,Str->Length);//���� ��, �� ���������� ������ ��� �������
		}	else	{//����� ���� ����������
			for (int n=0;n<NumVars;n++)//���� ������ ���������� � ����
			{
				if (Vars[n]->Name==Str) return Vars[n]->Value;//���� ��� ���������� ��������� � �������� �������, �� ���������� �������� ����������
			}
		}
	}

//������� ����������� ������� ��������� �������

//Str - �������� ������
//start - ����� �������, � ������� ���������� ������
//ch - ������, ������� �� ����
//
	int inStr(String^ Str, int start, char* ch)
	{
		if (String::IsNullOrEmpty(Str)) return -1;//���� ������ ������ ��� ����� NULL, �� ���������� ������
		int S1=(int)(ch[0]);//��� �������, ������� ����
		for(int n=start;n<Str->Length;n++)
		{
			int S=(int)(Str[n]);//��� ������� ����� n � �������� ������
			if (S==S1) return n+1;//���������� ��� ����, ���� ��� �����, �� ���������� ����� �������
		}
		return -1;//���� ������ �� �����������, �� ���������� -1
	}

//������� ��������� ������

//Str - �������� ������
//start - ��������� ������
//close - �������� ������
//
	String^ mid(String^ Str, int start, int close)
	{
		String^ Temp;
		if (String::IsNullOrEmpty(Str)) return "";//�������� �� ������� � �� NULL
		if (Str->Length<close-start) return Str;//���� ����� �������� ������ ������ ��������� �����, �� ���������� �������� ������
		for (int n=start;n<close-1;n++)
		{
			Temp+=Str[n];//����� �� ������� ���������� � ���������� Temp
		}
		return Temp;
	}


//������� ���������� ������ �� ������ ������ � ������� �������

//Str - �������� ������
//ch - ������
//
	array<String^>^ Split(String^ Str, char* Ch){
		//�� ���� �����������������, �.�. ��� ����� �������
		array <String ^> ^Blocks = gcnew array<String ^> (100);
		String^ T="";
		int n;
		int h=0;
		int S,start;
		start=0;
		S=(int)(Ch[0]);
		for (n=0;n<Str->Length;n++)
		{
			if ((int)(Str[n])==S)
			{
				if (String::IsNullOrEmpty(T))
				{
					Blocks[h]=RemoveSpaces(mid(Str,start,n+1));
					h+=1;
					start=n+1;
				}
			}	else if((int)(Str[n])==(int)("("[0]))	{
				T+="(";
			}	else if((int)(Str[n])==(int)(")"[0]))	{
				if ((int)(T[T->Length-1]==(int)("("[0])))	{
					T=mid(T,0,T->Length);
				}
			}	else if((int)(Str[n])==(int)("'"[0]))	{
				if (String::IsNullOrEmpty(T))
				{
					T+="'";
				}	else	{
					if ((int)(T[T->Length-1]==(int)("'"[0])))	{
						T=mid(T,0,T->Length);
					}	else	{
						T+="'";
					}
				}
			}
		}
		Blocks[h]=RemoveSpaces(mid(Str,start,Str->Length+1));
		return Blocks;
	}

//������� �������� �������� � ������ � � ����� ������
	String^ RemoveSpaces(String^ Str)
	{
		if (String::IsNullOrEmpty(Str)) return "";//�������� �� ������� � �� NULL
		String^ Temp;
		int n=0;//����� �������
		while ((int)(Str[n])==(int)(" "[0])){//���� � ������ ���� ������
			n+=1;
		}
		int n1=Str->Length-1;//����� ������� � ����� ������
		while ((int)(Str[n1])==(int)(" "[0])){//���� � ����� ���� ������
			n1-=1;
		}
		Temp=mid(Str,n,n1+2);//��������� �������� �� ����
		return Temp;
	}

	bool StringToFStr(FString *exitString, String^ inputString)
	{
		// �������� ������������� ������ 
		array<unsigned char>^ encodedArray = System::Text::Encoding::GetEncoding(866)->GetBytes(inputString);
		// �������� ��������� �� ������
		pin_ptr<unsigned char> arrayPointer = &encodedArray[0];
		// ��������� ���������
		char* charString = (char*)arrayPointer;
		// ����� ����� ������, ������� ���� ��������
		int stringLength = strlen(charString);
		// ���� ������ ������� �������
		if (stringLength >= exitString->ArrayMax)
		{	// ���������� ��������������, �� ��������� ������
			return false;
		}
		// �������� ������ ��� ������ Unicode ��������
		wchar_t* wideString = new wchar_t[stringLength];
		// ������������ � Unicode
		MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED, charString,
			stringLength + 1, wideString, stringLength + 1);
		// ��������� ������ � FString
		wcscpy_s(exitString->Data, exitString->ArrayMax, wideString);
		// ������������� �������� ������ ����� �����
		exitString->UpdateArrayNum();
		// ������� ���������� ������
		delete[] wideString;
		return true;
	}

	struct answer{
		FString message;
		FString func;
		int parent;
	};

	struct quest{
		answer answer[6];
		FString message;
	};

	__declspec(dllexport) void LoadVars(wchar_t* name, wchar_t* value){
		Vars[NumVars]=new Var;
		Vars[NumVars]->Name=gcnew String(name);
		Vars[NumVars]->Value=gcnew String(value);
		NumVars++;
	}

	String^ Decrypt(String^ fName){
		BYTE a = 1;
		cli::array<BYTE> ^Key = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
		cli::array<BYTE> ^IV = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
		String^ str;
		System::Security::Cryptography::RijndaelManaged^ RMCrypto=gcnew System::Security::Cryptography::RijndaelManaged;
		RMCrypto->IV=IV;
		RMCrypto->Key=Key;
		System::IO::FileStream^ fs = gcnew System::IO::FileStream(fName,System::IO::FileMode::Open);
		System::Security::Cryptography::CryptoStream^ CryptStream = gcnew System::Security::Cryptography::CryptoStream(fs, RMCrypto->CreateDecryptor(), Security::Cryptography::CryptoStreamMode::Read);
		IO::StreamReader^ SReader = gcnew IO::StreamReader(CryptStream);
		str=SReader->ReadToEnd();
		SReader->Close();
		fs->Close();
		CryptStream->Close();
		return str;
	}

	/*__declspec(dllexport) void ShowString(TestStruct* UDKString)
	{
		wchar_t temp[1024];
		swprintf_s(temp, 1024, L"ArrayMax is %d,ArrayNum is %d, String is %s", UDKString->Stroka.ArrayMax,UDKString->Stroka.ArrayNum, UDKString->Stroka.Data);
		MessageBox(0, temp, L"Message1", MB_OK);
	}

	__declspec(dllexport) bool UpdateString(TestStruct* UDKString, wchar_t* MyString)
	{
		String^ asd;
		asd = "������� �����!";
		// ���� ������ ������� �������, ���������� UDK � ���������� ���������� ���
		if (!encoding2(&UDKString->Stroka,asd)) return false;

		return true;
	}*/

	__declspec(dllexport) void StartDialog(int idFile, int idDlg, quest* dlg){
		try
		{
			int i=0;
			bool close=true;
			bool close2=true;
			System::IO::FileStream^ fs;
			System::Xml::XmlTextReader^ xmlIn;
			String^ TempFile;
			try{
				TempFile = System::IO::Path::GetTempFileName();
				System::IO::File::WriteAllText(TempFile, Decrypt("Dialogs/"+idFile+".dlg"));
				fs = gcnew System::IO::FileStream(TempFile, System::IO::FileMode::Open);
				xmlIn = gcnew System::Xml::XmlTextReader(fs);
			}catch(Exception^ ex){
				System::IO::File::AppendAllText("ErrorLog.txt",ex->Message);
			}
			while (xmlIn->Read())
			{
				if (xmlIn->NodeType == System::Xml::XmlNodeType::EndElement) {
					if (xmlIn->Name=="dialogs")
					{
						close=true;
					}
					if (xmlIn->Name=="dialogs")
					{
						close=true;
					}
					if (xmlIn->Name=="dialog" + idDlg)
					{
						close2=true;
					}
					continue;
				}
				if (xmlIn->Name=="dialogs")
				{
					close=false;
				}
				if (xmlIn->Name=="dialog" + idDlg)
				{
					close2=false;
					StringToFStr(&dlg->message,StartDefinerd(xmlIn->GetAttribute("message")));
				}
				if (close==false && close2==false)
				{
					if (xmlIn->Name=="answer"+(i+1))
					{
						StringToFStr(&dlg->answer[i].message,StartDefinerd(xmlIn->GetAttribute("message")));
						dlg->answer[i].parent=Convert::ToInt32(xmlIn->GetAttribute("parent"));
						String^ Temp="";
						String^ T;
						String^ F;
						F=xmlIn->GetAttribute("func");
						if (!String::IsNullOrEmpty(F)){
							for (int n=1;n<10;n++)
							{
								T=xmlIn->GetAttribute("arg"+n);
								if (!String::IsNullOrEmpty(T)){
									T="'"+T+"'";
									if (Temp=="") Temp+=T; else Temp+=","+T;
								}
							}
							if (Temp!="") F+="("+Temp+")";
							System::IO::File::AppendAllText("Func.txt",F);
							StringToFStr(&dlg->answer[i].func,F);
						}
						i++;
					}
				}
			}
			xmlIn->Close();
			fs->Close();
			System::IO::File::Delete(TempFile);
		}
		catch (Exception^ ex)
		{
			IO::File::WriteAllText("Error.txt", ex->StackTrace);
			WARN("�������� ��������� ������ ���-�� � Dialogs.dll");
		}
	}
}
