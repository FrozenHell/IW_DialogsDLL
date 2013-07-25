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
			TEXT("Недопустимая ветка"),	\
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

	struct Var{//Структура переменных, которые раннее были введены
		gcroot<String^> Name;//Имя переменной
		gcroot<String^> Value;//И ее значение
	};

	enum iifOp{//Перечисление операторов функции iif
		r,//Равно
		nr,//Не равно
		b,//Больше
		m,//Меньше
		br,//Больше либо равно
		mr,//Меньше либо равно
		null//Ничего
	};

	// Объявление переменных

	// переменные, используемые в диалогах
	Var *Vars[100];
	
	int NumVars=0;//Кол-во переменных в памяти


	//Объявление функций

	//Описаны будут рядом с каждой функцией
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
	//Конец объявления функций

//Главная функция
	String^ StartDefinerd(String^ str)
	{
		String^ str2=str;//Создание строки, которая и изменяется в конечную
		String^ code;//Переменная, в которой храниться код, заключенный в {...}
		int startInStr=0,//Переменная, в которой находиться число, с которого искать следующий символ "{"
			start=0,//Переменная, в которой находиться следующий символ "{"
			close;//Переменная, в которой находиться следующий символ "}"
		while (start!=-1)//Цикл поиска блоков {...}
		{
			start=inStr(str,startInStr,"{");//Запоминает номер символа, в которой находиться "{", начиная с startInStr
			close=inStr(str,startInStr+1,"}");//Запоминает номер символ в которой находиться "}", начиная с startInStr
			if (start==-1) break;//Если функция InStr возвратила -1, то это значит что символ "{" больше не встречается и выходим из цикла
			code=mid(str,start,close);//Иначе вырезаем сам код, ограниченный {...}
			startInStr=close;//Запоминаем число, с которого в следующий раз будем искать
			str2=str2->Replace("{"+code+"}",ParsingCode(code));//Заменяем {...} на разобранный код ...
		}
		//Тут я думаю все понятно =)
		return str2;
	}


//Собственно функция разбора кода вида {... & ... & ...}
	String^ ParsingCode(String^ Str)
	{
		if (String::IsNullOrEmpty(Str)) return "";//Проверка на Null и путоту
		array <String ^> ^Blocks = gcnew array<String ^> (100);//Массив блоков кода, разделенный символами "&"
		String^ Temp="";//Строка, которую мы изменяем, разбирая код
		//P.S. Она будет встречаться практически в каждой функции, так что дальше ее комментировать не буду

		Str=RemoveSpaces(Str);//Удаляем пробелы в начале и в конце строки
		Blocks=Split(Str,"+");//Разделяем строку символом "&", игнорируя те сиволы, которые находяться внутри скобок и ковычек
		for (int n=0;n<100;n++)//Цикл перебора блоков, разделенных символами "&"
		{
			String^ Func=FunctionDefiction(Blocks[n]);//Определение функции блока
			if (Func=="iif"){//Если функция iif
				Temp+=iif(mid(Blocks[n],4,Blocks[n]->Length));//Добавляем к переменной Temp разобранный код с помощью функции iif
			}	else if (Func=="rand")	{//Если функция rand
				Temp+=randF(mid(Blocks[n],5,Blocks[n]->Length));//Добавляем к переменной Temp разобранный код с помощью функции rand
			}	else	{//Иначе, если задана переменная или константа, то
				Temp+=RetVar(Blocks[n]);//Добавляем к переменной Temp разобранный код с помощью функции RetVar
			}
		}
		return Temp;
	}

//Фунция определения функции

	String^ FunctionDefiction(String^ Str){
		if (String::IsNullOrEmpty(Str)) return "";//Проверка на Null
		Str=RemoveSpaces(Str);
		String^ Temp=mid(Str->Replace(" ",""),0,5);
		if (mid(Str,0,4)=="iif" && mid(Str->Replace(" ",""),0,5)=="iif(") return "iif";
		if (mid(Str,0,5)=="rand" && mid(Str->Replace(" ",""),0,6)=="rand(") return "rand";
		return Str;//Иначе если это ни одна из функций, то возвращает изначальную строку
	}


//Функция разбора оператора iif
	String^ iif(String^ Str)
	{
		iifOp Operator;//Переменная, в которой храниться оператор функции
		array<String^>^ Blocks = Split(Str,",");//Блоки функции iif
		//iif(условие, что возвращать, если условие истинно, что возвращать, если условие ложно)
		String^ A=ParsingCode(Blocks[1]);//блок, когда условие истинно
		String^ B=ParsingCode(Blocks[2]);//блок, когда условие ложно
		//Эти два блока соответственно разбираются с помощью функции ParsingCode, что позволяет использовать принци функции в функции
		//Думаю, что понятно объяснил

		array<String^>^ C;//Блок условия, он разбирается отдельно
		String^ A1;//Блок, находящийся перед оператором условия
		String^ A2;//Блок, находящийся после оператора условия
		Operator=iifOperator(Blocks[0]);//Определение оператора условия

		if(iifOperator(Blocks[0])==r){//Если оператор "Равно"
			C=Split(Blocks[0],"=");//Разделение блока условия по первому символу оператора условия
			A1=ParsingCode(C[0]);//Разбор блока до оператора условия
			A2=ParsingCode(C[1]);//Разбор блока после оператора условия
			if (A1==A2) return A; else return B;	//Возвращаем блок A или блок B в зависимости от условия


		}	else if(iifOperator(Blocks[0])==nr) {//Если оператор "Не равно"
			C=Split(Blocks[0],"<");//Разделение блока условия по первому символу оператора условия
			A1=ParsingCode(C[0]);//Разбор блока до оператора условия
			A2=ParsingCode(mid(C[1],1,C[1]->Length+1));//Разбор блока после оператора условия
			//Функция mid нужна, т.к. оператор условия состоит из 2-х символов
			if (A1!=A2) return A; else return B;		//Возвращаем блок A или блок B в зависимости от условия


		}	else if(iifOperator(Blocks[0])==br) {//Если оператор "Больше, либо равно"
			C=Split(Blocks[0],">");//Разделение блока условия по первому символу оператора условия
			A1=ParsingCode(C[0]);//Разбор блока до оператора условия
			A2=ParsingCode(mid(C[1],1,C[1]->Length+1));//Разбор блока после оператора условия
			if (Convert::ToInt32(A1)>=Convert::ToInt32(A2)) return A; else return B;	//Возвращаем блок A или блок B в зависимости от условия


		}	else if(iifOperator(Blocks[0])==mr) {//Если оператор "Меньше, либо равно"
			C=Split(Blocks[0],"<");//Разделение блока условия по первому символу оператора условия
			A1=ParsingCode(C[0]);//Разбор блока до оператора условия
			A2=ParsingCode(mid(C[1],1,C[1]->Length+1));//Разбор блока после оператора условия
			if (Convert::ToInt32(A1)<=Convert::ToInt32(A2)) return A; else return B;	//Возвращаем блок A или блок B в зависимости от условия


		}	else if(iifOperator(Blocks[0])==b) {//Если оператор "Больше"
			C=Split(Blocks[0],">");//Разделение блока условия по первому символу оператора условия
			A1=ParsingCode(C[0]);//Разбор блока до оператора условия
			A2=ParsingCode(C[1]);//Разбор блока после оператора условия
			if (Convert::ToInt32(A1)>Convert::ToInt32(A2)) return A; else return B;	//Возвращаем блок A или блок B в зависимости от условия


		}	else if(iifOperator(Blocks[0])==m) {//Если оператор "Меньше"
			C=Split(Blocks[0],"<");//Разделение блока условия по первому символу оператора условия
			A1=ParsingCode(C[0]);//Разбор блока до оператора условия
			A2=ParsingCode(C[1]);//Разбор блока после оператора условия
			if (Convert::ToInt32(A1)<Convert::ToInt32(A2)) return A; else return B;	//Возвращаем блок A или блок B в зависимости от условия


		}	else	{//Если оператор "Ничего"
			return "";
		}
		return "";
	}


//Фунция определения опреатора условия
	iifOp iifOperator(String^ Str)
	{
		//Определяется по принципу функции определения функции
		if (Str->Replace("<>","")!=Str) return nr;
		if (Str->Replace("<=","")!=Str) return mr;
		if (Str->Replace(">=","")!=Str) return br;
		if (Str->Replace("<","")!=Str) return m;
		if (Str->Replace(">","")!=Str) return b;
		if (Str->Replace("=","")!=Str) return r;
		return null;//Если ни один оператор не подходит, то возвращаем "НИЧЕГО"
	}

//Функция разбора оператора rand
	String^ randF(String^ Str)
	{
		srand((int)time(0));//Перетусовка
		int n;//Количество блоков, из которых выбирается случайный блок
		array<String^>^ Temp=Split(Str,",");//Массив блоков, из которых выбирается случайный
		for (n=0;n<100;n++)//Определение кол-ва блоков, из которых выбирается случайное
		{
			if (String::IsNullOrEmpty(Temp[n])) break; //Если строка пустая или равна NULL, то выходим из цикла
		}
		return ParsingCode(Temp[rand() %n ]);//Возвращаем разобранную строку
	}

//Функция разбора константы '...', или переменной
	String^ RetVar(String^ Str)
	{
		if (String::IsNullOrEmpty(Str)) return "";//Если строка пустая или равна NULL, то возвращаем путоту
		Str=RemoveSpaces(Str);//Удаляем все пробелы в начале и в конце строки
		try{//Определение, является ли строка числом вида 80
			int ch=Convert::ToInt32(Str);//Пробуем переконвертировать строку в число
										 //, если строка имеет какие то "левые" символ, то произойдет исключение, которое перехватывается
			return Str;//Если строка есть число, то просто возвращаем исходную строку
		}catch(FormatException^){//Перехват исключения
		}
		if ((int)(Str[0])==(int)("'"[0]) && (int)(Str[Str->Length-1])==(int)("'"[0])) {//Определяем, заключена ли  исходная строка в ковычки
			return mid(RemoveSpaces(Str),1,Str->Length);//Если да, то возвращаем строку без ковычек
		}	else	{//Иначе ищем переменную
			for (int n=0;n<NumVars;n++)//Цикл поиска переменных в базе
			{
				if (Vars[n]->Name==Str) return Vars[n]->Value;//Если имя переменной совпадает с исходной строкой, то возвращаем значение переменной
			}
		}
	}

//Функция определения первого вхождения символа

//Str - исходная строка
//start - номер символа, с котрого необходимо искать
//ch - символ, который мы ищем
//
	int inStr(String^ Str, int start, char* ch)
	{
		if (String::IsNullOrEmpty(Str)) return -1;//Если строка пустая или равна NULL, то возвращаем путоту
		int S1=(int)(ch[0]);//Код символа, который ищем
		for(int n=start;n<Str->Length;n++)
		{
			int S=(int)(Str[n]);//Код символа номер n в исходной строке
			if (S==S1) return n+1;//Сравниваем два кода, если они равны, то возвращаем номер символа
		}
		return -1;//Если символ не встречается, то возвращаем -1
	}

//Функция обрезания строки

//Str - исходная строка
//start - начальный символ
//close - конечный символ
//
	String^ mid(String^ Str, int start, int close)
	{
		String^ Temp;
		if (String::IsNullOrEmpty(Str)) return "";//Проверка на пустоту и на NULL
		if (Str->Length<close-start) return Str;//Если длина исходной строки меньше требуемой длины, то возвращаем исходную строку
		for (int n=start;n<close-1;n++)
		{
			Temp+=Str[n];//Иначе по символу прибавляем к переменной Temp
		}
		return Temp;
	}


//Функция разделения строки на разные строки с помощью символа

//Str - исходная строка
//ch - символ
//
	array<String^>^ Split(String^ Str, char* Ch){
		//Не стал раскомментировать, т.к. она очень тяжелая
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

//Функция удаления пробелов в начале и в конце строки
	String^ RemoveSpaces(String^ Str)
	{
		if (String::IsNullOrEmpty(Str)) return "";//Проверка на пустоту и на NULL
		String^ Temp;
		int n=0;//Номер символа
		while ((int)(Str[n])==(int)(" "[0])){//Пока в начале есть пробел
			n+=1;
		}
		int n1=Str->Length-1;//Номер символа в конце строки
		while ((int)(Str[n1])==(int)(" "[0])){//Пока в конце есть пробел
			n1-=1;
		}
		Temp=mid(Str,n,n1+2);//Отделение пробелов от кода
		return Temp;
	}

	bool StringToFStr(FString *exitString, String^ inputString)
	{
		// получаем дешифрованный массив 
		array<unsigned char>^ encodedArray = System::Text::Encoding::GetEncoding(866)->GetBytes(inputString);
		// получаем указатель на массив
		pin_ptr<unsigned char> arrayPointer = &encodedArray[0];
		// переносим указатель
		char* charString = (char*)arrayPointer;
		// узнаём длину строки, которую надо записать
		int stringLength = strlen(charString);
		// если строка слишком длинная
		if (stringLength >= exitString->ArrayMax)
		{	// прекращаем преобразование, не дожидаясь ошибок
			return false;
		}
		// выделяем память под массив Unicode символов
		wchar_t* wideString = new wchar_t[stringLength];
		// конвертируем в Unicode
		MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED, charString,
			stringLength + 1, wideString, stringLength + 1);
		// переносим строку в FString
		wcscpy_s(exitString->Data, exitString->ArrayMax, wideString);
		// устонавливаем конечной строке новую длину
		exitString->UpdateArrayNum();
		// очищаем выделенную память
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
		asd = "Русский текст!";
		// если строка слишком длинная, уведомляем UDK и возвращаем управление ему
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
			WARN("Возможно фатальная ошибка где-то в Dialogs.dll");
		}
	}
}
