#include "fragment.h"

Fragment::Fragment(const QByteArray& bytes)
{
    rawPkg = bytes;
    memcpy(&identifier,bytes.data(),2);
    memcpy(&checksum,bytes.data()+2,2);
    memcpy(&fragment_count,bytes.data()+4,2);
    memcpy(&fragment_offset,bytes.data()+6,2);
    memcpy(&len,bytes.data()+8,4);
    data.append(bytes.data()+12,len);
}


void Fragment::generate()
{
    rawPkg = QByteArray(12,0);
    memcpy(rawPkg.data(),&identifier,2);
    memcpy(rawPkg.data()+4,&fragment_count,2);
    memcpy(rawPkg.data()+6,&fragment_offset,2);
    memcpy(rawPkg.data()+8,&len,4);
    rawPkg.append(data);
    checksum = generateChecksum();
    memcpy(rawPkg.data()+2,&checksum,2);
}

quint16 Fragment::generateChecksum(){
    quint16 sum = 0;
    for(quint32 i = 0;i<len;i++)
    {
        if(i>15 && i <32)
            continue;
        sum += (unsigned char)data[i];
    }
    return sum;
}

FragmentList::FragmentList(quint16 identifier){
            this->identifier = identifier;
//            connect(&timer,SIGNAL(timeout()),this,SLOT(send_timeout())); //内存泄漏隐患
        }
