// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// Пример использования библиотеки морфоанализа mcr.dll v2.4 (windows)
// 12.10.2004 by Dim	http://macrocosm.narod.ru/madown.html
// Пример для Visual Studio (console application)
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

#include "mcrdll.h"

#include <QtCore/QDebug>
#include <QStringList>

unsigned long tmplink=0;
const unsigned char MAX_WORD_LEN=32;//максимальная длина входного слова
const unsigned char MAX_WORD_COUNT=200;//максимальное количество слов в парадигме

// *** Структура определяющая одно слово и его грамматические характеристики   *** //
struct Tinlex
{
    char anword[MAX_WORD_LEN];//словоформа
    unsigned char cid,vid;  //постоянная и переменная грамматическая характеристика
    char virt;            //optiaonlity of word //опциональность слова если есть ~
    unsigned char para;  //внутренняя системная переменная - явно не используется
};

// *** Общая структура используемая для добавления или считывания парадигмы (слова) ***//
struct Tinlexdata
{
    Tinlex inlex[MAX_WORD_COUNT];
    int count; //количество
};

// *** Cтруктура определяющая уникальный идентификатор (UIN) для _одной_ словоформы ***//
struct Tid
{
    unsigned long lnk; //ссылка на лексему (макс 20 бит)
    unsigned char en;  //ссылка на номер слова в парадигме
};

// ***  Общая структура идентификаторов ***//
struct Tids
{
    Tid ids[MAX_WORD_COUNT];         //lixema uin+ending no
    int count;//количество
};

Mcr::Mcr()
{
    loadDll("mcr.dll");
    loadDictionary("zal.mcr");
}

Mcr::~Mcr()
{
    if(mcrDll.isLoaded())
    {
        mcrDll.unload();
    }
}

void Mcr::loadDll(QString fileName)
{
    qDebug("Loading mcr.dll");
    mcrDll.setFileName(fileName);
    bool isDllLoaded = mcrDll.load();
    if(isDllLoaded)
    {
        qDebug("mcr.dll is loaded");
    }else{
        qDebug("mcr.dll is not loaded");
        return;
    }
    pInit   =  (p1Func)mcrDll.resolve("InitMcr");
    pLoad   =  (p2Func)mcrDll.resolve("LoadMcr");
    pSave   =  (p3Func)mcrDll.resolve("SaveMcr");
    pFind   =  (p4Func)mcrDll.resolve("FindWID");
    pGetBid =  (p5Func)mcrDll.resolve("GetWordById");
    pROnly  =  (p6Func)mcrDll.resolve("ReadOnlyDict");
    pAdd    =  (p7Func)mcrDll.resolve("AddParadigma");
    p3Space =  (p8Func)mcrDll.resolve("FreeSpace");
    pVer    =  (p9Func)mcrDll.resolve("Ver");
    pCTS    = (p10Func)mcrDll.resolve("ConstIdToStr");
    pVTS    = (p11Func)mcrDll.resolve("VarIdToStr");
    pBGH    = (p12Func)mcrDll.resolve("GetBGH");

    if (!(pInit && pLoad && pSave && pFind && pGetBid && pAdd && p3Space && pROnly && pVTS && pCTS && pBGH ))
    {
        qDebug("Error on load dll functions");
        mcrDll.unload();
        return;
    }else{
        qDebug("Dll functions load successful");
    }
}

bool Mcr::loadDictionary(QString filePathName)
{
    if ( (*pLoad)(filePathName.toLocal8Bit().data()) <=0)
    {
        qDebug() << "Error on load dictionaary" << filePathName;
        return false;
    }
    char outstr[255];
    (*pVer)(outstr);
    qDebug() << "Loaded dictionary. Version: " << outstr;

    if ( (*pROnly)() )
    {
        qDebug() << "Loaded dictionary is read only";
    }
    return true;
}

QStringList Mcr::getWordLemm(QString word)
{
    QStringList result;

    int count;
    Tids Uids;
    count = (*pFind)(word.toLocal8Bit().data(), &Uids);
    if (count<=0)
    {
        qDebug() << word;
        result.append(word);
        switch (count)
        {
            case  0:   qDebug("Word not found");
                break;
            case -1:   qDebug("Not cyrillic letters found");
                break;
            case -128: qDebug("Dictionary not initialized");
                break;
            }
    }else{
        Tinlexdata wout;
        for (int i=0; i<count; i++)
        {
            int rc = (*pGetBid)(Uids.ids[i],false,false,&wout);

            if ( i>0 && Uids.ids[i].lnk == Uids.ids[i-1].lnk )
            {
                wout.inlex[0].anword[0]=0;
            }

            if (rc>0)
            {
                QString lemm = QString::fromLocal8Bit(wout.inlex[0].anword);
                if (!lemm.isEmpty() && !result.contains(lemm))
                {
                    result.append(lemm);
                }
            }else{
                switch (rc)
                {
                    case  -1: qDebug("Read Only Dict");
                        break;
                    case  -2:
                    case  -3: qDebug("Bad uid");
                        break;
                }
            }
        }
    }
    return result;
}

//int SaveMCR(char * fn)
//{
//	int rc;

//	say("Сохранение словаря mcr");
//     if ( ( rc = (*pSave)(fn) ) <=0)
//	 {
//		 say("Ошибка сохранения словаря");
//		 switch (rc)
//		 {
//			 case -1:say("Ошибка создания файла");
//				break;
//			 case -2:say("Ошибка сохранения файла");
//				break;
//		 	 case -3:say("Невозможно сохранить словарь ReadOnly");
//				break;

//		 }
//		 return -1;
//	 }
//	 say("Ок");
//	 return 0;
//}


//int ShowParadigm(char * s)
//{
//	int rc,i,j,count;
//	char instr[MAX_WORD_LEN],outstr[255];
//    Tids Uids; //идентификаторы
//	Tinlexdata wout;

//	 say("_________________________________");
//		 strcpy(instr,s);
//		 sprintf(outstr,"Искомое слово '%s'",instr);
//		 say(outstr);
//		 say("_________________________________");
//		 say("№ )Слово - грамматические характериситики [потенциальность]");
//		 say("");
//		 //126 = потенциальная словоформа

//		//нати все идентификаторы для слова
//		 count=(*pFind)(instr, &Uids);
//		 if (count<=0)
//		 {
//			 switch (count)
//			 {
//				case  0:   say("Слово не найдено");
//				 break;
//				case -1:   say("Встречены не русские буквы");
//				 break;
//				case -128: say("Словарь не инициализирован");
//				 break;
//			 }
			 
//		 }
//		  else
//		 for (i=0;i<count;i++)
//		 {

//            // получить информацию о всей парадигме и грамматических характеристиках через идентификатор
//			// результат в wout
//			// вызов функции возможен только для словарей без пометы ReadOnly
//		    rc=(*pGetBid)(Uids.ids[i],false,true,&wout);

//			//пропустить вывод на консоль, если слова от одной и той же леммы
//		    if (i>0) if (Uids.ids[i].lnk==Uids.ids[i-1].lnk) rc=0;

//			 tmplink=Uids.ids[i].lnk;
//		    //Вывод на консоль
//		   if (rc>0)
//		   {
//			   for (j=0;j<wout.count;j++)
//			   {
//			    sprintf(outstr,"%i)%s - %s\t%s [%i]",j+1,wout.inlex[j].anword,(*pCTS)(wout.inlex[j].cid),(*pVTS)(wout.inlex[j].cid,wout.inlex[j].vid), wout.inlex[j].virt);
//				say(outstr);
//			   }
//		   }
//		   else
//		   {
//			 switch (rc)
//			 {
//				case  -1: say("Read Only Dict");
//				 break;
//				case  -2:
//				case  -3: say("Bad uid");
//				 break;
//			 }
//		   }

//		 }
		 
//		 say("_________________________________");

//		 return 0;

//}

//int GetWBGH(unsigned long link, unsigned char const_gh,unsigned char var_gh)
//{
//	int i,count;
//	char outstr[255];
//	Tinlexdata wout;

		 
//		 sprintf(outstr,"Поиск словоформы по заданным грамматическим характеристикам");
//		 say(outstr);
//		 sprintf(outstr,"ID: %i, Const: %i, Var: %i",link,const_gh,var_gh);
//		 say(outstr);

//		 count=(*pBGH)(link, const_gh,var_gh,true, &wout);
//		 if (count<=0)
//		 {
//			 switch (count)
//			 {
//				case  0:   say("Слово не найдено");
//				 break;
//				case -1:   say("Встречены не русские буквы");
//				 break;
//				case -128: say("Словарь не инициализирован");
//				 break;
//			 }
			 
//			 return -1;
//		 }
	  
//		 for (i=0;i<count;i++)
//		 {
//		    sprintf(outstr,"%i)%s - %s\t%s [%i]",i+1,wout.inlex[i].anword,(*pCTS)(wout.inlex[i].cid),(*pVTS)(wout.inlex[i].cid,wout.inlex[i].vid), wout.inlex[i].virt);
//				say(outstr);
//		 }

		 
//		 say("_________________________________");

//		 return 0;

//}


//	//*************************************************
//	//            		Добавление парадигмы в словарь
//	//*************************************************

//int UploadFromTxt(char * fn)
//{
//	FILE *stream;
//	char buf[100];
//	Tinlexdata wout;
//	int rc;

	
	 	 
//	 stream = fopen(fn,"r");//открыть файл
//     if (stream == NULL){
//             say(" Ошибка при открытии файла\n");
//             return -1;
//        }
//	say("Инициализация словаря");
//     (*pInit)();//инициализировать словарь

//	  wout.count=0;
//	//построчное чтение из файла парадигмы и занесение ее в структуру Tinlexdata
//	  while (fgets (buf, MAX_WORD_LEN+5, stream) != NULL)
//	  {
//	   wout.inlex[wout.count].virt=0;
//	   if (buf[0]!=9) //пропускаются строчки комментарии
//	   {
       
//	     if ((rc=sscanf(buf,"%s\t%i\t%i", wout.inlex[wout.count].anword, &wout.inlex[wout.count].cid, &wout.inlex[wout.count].vid)) == 3)
//		 {

//	   	  if (wout.inlex[wout.count].anword[0]=='~')//~ знак потенциальности словоформы, необходимо заносить в virt
//		   {
//			wout.inlex[wout.count].virt='~';
//			strcpy(wout.inlex[wout.count].anword,wout.inlex[wout.count].anword+1);//убрать символ ~
//		   }
//		  wout.count++;

//		 }
//		 else
//		 {
//			 rc=(*pAdd)(&wout);//добавление парадигмы в словарь
//			 wout.count=0;
//			 if (rc>=0) say(".");
//			 else
//			 switch (rc)
//			 {
//				 case  -1:
//				 case  -2:
//				 case  -3:
//				 case  -4:
//				 case  -5:
//					 say("Нет свободного места");
//					 break;
//				 case  -6:
//					 say("Количество добавляемых слов больше допустимого");
//					 break;
//				 case  -9:   say("Пустая парадигма");
//					 break;
//				 case -10:   say("Встречены неверные символы");
//					 break;
//			 }
//		 }

//	   }
//	  } //~while

//	  fclose (stream);//закрыть файл
//	  return 0;
//}

//int main(int argc, char* argv[])
//{	//Hint Правильно укажите пути к файлам
	
//	if (LoadDLL("../mcr.dll")<0)
//		return -1;

//	if (LoadMCR("../zal.mcr")<0) //Подгрузить словарь Зализняка
//	{
//		FreeDLL();
//		return -2;
//	}

//	 FindMyWord("стали");
//	 FindMyWord("село");


//	 if (UploadFromTxt("../mydict.txt")>=0)
//	 {
//        ShowParadigm("интернет");
//       // найти все словоформы с переменной гр.характеристикой=2
//	   //(для сущ. это Р.П. ед. ч)
//        GetWBGH(tmplink,0,2);
//        SaveMCR("../mydict.mcr");
//	 }
	 

   

//    FreeDLL();
//	return 0;
//}


