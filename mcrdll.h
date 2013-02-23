#ifndef MCRDLL_H
#define MCRDLL_H

#include <QLibrary>

struct Tinlex;
struct Tinlexdata;
struct Tid;
struct Tids;

class Mcr
{
private:
    QLibrary mcrDll;

    void loadDll(QString fileName);

    // InitMcr() - инициализировать словарь mcr
    typedef int (*p1Func)();
    p1Func pInit;

    // LoadMcr - загрузить словарь mcr
    typedef int (*p2Func)(const char * s);
    p2Func pLoad;

    // SaveMcr - сохранить словарь mcr
    typedef int (*p3Func)(const char * s);
    p3Func pSave;

    // FindID - поиск слова s в словаре mcr, возвращает int количество найденых слов и ids (уникальный идентификтор слова = уникальный идентификатор леммы + номер слова в парадигме)
    // Используйте полученные идентификаторы для получения грамматических характеристик слова или возврата всей парадигмы
    typedef int (*p4Func)(const char * s, Tids * ids);
    p4Func pFind;

    // GetByID - для идентификатора id, функция возвращает грамматические характеристики, лемму или всю парадигмы в outdata
    // :: gh_only = true - возвращать только грамматические характеристики
    // :: gh_only = false - возвратить грамматические характеристики для id и лемму (win1251)
    // :: all = true  - поместить в wout всю парадигму для указанного id с грамматическими характериситками, доступно только для словарей без пометы ReadOnly
    typedef int (*p5Func)(const Tid id,bool gh_only, bool all,Tinlexdata * outdata);
    p5Func pGetBid;

    // ROnly Проверка является ли подключенный словарь - словарем только для чтения (ReadOnlyDict)
    typedef bool (*p6Func)();
    p6Func pROnly;

    // AddPara - добавление парадигмы в словарь
    typedef int (*p7Func)(Tinlexdata * indata);
    p7Func pAdd;

    //3Space - проверка свободного места в словаре, если любой из аргументов близок к 100% то добавление парадигм будет вскоре невозможно
    typedef int (*p8Func)(unsigned char * ar1, unsigned char * ar2,unsigned char * ar3);
    p8Func p3Space;

    // Информация о версии mcr.dll
    typedef int (*p9Func)(char * s);
    p9Func pVer;

    //Строка постоянной грамматической характеристики
    typedef char * (*p10Func)(const unsigned char cid);
    p10Func pCTS;

    //Строка переменной грамматической характеристики
    typedef char * (*p11Func)(const unsigned char cid, const unsigned char vid);
    p11Func pVTS;


    //Нахождение словоформ по заданной грамматической характеристике (только для не ReadOnly словарей)
       //lnk - номер парадигмы
        //const_gh - код постоянной грамматическрй характеристики (const_gh=0 - любая)
        //var_gh - переменная гр. характеристика
        //all=false - вернуть только первую найденную словоформу
    typedef int (*p12Func)(const int lnk, unsigned char const_gh,unsigned char var_gh,const bool all, Tinlexdata * outdata);
    p12Func pBGH;

public:
    Mcr();
    ~Mcr();

    bool loadDictionary(QString filePathName);
    QStringList getWordLemm(QString word);
};

#endif // MCRDLL_H
