#ifndef GOOGLESPEECH_H
#define GOOGLESPEECH_H

#include <QObject>
#include <QFile>

class QNetworkReply;
class QNetworkAccessManager;

class GoogleSpeech : public QObject
{
    Q_OBJECT
public:
    explicit GoogleSpeech(QObject *parent = 0);

    void sent(QString);
    
signals:
    void getText(QString);
    void logging(QString);
    void error();
    
public slots:
    void finishedSlot(QNetworkReply*);

private:
    QNetworkAccessManager* nam;
    QFile * m_file;
    
};

#endif // GOOGLESPEECH_H
