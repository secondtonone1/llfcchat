#include "loadingdlg.h"
#include "ui_loadingdlg.h"
#include <QLabel>
#include <QMovie>
#include <QGraphicsOpacityEffect>
#include <QGuiApplication>
#include <QScreen>

LoadingDlg::LoadingDlg(QWidget *parent, QString tip):
    QDialog(parent),
    ui(new Ui::LoadingDlg)
{
    ui->setupUi(this);

    // 1. 让这个 Widget 透明背景、无边框、拦截底部事件
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);// 设置背景透明
    // 2. 让它覆盖父窗口整个面积
    if (parent) {
        // 获取屏幕尺寸
        setFixedSize(parent->size()); // 设置对话框为全屏尺寸
    }

    if (parent) {
        QPoint topLeft = parent->mapToGlobal(QPoint(0, 0));
        move(topLeft);
    }

    // 3. 半透明黑色背景（alpha = 128，大约 50% 透明度）
   // setStyleSheet("background-color: rgba(0, 0, 0, 128);");

    QMovie *movie = new QMovie(":/res/loading.gif"); // 加载动画的资源文件
    ui->loading_lb->setMovie(movie);
    movie->start();
    // 3. 告诉 QMovie：将解码后的每一帧缩放到 label大小
    movie->setScaledSize(ui->loading_lb->size());
    ui->status_lb->setText(tip);
}

LoadingDlg::~LoadingDlg()
{
    delete ui;
}
