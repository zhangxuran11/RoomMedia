#ifndef CQGSTBASIC_H
#define CQGSTBASIC_H
#include<QTimer>
#include <QObject>
#include <gstreamer-0.10/gst/gst.h>
#include<QRect>
class CQGstBasic :public QObject
{
    Q_OBJECT

public:
    QRect row_rect;
    int qosMax;
    enum ErrorType
    {
        TIMEOUT,
        FATAL,
        QOS,
        WARNING,
        ERROR,
        STOP
    };
    enum PlayingState
    {
        PLAYING,
        PAUSED,
        STOPPED
    };
    CQGstBasic(int windowID,QWidget* win = 0 ,QObject *parent = NULL);
    ~CQGstBasic();

    void updateOverlay();
    void screenFull();
    void screenNormal();
    PlayingState getPlayingState(){return state;}
    void printDot(const gchar* fileName);//打印dot图
    QString getUrl()const{return url;}
    virtual bool loadURL(const QString& url) = 0;
    void qosAdd(){qos_cnt++;}
    int getQosCnt()const{return qos_cnt;}

    QTimer playOutTimer;
public slots:
    void qosClear();
signals:
    void error(CQGstBasic::ErrorType);
protected:

    GstElement* pipeline;
    PlayingState state;

    QString url;
private:
    int qos_cnt;
    virtual void _updateDecoder() = 0;
    virtual void _updateDemux() = 0;
    const int winID;
    QWidget* barrier;

private slots:


public slots:

    bool paused();
    bool play();
    bool play(const QString& url);
    bool stop();
    bool stopWithoutSignal();
    void emitError(ErrorType type){emit error(type);}
    void _playTimeOutExec();
};


#endif // CQGSTBASIC_H
