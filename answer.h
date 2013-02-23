#ifndef ANSWER_H
#define ANSWER_H

#include <QObject>
#include <QList>
#include <QMultiHash>
#include <QHash>
#include <QStringList>

class Mcr;

class Answer : public QObject
{
    Q_OBJECT
public:
    explicit Answer(QObject *parent = 0);

    QString get(QString);

private:
    Mcr * mcr;
    QList<QString> answers;
    QList<QString> keywords;
    QHash<QString, QStringList> opposites;
    QStringList currentOpposites;
    QMultiHash<QString,QString> questions;
    QMultiHash<QString,int> word_ref;
    QHash<int, QStringList *> ans_ref;
    QList<int> refs;

    bool subQuestion;

    void addOpposite(QString word, QStringList opposites);
    void addQuestion(QString word, QString question);
    void addWordRef(QString word, int ref);
    QStringList words2lemms(QStringList words);
    QStringList getAlters(QList<int> ans);

    void test();
    
signals:
    
public slots:
    
};

#endif // ANSWER_H
