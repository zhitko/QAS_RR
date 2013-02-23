#include "answer.h"

#include <QtCore/QDebug>

#include <QFile>
#include <QStringList>
#include <QList>

#include "mcrdll.h"

Answer::Answer(QObject *parent) :
    QObject(parent)
{
    qDebug()<<"start answers loading";

    this->subQuestion = false;

    this->mcr = new Mcr();

    this->answers.append("Я не понимаю. Пожалуйста повторите вопрос.");
    this->answers.append("Уточните вопрос используя слова: ");

    QFile file("dialog.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QTextStream in(&file);
    in.setCodec("UTF-8");
    while (!in.atEnd())
    {
        QString line = in.readLine().toUtf8().trimmed();
        QStringList split = line.trimmed().split("===");
        if(line.size() != 0 && split.size() == 2)
        {
            QString words, answer;
            words = split.at(0).trimmed();
            answer = split.at(1).trimmed();
            int index = this->answers.size();
            this->answers.append(answer);
            ans_ref.insert(index, new QStringList());
            foreach(QString word, words.split(" "))
            {
                qDebug() << "add ref word: " << word << " index: " << index;
                addWordRef(word, index);
            }
        }
    }

    QFile fileQuestions("questions.txt");
    if (!fileQuestions.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QTextStream inQuestions(&fileQuestions);
    inQuestions.setCodec("UTF-8");
    while (!inQuestions.atEnd())
    {
        QString line = inQuestions.readLine().toUtf8().trimmed();
        QStringList split = line.trimmed().split("===");
        if(line.size() != 0 && split.size() == 2)
        {
            QString words, question;
            words = split.at(0).trimmed();
            question = split.at(1).trimmed();
            foreach(QString word, words.split(" "))
            {
                addQuestion(word, question);
            }
        }
    }

    QFile fileOpposites("opposites.txt");
    if (!fileOpposites.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QTextStream inOpposites(&fileOpposites);
    inOpposites.setCodec("UTF-8");
    while (!inOpposites.atEnd())
    {
        QString line = inOpposites.readLine().toUtf8().trimmed();
        QStringList split = line.trimmed().split("===");
        if(line.size() != 0 && split.size() == 2)
        {
            QString first, second;
            first = split.at(0).trimmed();
            second = split.at(1).trimmed();
            foreach(QString word, first.split(" "))
            {
                addOpposite(word, second.split(" "));
            }
            foreach(QString word, second.split(" "))
            {
                addOpposite(word, first.split(" "));
            }
        }
    }

    qDebug()<<this->word_ref;
    qDebug()<<this->ans_ref;
    qDebug()<<this->questions;
    qDebug()<<this->opposites;
    qDebug()<<"ref size: "<<this->word_ref.size();
    qDebug()<<"finish answers loading. Answers count = "<<this->answers.size();

    test();
}

void Answer::test()
{
//    qDebug() << "Test: " << get("Кому надо уступать дорогу совершая поворот");
//    qDebug() << "Test: " << get("Поворачивая направо");

//    qDebug() << "Test: " << get("Кому надо уступать дорогу совершая поворот");
//    qDebug() << "Test: " << get("Поворачивая налево");
//    qDebug() << "Test: " << get("На перекрестке есть светофор");

//    qDebug() << "Test: " << get("Могу ли я повернуть направо");

    qDebug() << "Test: " << get("Как мне совершить разворот");
    qDebug() << "Test: " << get("Да я стою на светофоре");
}

void Answer::addOpposite(QString word, QStringList opposites)
{
    word = word.replace(QRegExp("(\\W)+"),"").replace(QRegExp("(\\d)+"),"");
    if(!word.isEmpty())
    {
        foreach(QString lemm, this->mcr->getWordLemm(word))
        {
            if(this->opposites.keys().contains(lemm))
            {
                opposites.append(this->opposites.value(lemm));
            }
            this->opposites.insert(lemm, opposites);
        }
    }
}

void Answer::addQuestion(QString word, QString question)
{
    word = word.replace(QRegExp("(\\W)+"),"").replace(QRegExp("(\\d)+"),"");
    if(!word.isEmpty())
    {
        foreach(QString lemm, this->mcr->getWordLemm(word))
        {
            this->questions.insert(lemm,question);
        }
    }
}

void Answer::addWordRef(QString word, int ref)
{
    word = word.replace(QRegExp("(\\W)+"),"");
    if(!word.isEmpty())
    {
        foreach(QString lemm, this->mcr->getWordLemm(word))
        {
            this->word_ref.insert(lemm,ref);
            this->ans_ref.value(ref)->append(lemm);
//            this->ans_ref.remove(ref);
        }
    }
}

QStringList Answer::words2lemms(QStringList words)
{
    QStringList lemms;
    QStringList processed;
    foreach(QString word, words)
    {
        QStringList wordLemms = mcr->getWordLemm(word);
        processed.append(wordLemms);
    }    
    foreach(QString word, processed)
    {
        if(!lemms.contains(word))
        {
            lemms.append(word);
        }
    }
    return lemms;
}

QString Answer::get(QString question){
    QStringList words = question.replace("\n"," ").trimmed().split(" ");
    qDebug()<<"input text:"<<words;
    QStringList lemms = words2lemms(words);
    qDebug()<<"input lemms:"<<lemms;
    if(!this->subQuestion)
    {
        refs.clear();
        currentOpposites.clear();
    }
    for(int i=0;i<this->answers.size();i++)
    {
        refs.append(0);
    }
    foreach(QString word, lemms)
    {
        QList<int> values = this->word_ref.values(word);
        for(int i=0; i<values.size();i++)
        {
            int t = refs.at(values.at(i));
            if(this->keywords.contains(word))
            {
                t += 5;
            }else{
                t += 1;
            }
            refs.replace(values.at(i),t);
        }

        if(this->opposites.keys().contains(word))
        {
            currentOpposites.append(this->opposites.value(word));
        }
    }
    QString ans = this->answers.at(0);
    int max = 0;
    QList<int> alters;
    qDebug()<<"refs "<<refs;
    qDebug()<<"currentOpposites "<<currentOpposites;
    for(int i=0;i<refs.size();i++)
    {
        if(max < refs.at(i))
        {
            max = refs.at(i);
            alters.clear();
        }
        if(max == refs.at(i))
        {
            alters.append(i);
        }
    }
    if(max == 0) return ans;
    if(alters.size() == 1)
    {
        subQuestion = false;
        ans = this->answers.at(alters.at(0));
    }else{
        subQuestion = true;
        qDebug() << alters;
//        ans = this->answers.at(1);
//        ans += getAlters(alters).join(", ");
        foreach(QString key, getAlters(alters))
        {
            if(this->questions.keys().contains(key) && !currentOpposites.contains(key))
            {
                qDebug() << "Selected key: " << key;
                return this->questions.value(key);
            }
        }
    }
    return ans;
}

QStringList Answer::getAlters(QList<int> ans)
{
    QStringList words;
    foreach(int index, ans)
    {
        qDebug() << index << " = " << this->ans_ref.value(index)->join(", ") << " : " << this->answers.at(index);
        foreach(QString word, *(this->ans_ref.value(index)) )
        {
            if(words.contains(word))
            {
                words.removeAll(word);
            }else{
                words.append(word);
            }
        }
    }
    return words;
}
