#ifndef MAP_PANEL_H
#define MAP_PANEL_H
#include <QMutex>
#include <QWidget>
#include <QImage>
#include"geogpixsample.h"
#include"dbservice.h"
#include<QPaintEvent>
#include<QRect>
//map type
#define EMPTY_PANEL     -1
#define DETAIL_PANEL    0
#define THUMBNAIL_PANEL 1
class StackPanel;
class ZTPManager;

typedef std::vector<GeogPixSample> SamplesList;
class MAP_Panel : public QWidget
{
    Q_OBJECT
    StackPanel* m_parent;
    SamplesList samples;
    SamplesList samples2; //for thumbnail
    ZTPManager* ztpm;
    const QString mapPath;
    const QString thumbnailPath;
    QImage mapImage;
    QImage thumbnailImage;
    QMutex mapMutex;
    double lng;
    double lat;
    int satelliteNr;
    int train_id;
    bool flush;

public:    
    DBService* dbService;
    int cur_index;
    int map_type;
    MAP_Panel(){}
    explicit MAP_Panel(StackPanel* stackPanel, const QRect& _rect,QWidget *parent,const QString& _mapPath,const QString& _thumbnailPath,const SamplesList &_samples,const SamplesList &_samples2);
    ~MAP_Panel();

    QPointF geog2pix(double lng,double lat);
    QPointF geog2pix2(double lng,double lat);//for thumbnail
    const QImage& getImage();

    void setTrainId(double _train_id)
    {
        if (_train_id == train_id)
            return;
        train_id = _train_id;
        flush = true;
        paintEvent(new QPaintEvent(geometry()));
    }

    double getLng()const {return lng;}
    double getLat()const{return lat;}
    int getSatelliteNr()const{return satelliteNr;}

    void setLng(double _lng){lng = _lng;}
    void setLat(double _lat){lat = _lat;}
    void setSatelliteNr(int _satelliteNr){ satelliteNr = _satelliteNr;}


    void saveImage(int width,int height,const QString& toFileName);
private:
    const SamplesList findSample(double lng,double lat);
    const SamplesList findSample2(double lng,double lat);//for thumbnail
    void drawMap();
private slots:
    void onRead();
    void paintEvent(QPaintEvent *);
public slots:
    void OnUpdateMap();

};

#endif // MAP_PANEL_H





