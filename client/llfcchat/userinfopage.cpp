#include "userinfopage.h"
#include "ui_userinfopage.h"
#include "usermgr.h"
#include <QDebug>
#include <QMessageBox>
#include "imagecropperdialog.h"
#include "imagecropperlabel.h"
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>

UserInfoPage::UserInfoPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UserInfoPage)
{
    ui->setupUi(this);
    auto icon = UserMgr::GetInstance()->GetIcon();
    qDebug() << "icon is " << icon ;
    QPixmap pixmap(icon);
    QPixmap scaledPixmap = pixmap.scaled( ui->head_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation); // 将图片缩放到label的大小
    ui->head_lb->setPixmap(scaledPixmap); // 将缩放后的图片设置到QLabel上
    ui->head_lb->setScaledContents(true); // 设置QLabel自动缩放图片内容以适应大小
    //获取nick
    auto nick = UserMgr::GetInstance()->GetNick();
    //获取name
    auto name = UserMgr::GetInstance()->GetName();
    //描述
    auto desc = UserMgr::GetInstance()->GetDesc();
    ui->nick_ed->setText(nick);
    ui->name_ed->setText(name);
    ui->desc_ed->setText(desc);
}

UserInfoPage::~UserInfoPage()
{
    delete ui;
}

//上传头像
void UserInfoPage::on_up_btn_clicked()
{
    // 1. 让对话框也能选 *.webp
    QString filename = QFileDialog::getOpenFileName(
        this,
        tr("选择图片"),
        QString(),
        tr("图片文件 (*.png *.jpg *.jpeg *.bmp *.webp)")
    );
    if (filename.isEmpty())
        return;

    // 2. 直接用 QPixmap::load() 加载，无需手动区分格式
    QPixmap inputImage;
    if (!inputImage.load(filename)) {
        QMessageBox::critical(
            this,
            tr("错误"),
            tr("加载图片失败！请确认已部署 WebP 插件。"),
            QMessageBox::Ok
        );
        return;
    }

    QPixmap image = ImageCropperDialog::getCroppedImage(filename, 600, 400, CropperShape::CIRCLE);
    if (image.isNull())
        return;

    QPixmap scaledPixmap = image.scaled( ui->head_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation); // 将图片缩放到label的大小
    ui->head_lb->setPixmap(scaledPixmap); // 将缩放后的图片设置到QLabel上
    ui->head_lb->setScaledContents(true); // 设置QLabel自动缩放图片内容以适应大小

    QString storageDir = QStandardPaths::writableLocation(
                             QStandardPaths::AppDataLocation);
    // 2. 在其下再建一个 avatars 子目录
    QDir dir(storageDir);
    if (!dir.exists("avatars")) {
        if (!dir.mkpath("avatars")) {
            qWarning() << "无法创建 avatars 目录：" << dir.filePath("avatars");
            QMessageBox::warning(
                this,
                tr("错误"),
                tr("无法创建存储目录，请检查权限或磁盘空间。")
            );
            return;
        }
    }
    // 3. 拼接最终的文件名 head.png
    QString filePath = dir.filePath("avatars/head.png");

    // 4. 保存 scaledPixmap 为 PNG（无损、最高质量）
    if (!scaledPixmap.save(filePath, "PNG")) {
        QMessageBox::warning(
            this,
            tr("保存失败"),
            tr("头像保存失败，请检查权限或磁盘空间。")
        );
    } else {
        qDebug() << "头像已保存到：" << filePath;
        // 以后读取直接用同一路径：storageDir/avatars/head.png
    }
}
