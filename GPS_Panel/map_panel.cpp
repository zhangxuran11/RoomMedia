#include "map_panel.h"
#include"ztpmanager.h"
#include<QHostAddress>
#include<QFile>
#include<QPainter>
#include<math.h>
#include<QtCore>
#include<stackpanel.h>
MAP_Panel::MAP_Panel(StackPanel* stackPanel,const QRect& _rect,QWidget *parent,const QString& _mapPath,const QString& _thumbnailPath,const SamplesList &_samples,const SamplesList &_samples2) :
    QWidget(parent), samples(_samples),samples2(_samples2),mapPath(_mapPath),thumbnailPath(_thumbnailPath)
{
    m_parent = stackPanel;
    QTime time;
    time.start();
    QThreadPool::globalInstance()->setMaxThreadCount(QThreadPool::globalInstance()->maxThreadCount()+1);
    lng = 0;
    lat = 0;
    satelliteNr = 0;
    cur_index = -1;
    flush = false;
    map_type = EMPTY_PANEL;
    train_id = -1;
    setGeometry(_rect);
    hide();
    ztpm = new ZTPManager(7878,QHostAddress("224.102.228.40"));
    connect(ztpm,SIGNAL(readyRead()),this,SLOT(onRead()));
    mapImage.load(mapPath);//1 s
    thumbnailImage.load(thumbnailPath); //100ms
    dbService = new DBService("/srv/tftp/TrainLine.db","MapPanelConnName");// 2ms
}

MAP_Panel::~MAP_Panel()
{
    delete ztpm;
}
void MAP_Panel::onRead()
{
    ZTPprotocol ztp;
    ztpm->getOneZtp(ztp);
    if(this->parentWidget()->isHidden())
        return;
    if(ztp.getPara("T") == "GPS_map")
    {
        //ztp.print();
        double lat = ztp.getPara("lat").toDouble();
        double lng = ztp.getPara("lng").toDouble();
        if(lat > -20.499064 && lat < -5.648786 &&
                lng > 97.283936 && lng < 105.611572)
        {
            setLat(lat);
            setLng(lng);
        }
        setSatelliteNr(ztp.getPara("satelliteNr").toInt());
        if(ztp.getPara("cur_index").toInt() != -1)
            cur_index = ztp.getPara("cur_index").toInt();
        else
            return;
        update();
    }
}
static ResultSet stationSet;
void MAP_Panel::drawMap()
{
    static QColor colors[5] = {QColor(Qt::magenta),QColor(Qt::blue),QColor(Qt::cyan),QColor(Qt::green),QColor(Qt::yellow)};
    mapMutex.lock();
    mapImage.load(mapPath);
    if(mapImage.isNull())
    {
        qDebug("/srv/tftp/GPSMap.png is not exsists.");
        mapMutex.unlock();
        return;
    }
    QPainter painter;
    delete dbService;
    dbService = new DBService("/srv/tftp/TrainLine.db","MapPanelConnName");
    QString sql = QString("select lng,lat from tb_railway_info where train_id=%1").arg(train_id);
    ResultSet sampleSet = dbService->query(sql);
//    qDebug()<<"drawMap 2222222222222222222222222222";
    painter.begin(&mapImage);
    qDebug()<<"sampleSet length = "<<sampleSet.count();
    for(int j = 0;j < sampleSet.count();j++)
    {
        ResultRow sampleRow = sampleSet[j];
        QPen pen(painter.pen());
        pen.setWidth(5);
        pen.setColor(colors[1]);
        painter.setPen(pen);
        QPointF p = geog2pix(sampleRow.getPara("lng").toDouble(),sampleRow.getPara("lat").toDouble());
        painter.drawPoint(p);
    }
    painter.end();
//    QString imageName = QString("/tmp/%1.png").arg(train_id);
//    mapImage.save(imageName,"PNG");
    sql = QString("select indexNu,lng,lat,station_name_en,station_name_th from tb_station_info where train_id=%1 order by indexNu").arg(train_id);
    stationSet = dbService->query(sql);
    flush = false;
    mapMutex.unlock();
    update();
}
#if 0
static void drawSignalBars(QPainter &painter,float x,float y,float rectW,float rectH,int maxBar)
{
    QPen penTmp(painter.pen());
    penTmp.setColor(QColor(Qt::white));
    painter.setPen(penTmp);
    int startAngle = 30 * 16;
    int spanAngle = 120 * 16;

    QRectF rectTmp(x, y-10, rectW, rectW);
    painter.setBrush(QBrush(Qt::black));
    painter.drawRect(rectTmp);
    for(int i = 0;i < maxBar;i++)
    {
        float rectWW = rectW * i / maxBar;
        float rectHH = rectH * i / maxBar;
        float xx = x + (rectW - rectWW)/2;
        float yy = y + (rectH - rectHH)/2;
        QRectF rectangle(xx, yy, rectWW, rectHH);
        painter.drawArc(rectangle, startAngle, spanAngle);
    }
}
#endif
#if 1
static void drawSignalBars(QPainter &painter,float x,float y,float rectW,float rectH,int maxBar,int sig)//信号强弱  0～4
{
    int barW = 6;
//    int barH = (rectH*maxBar)/(2*(maxBar+1));
    int barH = rectH/(2*(maxBar+1));    //最小bar宽高
    QPen penTmp(painter.pen());
    penTmp.setColor(QColor(Qt::black));
    painter.setPen(penTmp);
    painter.setBrush(QBrush(Qt::black));

    for(int i = 1;i <= maxBar;i++)
    {
        float barWW = barW;
        float barHH = barH*i;
        float xx = x + (rectW - maxBar*barW)/(maxBar+1)*i+(i-1)*barWW;
        float yy = y + rectH/2-barHH;
        QRectF rectangle(xx, yy, barWW, barHH);
        if(i == sig+1)
            painter.setBrush(QBrush(Qt::white));
        painter.drawRect(rectangle);
    }
}
#endif
void MAP_Panel::OnUpdateMap()
{
    QtConcurrent::run(this,&MAP_Panel::drawMap);
}
void MAP_Panel::paintEvent(QPaintEvent *)
{
    QPainter painter;
    static QImage posTrain = QImage(":/images/train.png");
    static QImage posTrain2 = QImage(":/images/train2.png");
#if 0
    train_id =11;
    flush = true;
#endif
    if(train_id == -1)
        return;
    if(map_type == DETAIL_PANEL)
    {
        if(!mapMutex.tryLock())
            return;
        if(mapImage.isNull())
        {
            qDebug("/srv/tftp/GPSMap.png is not exsists.");
            mapMutex.unlock();
            return;
        }
        QPointF p = geog2pix(lng,lat);
        int _width = width()>>1,_height = height()>>1;
        QRect rect(p.x()-_width/2,p.y()-_height/2, _width,_height);


        QImage img = mapImage.copy(rect);
        img = img.scaled(img.width()<<1,img.height()<<1);

        painter.begin(&img);

        for(int j = 0;j < stationSet.count();j++)
        {
            ResultRow stationRow = stationSet[j];
            QPen pen(painter.pen());
            pen.setWidth(3);
            pen.setColor(QColor(Qt::green));
            painter.setPen(pen);
            painter.setBrush(QBrush(Qt::green,Qt::SolidPattern));


            QPointF p = geog2pix(stationRow.getPara("lng").toDouble(),stationRow.getPara("lat").toDouble());
            QPoint pp(p.x(),p.y());
            if(!rect.contains(pp))
                continue;
            QString str;
            if(m_parent->translator->curLan == "En")
                str = QString("%1.%2").arg(stationRow.getParaInt("indexNu")).arg(stationRow.getPara("station_name_en"));
            else
                str = QString("%1.%2").arg(stationRow.getParaInt("indexNu")).arg(stationRow.getPara("station_name_th"));
            QPoint point = QPoint((pp.x() - rect.x())<<1,(pp.y() - rect.y())<<1);
            painter.drawEllipse(point,4,4);
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(point,8,8);
            QPen pen2(painter.pen());
            pen2.setColor(QColor(Qt::red));
            painter.setPen(pen2);
            painter.setFont(QFont("Times", 10, QFont::Bold));
            painter.drawText(((pp.x() - rect.x())<<1)+12,((pp.y() - rect.y())<<1)+3,str);
        }
        painter.drawImage(width()/2-30,height()/2-14,posTrain);
        //画信号强度
        float rectW = 50.0;
        float rectH = 60.0;
        float x = rectW * (width()/rectW-1);
        float y = rectH * 0;
        QPen pen2(painter.pen());
        pen2.setColor(QColor(Qt::black));
        painter.setPen(pen2);
        painter.setFont(QFont("Times", 20, QFont::Bold));
        painter.drawText(x - rectW-10,rectH/2,"GPS:");


        drawSignalBars(painter,x,y,rectW,rectH,3,getSatelliteNr()/3);

        //画信号强度

        painter.end();

        painter.begin(this);
        painter.drawImage(0,0,img);
        painter.end();
        mapMutex.unlock();
    }
    else if(map_type == THUMBNAIL_PANEL)
    {
        if(thumbnailImage.isNull())
        {
            qDebug("/srv/tftp/thumbnail.jpg is not exsists.");
            return;
        }
        QPointF p = geog2pix2(lng,lat);
        QImage img = thumbnailImage.scaled(width(),height());
        painter.begin(&img);
        painter.scale(width()*1.0/thumbnailImage.width(),height()*1.0/thumbnailImage.height());
        QPen pen(painter.pen());
        pen.setWidth(5);
        pen.setColor(QColor(Qt::red));
        painter.setPen(pen);
        QImage train2 = posTrain2.scaled(posTrain2.width()/2,posTrain2.height()/2);
        painter.drawImage(p.x()-train2.width()/2+2,p.y()-train2.width(),train2);

        painter.end();

        painter.begin(this);
        painter.drawImage(0,0,img);
        painter.end();
    }
}
QPointF MAP_Panel::geog2pix(double lng,double lat)
{
    const SamplesList refPoints = findSample(lng,lat);
    double x_lng_factor = (refPoints[1].pix_x-refPoints[0].pix_x)/(refPoints[1].lng-refPoints[0].lng);
    double y_lat_factor = (refPoints[1].pix_y-refPoints[0].pix_y)/(refPoints[1].lat-refPoints[0].lat);

    double x = refPoints[0].pix_x + (lng - refPoints[0].lng) * x_lng_factor;
    double y = refPoints[0].pix_y + (lat - refPoints[0].lat) * y_lat_factor;
    return QPointF(x,y);
}
QPointF MAP_Panel::geog2pix2(double lng,double lat)
{
    const SamplesList refPoints = findSample2(lng,lat);
    double x_lng_factor = (refPoints[1].pix_x-refPoints[0].pix_x)/(refPoints[1].lng-refPoints[0].lng);
    double y_lat_factor = (refPoints[1].pix_y-refPoints[0].pix_y)/(refPoints[1].lat-refPoints[0].lat);

    double x = refPoints[0].pix_x + (lng - refPoints[0].lng) * x_lng_factor;
    double y = refPoints[0].pix_y + (lat - refPoints[0].lat) * y_lat_factor;
    return QPointF(x,y);
}

const SamplesList MAP_Panel::findSample(double lng,double lat)
{
    SamplesList v;
    GeogPixSample* sample1 = &samples[0];
    GeogPixSample* sample2 = &samples[1];
    double dist1 = 180,dist2 = 180;
    for(SamplesList::iterator it = samples.begin(); it != samples.end(); it++)
    {
        double dist = sqrt(pow(it->lat - lat,2)+pow(it->lng - lng,2));
        if (dist < dist1)
        {
            dist2 = dist1;
            dist1 = dist;
            sample2 = sample1;
            sample1 = &(*it);
        }
        else if(dist < dist2)
        {
            dist2 = dist;
            sample2 = &(*it);
        }

    }
    v.push_back(*sample1);
    v.push_back(*sample2);
    return v;
}
const SamplesList MAP_Panel::findSample2(double lng,double lat)
{
    SamplesList v;
    GeogPixSample* sample1 = &samples2[0];
    GeogPixSample* sample2 = &samples2[1];
    double dist1 = 180,dist2 = 180;
    for(SamplesList::iterator it = samples2.begin(); it != samples2.end(); it++)
    {
        double dist = sqrt(pow(it->lat - lat,2)+pow(it->lng - lng,2));
        if (dist < dist1)
        {
            dist2 = dist1;
            dist1 = dist;
            sample2 = sample1;
            sample1 = &(*it);
        }
        else if(dist < dist2)
        {
            dist2 = dist;
            sample2 = &(*it);
        }

    }
    v.push_back(*sample1);
    v.push_back(*sample2);
    return v;
}
const QImage& MAP_Panel::getImage()
{
    return mapImage;
}
void MAP_Panel::saveImage(int width,int height,const QString& toFileName)
{
    QImage img(mapImage.copy(geog2pix(lng,lat).x()-width/2,geog2pix(lng,lat).y()-height/2, width,height));
    QPainter painter;
    painter.begin(&img);
    QPen pen(painter.pen());
    pen.setWidth(10);
    pen.setColor(QColor(Qt::red));
    painter.setPen(pen);
    painter.drawPoint(width/2,height/2);
    painter.end();

    img.save(toFileName);
}


