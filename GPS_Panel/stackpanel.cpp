#include "stackpanel.h"
#include "ui_stackpanel.h"
#include "map_panel.h"
#include<QDebug>
#include<QTableWidget>
#include<QFile>
#include"mytranslator.h"
StackPanel::StackPanel(MyTranslator* _translator ,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StackPanel)
{
    secs_off = 0;
    s_off = false;
    translator = _translator;
    trainId = -1;
    ztpm = new ZTPManager(7676,QHostAddress("224.102.228.40"));
    connect(ztpm,SIGNAL(readyRead()),this,SLOT(onRead()));
    ui->setupUi(this);
    setGeometry(parent->geometry());
    ui->stackedWidget->setGeometry(parent->geometry());
    SamplesList v;
    v.push_back(GeogPixSample(594, 355, 98.998619,-18.784938));
    v.push_back(GeogPixSample(1113,1094, 100.429856,-16.825923));
    v.push_back(GeogPixSample(1178,2284, 100.606705,-13.673508));
    v.push_back(GeogPixSample(1962,718, 102.758951,-17.812939));
    v.push_back(GeogPixSample(1982,1261, 102.811565,-16.389620));
    v.push_back(GeogPixSample(2731,1719,104.872553,-15.175408));
    v.push_back(GeogPixSample(2199,1925, 103.408432,-14.631016));
    v.push_back(GeogPixSample(1132,4765, 100.479170,-6.976231));
    v.push_back(GeogPixSample(631,3762, 99.106193,-9.702211));

    SamplesList v2;
    v2.push_back(GeogPixSample(270,17	, 97.350985,-18.601554));
    v2.push_back(GeogPixSample(416,245	, 100.541089,-13.712088));
    v2.push_back(GeogPixSample(405,573	, 100.236210,-6.726305));
    v2.push_back(GeogPixSample(628,221	, 105.207017,-14.342240));
    v2.push_back(GeogPixSample(513,55, 102.689320,-17.811214));
    v2.push_back(GeogPixSample(342,122, 98.880545,-16.420863));
    v2.push_back(GeogPixSample(347,377, 99.018984,-10.978673));
    mapPanel = new MAP_Panel(this,QRect(0,0,ui->stackedWidget->width(),ui->stackedWidget->height()),ui->stackedWidget,"/srv/tftp/GPSMap.png","/srv/tftp/thumbnail.jpg",v,v2);
    mapPanel->setLat(-13.673508);//DEBUG
    mapPanel->setLng(100.606705);//DEBUG
    mapPanel->setSatelliteNr(0);
    ui->stackedWidget->addWidget(mapPanel);
    checkVersionThread = new CheckVersionThread;
    connect(checkVersionThread,SIGNAL(updateMap()),mapPanel,SLOT(OnUpdateMap()));
    checkVersionThread->start();

    QFile file(":/tablewidget_stylesheet.qss");
    file.open(QFile::ReadOnly);
    QString styleText = file.readAll();
    file.close();
    tableWidget = new QTableWidget;

    tableWidget->setStyleSheet(styleText);

    tableWidget->setGeometry(ui->stackedWidget->geometry());
    tableWidget->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    tableWidget->setAlternatingRowColors(true);
    tableWidget->verticalHeader()->setHidden(true);
    tableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->stackedWidget->addWidget(tableWidget);

    loadTimeTable(1);//初始化表头
    ui->stackedWidget->setCurrentIndex(0);
    //test
    if(0)
    {
        loadTimeTable(1);
        ui->stackedWidget->setCurrentIndex(1);
    }

}
void StackPanel::onRead()
{
    ZTPprotocol ztp;
    ztpm->getOneZtp(ztp);
    if(ztp.getPara("T") == "MapTable")
    {
        //ztp.print();
        #if 0
        ztp.addPara("flag","map");
        ztp.addPara("map_type","detail");
        #endif
        if(ztp.getPara("flag") == "table")
        {
            loadTimeTable(trainId);
            ui->stackedWidget->setCurrentIndex(1);
        }
        else if(ztp.getPara("flag") == "map")
        {
            if(ztp.getPara("map_type") == "detail")
            {
                mapPanel->map_type = DETAIL_PANEL;
            }
            else if(ztp.getPara("map_type") == "thumbnail")
            {
                mapPanel->map_type = THUMBNAIL_PANEL;
            }
            ui->stackedWidget->setCurrentIndex(0);
            ui->stackedWidget->update();
        }
    }
}
void StackPanel::loadTimeTable(int train_id)
{
    tableWidget->clear();
    tableWidget->setRowCount(0);
    tableWidget->setColumnCount(4);
    QTableWidgetItem* item = new QTableWidgetItem(translator->tr("Station number"));
    item->setFlags(Qt::NoItemFlags);
    item->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    item->setSizeHint(QSize(200,40));
    tableWidget->setHorizontalHeaderItem(0,item);

    item = new QTableWidgetItem(translator->tr("Station name"));
    item->setFlags(Qt::NoItemFlags);
    item->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    item->setSizeHint(QSize(200,40));
    tableWidget->setHorizontalHeaderItem(1,item);

    item = new QTableWidgetItem(translator->tr("Arrival time"));
    item->setFlags(Qt::NoItemFlags);
    item->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    item->setSizeHint(QSize(200,40));
    tableWidget->setHorizontalHeaderItem(2,item);

    item = new QTableWidgetItem(translator->tr("Strat time"));
    item->setFlags(Qt::NoItemFlags);
    item->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    item->setSizeHint(QSize(200,40));
    tableWidget->setHorizontalHeaderItem(3,item);

    int index = mapPanel->cur_index - 8;
    if(index < 1)
        index = 1;
    QString sql = QString("select indexNu,station_name_en,station_name_th,arrive_time,start_time from tb_station_info where train_id=%1 order by indexNu limit %2,17").arg(train_id).arg(index-1);
    ResultSet rowSet = mapPanel->dbService->query(sql);
    tableWidget->setRowCount(rowSet.count());
    for(int i = 0;i < rowSet.count();i++)
    {
        bool flag = false;
        if( i == mapPanel->cur_index - index)
            flag = true;
        QTableWidgetItem* item = new QTableWidgetItem(rowSet[i].getPara("indexNu"));
        if(flag)
            item->setTextColor(Qt::red);
        item->setFlags(Qt::NoItemFlags);
        item->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        tableWidget->setItem(i,0,item);
        if(translator->curLan == "En")
            item = new QTableWidgetItem(rowSet[i].getPara("station_name_en"));
        else
            item = new QTableWidgetItem(rowSet[i].getPara("station_name_th"));
        if(flag)
            item->setTextColor(Qt::red);
        item->setFlags(Qt::NoItemFlags);
        item->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        tableWidget->setItem(i,1,item);
        QString str = rowSet[i].getPara("arrive_time");
        QTime t = QTime::fromString(str,"hh:mm");
        if(t.isValid())
        {
            t = t.addSecs(secs_off);
            str = t.toString("hh:mm");
        }
        item = new QTableWidgetItem(str);
        if(flag)
            item->setTextColor(Qt::red);
        item->setFlags(Qt::NoItemFlags);
        item->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        tableWidget->setItem(i,2,item);
        str = rowSet[i].getPara("start_time");
        t = QTime::fromString(str,"hh:mm");
        if(t.isValid())
        {
            t = t.addSecs(secs_off);
            str = t.toString("hh:mm");
        }
        item = new QTableWidgetItem(str);
        if(flag)
            item->setTextColor(Qt::red);
        item->setFlags(Qt::NoItemFlags);
        item->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        tableWidget->setItem(i,3,item);

    }

}
StackPanel::~StackPanel()
{
    delete ui;
}
void StackPanel::updateLan()
{
    if(!isHidden()){
        loadTimeTable(trainId);
        ui->stackedWidget->update();
    }

}
