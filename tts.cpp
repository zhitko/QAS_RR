#include "tts.h"
#include <QtCore/QDebug>

TTS::TTS(QObject *parent) :
    QObject(parent)
{
    QtSpeech::VoiceName voice = QtSpeech::voices().first();
    foreach(QtSpeech::VoiceName v, QtSpeech::voices()){
        qDebug() << "id:" << v.id << "name:" << v.name;
        if(v.name=="Boris Lobanov") voice = v;
    }
    qDebug() << "create tts:" << voice.name;
    this->speech = new QtSpeech(voice);
    connect(this->speech, SIGNAL(finished()), this, SLOT(_finished()));
}

TTS::~TTS(){
    delete this->speech;
}

void TTS::say(QString data){
//    QtSpeech speech;
    qDebug() << "About to say synchrounously" << data << "using voice:" << speech->name().name;
//    speech->say(data);
    speech->tell(data);
    qDebug() << "Finish tts say";

//    qDebug() << "About to say asynchrounously" << data << "using voice:" << speech.name().name;
//    speech.tell(data);
}

void TTS::_finished()
{
    emit finished();
}
