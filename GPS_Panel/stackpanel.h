#ifndef STACKPANEL_H
#define STACKPANEL_H

#include <QWidget>
#include <QTableWidget>
#include"dbservice.h"
#include<QTimer>
#include"ztpmanager.h"
#include"checkversionthread.h"
#include"map_panel.h"
#include"mytranslator.h"
namespace Ui {
class StackPanel;
}

class StackPanel : public QWidget
{
    Q_OBJECT
    MAP_Panel* mapPanel;
    CheckVersionThread* checkVersionThread;
    int trainId;
    bool _isHidden;
public:
    bool s_off;
    int secs_off;
    void setSecsOff(int _secs_off)
        {secs_off = _secs_off;}
    MyTranslator* translator;
    explicit StackPanel(MyTranslator* _translator,QWidget *parent = 0);
    void updateLan();
    ~StackPanel();
    void loadTimeTable(int train_id);
    void setTrainId(int _trainId){mapPanel->setTrainId(_trainId);trainId = _trainId;}
    void show(){system("echo high > /tmp/play.txt");QWidget::show();}
    void OnUpdateMap(){mapPanel->OnUpdateMap();}
    void off(){
        s_off = true;
        if(isHidden())
            _isHidden = true;
        else
        {
            _isHidden = false;
            hide();
        }
    }
    void on(){
        s_off = false;
        if(!_isHidden)
            show();
    }
private slots:
    void onRead();
private:
    ZTPManager* ztpm;
    Ui::StackPanel *ui;
    QTableWidget* tableWidget;
};

#endif // STACKPANEL_H
