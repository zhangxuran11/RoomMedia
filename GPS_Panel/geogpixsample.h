#ifndef GEOGPIXSAMPLE_H
#define GEOGPIXSAMPLE_H

class GeogPixSample
{
    double pix_x;
    double pix_y;
    double lng;
    double lat;
    friend class MAP_Panel;
public:
    GeogPixSample(){}
    GeogPixSample(double _pix_x,double _pix_y,double _lng,double _lat);
    bool operator<(const GeogPixSample& sample)const{return lat<sample.lat;}
};

#endif // GEOGPIXSAMPLE_H
