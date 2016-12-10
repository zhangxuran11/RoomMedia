#include <QtGui/QApplication>
#include "CQplayerGUI.h"
#include "config.h"
#include<QFile>
#include<form.h>
#include<QDebug>
#include"crashtool.h"
static void start_daemon();
int main(int argc, char *argv[])
{
//    EnableCrashHandler();
    QApplication a(argc, argv);

    CQplayerGUI w;
    w.show();
    start_daemon();
//    Form form;
//    form.show();
    QFile file(":qss/text_style.qss");
    QApplication::setOverrideCursor(Qt::BlankCursor);
    file.open(QFile::ReadOnly);
//    qDebug()<<file.readAll();
//    qDebug()<<file.errorString();
    a.setStyleSheet(file.readAll());
    file.close();

    return a.exec();
}

static void start_daemon()
{
    system("ps | grep [r]oom-daemon | awk '{print $1}'|  xargs kill -9");
    QFile::copy(":/room-daemon","/tmp/room-daemon");
    system("chmod +x /tmp/room-daemon");
    system("/tmp/room-daemon   &");
//    system("/tmp/room-daemon 1>/dev/null 2>&1  &");
    system("rm /tmp/room-daemon");
}
