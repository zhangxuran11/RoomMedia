#ifndef CQPLAYERGUI_H
#define CQPLAYERGUI_H

#include <QDialog>
#include "CQmedia.h"
#include <iostream>
#include <QString>
#include <ui_CQplayerGUI.h>
#include <QFileDialog>
#include <QThread>
#include "callctlunit.h"
#include"stackpanel.h"
#include"versionsender.h"
#include"cqaudio.h"
#include "mytranslator.h"
class GstThread;
class RoomPanel;
class CQMedia;
class MAP_Panel;
class CQplayerGUI : public QDialog
{
    Q_OBJECT
    ZTPManager* ztpmForTest;
    QTimer* switchLanguage;

    QTimer *updateTimer; //更新时间标签
public:
    MyTranslator* translator;
    VersionSender* versionCtrl;
    enum EToggle{ON,OFF};
    enum PlayState{PLAYING,STOP,PAUSED};
    explicit CQplayerGUI(QWidget *parent = 0);
    ~CQplayerGUI();
private:
    EToggle screen_toggle;//true 开 false 关
    int channel;

public:
    Ui::CQplayerGUI* ui;
    CQMedia *media;
    CQAudio* cqAudio;
    StackPanel* stackPanel;


    EToggle getScreenToggle()const{return screen_toggle;}
    PlayState getPlayState()const{return playState;}
    void screenSwitch(EToggle toggle);
    int getCurrentChannel()const{return channel;}
    void play(int _channel);
    bool playback(int _channel);
    void paused();
    QPushButton* btn[9];

private:
    RoomPanel *serial;
    PlayState playState;

    void screenInit();

public slots:

    void onMediaSignal(CQGstBasic::ErrorType type);

    void onPlayButton_clicked();
    void onStopButton_clicked();
private slots:
    void refresh();
    void sendHeart();
    void slot_turnOffHeart();

    void updateTime();
    void stop();
    void on_recipeButton_clicked();
    void slot_disableTime();
    void on_btn_ch1_clicked();

    void on_btn_ch2_clicked();
    void on_btn_ch3_clicked();
    void on_btn_ch4_clicked();
    void on_btn_ch5_clicked();
    void on_btn_ch6_clicked();
    void on_btn_ch7_clicked();
    void on_btn_ch8_clicked();

    void on_btn_off_clicked();

    void on_btn_ch0_clicked();

    void on_sliderVlm_valueChanged(int position);

    void slot_procTestZtp();

    void on_btn_scr_full_clicked();

public slots:

    void updateDaemon();
    void clickStateReset();
    void mouseReleaseEvent( QMouseEvent * event );
    void touchAdjust(QMouseEvent *e);
};

#endif // CQPLAYERGUI_H





