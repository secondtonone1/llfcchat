---
title: QT实现头像裁剪功能
date: 2025-05-11 06:39:29
tags: [C++聊天项目]
categories: [C++聊天项目] 
---

## 前情回顾

前文我们实现了心跳，今天来实现头像框裁剪的功能，为以后头像上传和资源服务器做准备。

大体上头像上传框的效果如下

![image-20250511075018888](https://cdn.llfc.club/img/image-20250511075018888.png)

## 添加设置页面

我们需要在聊天对话框左侧添加设置按钮

![image-20250511075548367](https://cdn.llfc.club/img/image-20250511075548367.png)

左侧设置按钮是我们封装的类`StateWidget`

![image-20250511075648519](https://cdn.llfc.club/img/image-20250511075648519.png)

右侧添加`UserInfoPage`界面

![image-20250511075822544](https://cdn.llfc.club/img/image-20250511075822544.png)

`UserInfoPage`界面布局

![image-20250511082150907](https://cdn.llfc.club/img/image-20250511082150907.png)

属性表

![image-20250511082230811](https://cdn.llfc.club/img/image-20250511082230811.png)

## 头像裁剪逻辑

### 点击上传按钮

``` cpp
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

```

内部调用了我们的`ImageCropperDialog`，弹出对话框后会显示裁剪图片的界面。

接下来我们看看`ImageCropperDialog`实现

``` cpp
#ifndef IMAGECROPPER_H
#define IMAGECROPPER_H

#include <QWidget>
#include <QDialog>
#include <QPainter>
#include <QLabel>
#include <QPixmap>
#include <QString>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>

#include "imagecropperlabel.h"

/*******************************************************
 *  Loacl private class, which do image-cropping
 *  Used in class ImageCropper
*******************************************************/
class ImageCropperDialogPrivate : public QDialog {
    Q_OBJECT
public:
    ImageCropperDialogPrivate(const QPixmap& imageIn, QPixmap& outputImage,
                              int windowWidth, int windowHeight,
                              CropperShape shape, QSize cropperSize = QSize()) :
        QDialog(nullptr),  outputImage(outputImage)
    {
        this->setAttribute(Qt::WA_DeleteOnClose, true);
        this->setWindowTitle("Image Cropper");
        this->setMouseTracking(true);
        this->setModal(true);

        imageLabel = new ImageCropperLabel(windowWidth, windowHeight, this);
        imageLabel->setCropper(shape, cropperSize);
        imageLabel->setOutputShape(OutputShape::RECT);
        imageLabel->setOriginalImage(imageIn);
        imageLabel->enableOpacity(true);

        QHBoxLayout* btnLayout = new QHBoxLayout();
        btnOk = new QPushButton("OK", this);
        btnCancel = new QPushButton("Cancel", this);
        btnLayout->addStretch();
        btnLayout->addWidget(btnOk);
        btnLayout->addWidget(btnCancel);

        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->addWidget(imageLabel);
        mainLayout->addLayout(btnLayout);

        connect(btnOk, &QPushButton::clicked, this, [this](){
            this->outputImage = this->imageLabel->getCroppedImage();
            this->close();
        });
        connect(btnCancel, &QPushButton::clicked, this, [this](){
            this->outputImage = QPixmap();
            this->close();
        });
    }

private:
    ImageCropperLabel* imageLabel;
    QPushButton* btnOk;
    QPushButton* btnCancel;
    QPixmap& outputImage;
};


/*******************************************************************
 *  class ImageCropperDialog
 *      create a instane of class ImageCropperDialogPrivate
 *      and get cropped image from the instance(after closing)
********************************************************************/
class ImageCropperDialog : QObject {
public:
    static QPixmap getCroppedImage(const QString& filename,int windowWidth, int windowHeight,
                                   CropperShape cropperShape, QSize crooperSize = QSize())
    {
        QPixmap inputImage;
        QPixmap outputImage;

        if (!inputImage.load(filename)) {
            QMessageBox::critical(nullptr, "Error", "Load image failed!", QMessageBox::Ok);
            return outputImage;
        }

        ImageCropperDialogPrivate* imageCropperDo =
            new ImageCropperDialogPrivate(inputImage, outputImage,
                                          windowWidth, windowHeight,
                                          cropperShape, crooperSize);
        imageCropperDo->exec();

        return outputImage;
    }
};



#endif // IMAGECROPPER_H

```

### 私有对话框

1. **继承自 `QDialog`**
   - `QDialog(nullptr)`：以无父窗口方式创建，独立弹出。
   - `Qt::WA_DeleteOnClose`：关闭时自动 `delete` 对象，防止内存泄漏。
   - `setModal(true)`：对话框模式，阻塞主窗口输入。
2. **成员变量**
   - `ImageCropperLabel* imageLabel`：自定义裁剪视图。
   - `QPushButton* btnOk`, `btnCancel`：确认/取消按钮。
   - `QPixmap& outputImage`：引用外部提供的 `QPixmap`，用来保存裁剪结果。
3. **布局管理**
   - 水平布局 (`QHBoxLayout`) 放置按钮并居右。
   - 垂直布局 (`QVBoxLayout`) 先是大图，再是按钮区。
4. **Lambda 连接信号与槽**
   - OK 时，将裁剪后的图像复制给外部引用，然后 `close()`。
   - Cancel 时，将 `outputImage` 置空，表示用户放弃裁剪。

### 静态对话框

- **统一接口**：只要一行 `ImageCropperDialog::getCroppedImage(…)`，就能弹出裁剪 UI 并获取结果。
- **输入合法性检查**：先用 `QPixmap::load()` 加载文件，失败则弹错并返回空图。
- **阻塞执行**：`exec()` 会进入本地事件循环，直到用户点击 OK/Cancel 关闭对话框。
- **返回结果**：通过外部引用 `outputImage` 将裁剪结果“带出”函数作用域。



![image-20250511112606921](https://cdn.llfc.club/img/image-20250511112606921.png)

### 头像裁剪控件

头文件声明

``` cpp
/*************************************************************************
 *  class:          ImageCropperLabel
 *  author:         github@Leopard-C
 *  email:          leopard.c@outlook.com
 *  last change:    2020-03-06
*************************************************************************/
#ifndef IMAGECROPPERLABEL_H
#define IMAGECROPPERLABEL_H

#include <QLabel>
#include <QPixmap>
#include <QPen>

enum class CropperShape {
    UNDEFINED     = 0,
    RECT          = 1,
    SQUARE        = 2,
    FIXED_RECT    = 3,
    ELLIPSE       = 4,
    CIRCLE        = 5,
    FIXED_ELLIPSE = 6
};

enum class OutputShape {
    RECT    = 0,
    ELLIPSE = 1
};

enum class SizeType {
    fixedSize           = 0,
    fitToMaxWidth       = 1,
    fitToMaxHeight      = 2,
    fitToMaxWidthHeight = 3,
};


class ImageCropperLabel : public QLabel {
    Q_OBJECT
public:
    ImageCropperLabel(int width, int height, QWidget* parent);

    void setOriginalImage(const QPixmap& pixmap);
    void setOutputShape(OutputShape shape) { outputShape = shape; }
    QPixmap getCroppedImage();
    QPixmap getCroppedImage(OutputShape shape);

    /*****************************************
     * Set cropper's shape
    *****************************************/
    void setRectCropper();
    void setSquareCropper();
    void setEllipseCropper();
    void setCircleCropper();
    void setFixedRectCropper(QSize size);
    void setFixedEllipseCropper(QSize size);
    void setCropper(CropperShape shape, QSize size);    // not recommended

    /*****************************************************************************
     * Set cropper's fixed size
    *****************************************************************************/
    void setCropperFixedSize(int fixedWidth, int fixedHeight);
    void setCropperFixedWidth(int fixedWidht);
    void setCropperFixedHeight(int fixedHeight);

    /*****************************************************************************
     * Set cropper's minimum size
     * default: the twice of minimum of the edge lenght of drag square
    *****************************************************************************/
    void setCropperMinimumSize(int minWidth, int minHeight)
        { cropperMinimumWidth = minWidth; cropperMinimumHeight = minHeight; }
    void setCropperMinimumWidth(int minWidth) { cropperMinimumWidth = minWidth; }
    void setCropperMinimumHeight(int minHeight) { cropperMinimumHeight = minHeight; }

    /*************************************************
     * Set the size, color, visibility of rectangular border
    *************************************************/
    void setShowRectBorder(bool show) { isShowRectBorder = show; }
    QPen getBorderPen() { return borderPen; }
    void setBorderPen(const QPen& pen) { borderPen = pen; }

    /*************************************************
     * Set the size, color of drag square
    *************************************************/
    void setShowDragSquare(bool show) { isShowDragSquare = show; }
    void setDragSquareEdge(int edge) { dragSquareEdge = (edge >= 3 ? edge : 3); }
    void setDragSquareColor(const QColor& color) { dragSquareColor = color; }

    /*****************************************
     *  Opacity Effect
    *****************************************/
    void enableOpacity(bool b = true) { isShowOpacityEffect = b; }
    void setOpacity(double newOpacity) { opacity = newOpacity; }

signals:
    void croppedImageChanged();

protected:
    /*****************************************
     * Event
    *****************************************/
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *e) override;
    virtual void mouseMoveEvent(QMouseEvent *e) override;
    virtual void mouseReleaseEvent(QMouseEvent *e) override;

private:
    /***************************************
     * Draw shapes
    ***************************************/
    void drawFillRect(QPoint centralPoint, int edge, QColor color);
    void drawRectOpacity();
    void drawEllipseOpacity();
    void drawOpacity(const QPainterPath& path);     // shadow effect
    void drawSquareEdge(bool onlyFourCorners);

    /***************************************
     * Other utility methods
    ***************************************/
    int getPosInCropperRect(const QPoint& pt);
    bool isPosNearDragSquare(const QPoint& pt1, const QPoint& pt2);
    void resetCropperPos();
    void changeCursor();

    enum {
        RECT_OUTSIZD = 0,
        RECT_INSIDE = 1,
        RECT_TOP_LEFT, RECT_TOP, RECT_TOP_RIGHT, RECT_RIGHT,
        RECT_BOTTOM_RIGHT, RECT_BOTTOM, RECT_BOTTOM_LEFT, RECT_LEFT
    };

    const bool ONLY_FOUR_CORNERS = true;

private:
    QPixmap originalImage;
    QPixmap tempImage;

    bool isShowRectBorder = true;
    QPen borderPen;

    CropperShape cropperShape = CropperShape::UNDEFINED;
    OutputShape  outputShape  = OutputShape::RECT;

    QRect imageRect;     // the whole image area in the label (not real size)
    QRect cropperRect;     // a rectangle frame to choose image area (not real size)
    QRect cropperRect_;     // cropper rect (real size)
    double scaledRate = 1.0;

    bool isLButtonPressed = false;
    bool isCursorPosCalculated = false;
    int  cursorPosInCropperRect = RECT_OUTSIZD;
    QPoint lastPos;
    QPoint currPos;

    bool isShowDragSquare = true;
    int dragSquareEdge = 8;
    QColor dragSquareColor = Qt::white;

    int cropperMinimumWidth = dragSquareEdge * 2;
    int cropperMinimumHeight = dragSquareEdge * 2;

    bool isShowOpacityEffect = false;
    double opacity = 0.6;
};

#endif // IMAGECROPPERLABEL_H

```

具体实现

``` cpp
#include "imagecropperlabel.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QDebug>
#include <QBitmap>

ImageCropperLabel::ImageCropperLabel(int width, int height, QWidget* parent) :
         QLabel(parent)
{
    this->setFixedSize(width, height);
    this->setAlignment(Qt::AlignCenter);
    this->setMouseTracking(true);

    borderPen.setWidth(1);
    borderPen.setColor(Qt::white);
    borderPen.setDashPattern(QVector<qreal>() << 3 << 3 << 3 << 3);
}

void ImageCropperLabel::setOriginalImage(const QPixmap &pixmap) {
    originalImage = pixmap;

    int imgWidth = pixmap.width();
    int imgHeight = pixmap.height();
    int labelWidth = this->width();
    int labelHeight = this->height();
    int imgWidthInLabel;
    int imgHeightInLabel;

    if (imgWidth * labelHeight < imgHeight * labelWidth) {
        scaledRate = labelHeight / double(imgHeight);
        imgHeightInLabel = labelHeight;
        imgWidthInLabel = int(scaledRate * imgWidth);
        imageRect.setRect((labelWidth - imgWidthInLabel) / 2, 0,
                          imgWidthInLabel, imgHeightInLabel);
    }
    else {
        scaledRate = labelWidth / double(imgWidth);
        imgWidthInLabel = labelWidth;
        imgHeightInLabel = int(scaledRate * imgHeight);
        imageRect.setRect(0, (labelHeight - imgHeightInLabel) / 2,
                          imgWidthInLabel, imgHeightInLabel);
    }

    tempImage = originalImage.scaled(imgWidthInLabel, imgHeightInLabel,
                                     Qt::KeepAspectRatio, Qt::SmoothTransformation);
    this->setPixmap(tempImage);

    if (cropperShape >= CropperShape::FIXED_RECT) {
        cropperRect.setWidth(int(cropperRect_.width() * scaledRate));
        cropperRect.setHeight(int(cropperRect_.height() * scaledRate));
    }
    resetCropperPos();
}


/*****************************************
 * set cropper's shape (and size)
*****************************************/
void ImageCropperLabel::setRectCropper() {
    cropperShape = CropperShape::RECT;
    resetCropperPos();
}

void ImageCropperLabel::setSquareCropper() {
    cropperShape = CropperShape::SQUARE;
    resetCropperPos();
}

void ImageCropperLabel::setEllipseCropper() {
    cropperShape = CropperShape::ELLIPSE;
    resetCropperPos();
}

void ImageCropperLabel::setCircleCropper() {
    cropperShape = CropperShape::CIRCLE;
    resetCropperPos();
}

void ImageCropperLabel::setFixedRectCropper(QSize size) {
    cropperShape = CropperShape::FIXED_RECT;
    cropperRect_.setSize(size);
    resetCropperPos();
}

void ImageCropperLabel::setFixedEllipseCropper(QSize size) {
    cropperShape = CropperShape::FIXED_ELLIPSE;
    cropperRect_.setSize(size);
    resetCropperPos();
}

// not recommended
void ImageCropperLabel::setCropper(CropperShape shape, QSize size) {
    cropperShape = shape;
    cropperRect_.setSize(size);
    resetCropperPos();
}

/*****************************************************************************
     * Set cropper's fixed size
    *****************************************************************************/
void ImageCropperLabel::setCropperFixedSize(int fixedWidth, int fixedHeight) {
    cropperRect_.setSize(QSize(fixedWidth, fixedHeight));
    resetCropperPos();
}

void ImageCropperLabel::setCropperFixedWidth(int fixedWidth) {
    cropperRect_.setWidth(fixedWidth);
    resetCropperPos();
}

void ImageCropperLabel::setCropperFixedHeight(int fixedHeight) {
    cropperRect_.setHeight(fixedHeight);
    resetCropperPos();
}

/**********************************************
 * Move cropper to the center of the image
 * And resize to default
**********************************************/
void ImageCropperLabel::resetCropperPos() {
    int labelWidth = this->width();
    int labelHeight = this->height();

    if (cropperShape == CropperShape::FIXED_RECT || cropperShape == CropperShape::FIXED_ELLIPSE) {
        cropperRect.setWidth(int(cropperRect_.width() * scaledRate));
        cropperRect.setHeight(int(cropperRect_.height() * scaledRate));
    }

    switch (cropperShape) {
        case CropperShape::UNDEFINED:
            break;
        case CropperShape::FIXED_RECT:
        case CropperShape::FIXED_ELLIPSE: {
            cropperRect.setRect((labelWidth - cropperRect.width()) / 2,
                             (labelHeight - cropperRect.height()) / 2,
                             cropperRect.width(), cropperRect.height());
            break;
        }
        case CropperShape::RECT:
        case CropperShape::SQUARE:
        case CropperShape::ELLIPSE:
        case CropperShape::CIRCLE: {
            int imgWidth = tempImage.width();
            int imgHeight = tempImage.height();
            int edge = int((imgWidth > imgHeight ? imgHeight : imgWidth) * 3 / 4.0);
            cropperRect.setRect((labelWidth - edge) / 2, (labelHeight - edge) / 2, edge, edge);
            break;
        }
    }
}

QPixmap ImageCropperLabel::getCroppedImage() {
    return getCroppedImage(this->outputShape);
}

QPixmap ImageCropperLabel::getCroppedImage(OutputShape shape) {
    int startX = int((cropperRect.left() - imageRect.left()) / scaledRate);
    int startY = int((cropperRect.top() - imageRect.top()) / scaledRate);
    int croppedWidth = int(cropperRect.width() / scaledRate);
    int croppedHeight = int(cropperRect.height() / scaledRate);

    QPixmap resultImage(croppedWidth, croppedHeight);
    resultImage = originalImage.copy(startX, startY, croppedWidth, croppedHeight);

    // Set ellipse mask (cut to ellipse shape)
    if (shape == OutputShape::ELLIPSE) {
        QSize size(croppedWidth, croppedHeight);
        QBitmap mask(size);
        QPainter painter(&mask);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.fillRect(0, 0, size.width(), size.height(), Qt::white);
        painter.setBrush(QColor(0, 0, 0));
        painter.drawRoundRect(0, 0, size.width(), size.height(), 99, 99);
        resultImage.setMask(mask);
    }

    return resultImage;
}


void ImageCropperLabel::paintEvent(QPaintEvent *event) {
    // Draw original image
    QLabel::paintEvent(event);

    // Draw cropper and set some effects
    switch (cropperShape) {
        case CropperShape::UNDEFINED:
            break;
        case CropperShape::FIXED_RECT:
            drawRectOpacity();
            break;
        case CropperShape::FIXED_ELLIPSE:
            drawEllipseOpacity();
            break;
        case CropperShape::RECT:
            drawRectOpacity();
            drawSquareEdge(!ONLY_FOUR_CORNERS);
            break;
        case CropperShape::SQUARE:
            drawRectOpacity();
            drawSquareEdge(ONLY_FOUR_CORNERS);
            break;
        case CropperShape::ELLIPSE:
            drawEllipseOpacity();
            drawSquareEdge(!ONLY_FOUR_CORNERS);
            break;
        case CropperShape::CIRCLE:
            drawEllipseOpacity();
            drawSquareEdge(ONLY_FOUR_CORNERS);
            break;
    }

    // Draw cropper rect
    if (isShowRectBorder) {
        QPainter painter(this);
        painter.setPen(borderPen);
        painter.drawRect(cropperRect);
    }
}

void ImageCropperLabel::drawSquareEdge(bool onlyFourCorners) {
    if (!isShowDragSquare)
        return;

    // Four corners
    drawFillRect(cropperRect.topLeft(), dragSquareEdge, dragSquareColor);
    drawFillRect(cropperRect.topRight(), dragSquareEdge, dragSquareColor);
    drawFillRect(cropperRect.bottomLeft(), dragSquareEdge, dragSquareColor);
    drawFillRect(cropperRect.bottomRight(), dragSquareEdge, dragSquareColor);

    // Four edges
    if (!onlyFourCorners) {
        int centralX = cropperRect.left() + cropperRect.width() / 2;
        int centralY = cropperRect.top() + cropperRect.height() / 2;
        drawFillRect(QPoint(cropperRect.left(), centralY), dragSquareEdge, dragSquareColor);
        drawFillRect(QPoint(centralX, cropperRect.top()), dragSquareEdge, dragSquareColor);
        drawFillRect(QPoint(cropperRect.right(), centralY), dragSquareEdge, dragSquareColor);
        drawFillRect(QPoint(centralX, cropperRect.bottom()), dragSquareEdge, dragSquareColor);
    }
}

void ImageCropperLabel::drawFillRect(QPoint centralPoint, int edge, QColor color) {
    QRect rect(centralPoint.x() - edge / 2, centralPoint.y() - edge / 2, edge, edge);
    QPainter painter(this);
    painter.fillRect(rect, color);
}

// Opacity effect
void ImageCropperLabel::drawOpacity(const QPainterPath& path) {
    QPainter painterOpac(this);
    painterOpac.setOpacity(opacity);
    painterOpac.fillPath(path, QBrush(Qt::black));
}

void ImageCropperLabel::drawRectOpacity() {
    if (isShowOpacityEffect) {
        QPainterPath p1, p2, p;
        p1.addRect(imageRect);
        p2.addRect(cropperRect);
        p = p1.subtracted(p2);
        drawOpacity(p);
    }
}

void ImageCropperLabel::drawEllipseOpacity() {
    if (isShowOpacityEffect) {
        QPainterPath p1, p2, p;
        p1.addRect(imageRect);
        p2.addEllipse(cropperRect);
        p = p1.subtracted(p2);
        drawOpacity(p);
    }
}

bool ImageCropperLabel::isPosNearDragSquare(const QPoint& pt1, const QPoint& pt2) {
    return abs(pt1.x() - pt2.x()) * 2 <= dragSquareEdge
           && abs(pt1.y() - pt2.y()) * 2 <= dragSquareEdge;
}

int ImageCropperLabel::getPosInCropperRect(const QPoint &pt) {
    if (isPosNearDragSquare(pt, QPoint(cropperRect.right(), cropperRect.center().y())))
        return RECT_RIGHT;
    if (isPosNearDragSquare(pt, cropperRect.bottomRight()))
        return RECT_BOTTOM_RIGHT;
    if (isPosNearDragSquare(pt, QPoint(cropperRect.center().x(), cropperRect.bottom())))
        return RECT_BOTTOM;
    if (isPosNearDragSquare(pt, cropperRect.bottomLeft()))
        return RECT_BOTTOM_LEFT;
    if (isPosNearDragSquare(pt, QPoint(cropperRect.left(), cropperRect.center().y())))
        return RECT_LEFT;
    if (isPosNearDragSquare(pt, cropperRect.topLeft()))
        return RECT_TOP_LEFT;
    if (isPosNearDragSquare(pt, QPoint(cropperRect.center().x(), cropperRect.top())))
        return RECT_TOP;
    if (isPosNearDragSquare(pt, cropperRect.topRight()))
        return RECT_TOP_RIGHT;
    if (cropperRect.contains(pt, true))
        return RECT_INSIDE;
    return RECT_OUTSIZD;
}

/*************************************************
 *
 *  Change mouse cursor type
 *      Arrow, SizeHor, SizeVer, etc...
 *
*************************************************/

void ImageCropperLabel::changeCursor() {
    switch (cursorPosInCropperRect) {
        case RECT_OUTSIZD:
            setCursor(Qt::ArrowCursor);
            break;
        case RECT_BOTTOM_RIGHT: {
            switch (cropperShape) {
            case CropperShape::SQUARE:
            case CropperShape::CIRCLE:
            case CropperShape::RECT:
            case CropperShape::ELLIPSE:
                setCursor(Qt::SizeFDiagCursor);
                break;
            default:
                break;
            }
            break;
        }
        case RECT_RIGHT: {
            switch (cropperShape) {
            case CropperShape::RECT:
            case CropperShape::ELLIPSE:
                setCursor(Qt::SizeHorCursor);
                break;
            default:
                break;
            }
            break;
        }
        case RECT_BOTTOM: {
            switch (cropperShape) {
            case CropperShape::RECT:
            case CropperShape::ELLIPSE:
                setCursor(Qt::SizeVerCursor);
                break;
            default:
                break;
            }
            break;
        }
        case RECT_BOTTOM_LEFT: {
            switch (cropperShape) {
            case CropperShape::RECT:
            case CropperShape::ELLIPSE:
            case CropperShape::SQUARE:
            case CropperShape::CIRCLE:
                setCursor(Qt::SizeBDiagCursor);
                break;
            default:
                break;
            }
            break;
        }
        case RECT_LEFT: {
            switch (cropperShape) {
            case CropperShape::RECT:
            case CropperShape::ELLIPSE:
                setCursor(Qt::SizeHorCursor);
                break;
            default:
                break;
            }
            break;
        }
        case RECT_TOP_LEFT: {
            switch (cropperShape) {
            case CropperShape::RECT:
            case CropperShape::ELLIPSE:
            case CropperShape::SQUARE:
            case CropperShape::CIRCLE:
                setCursor(Qt::SizeFDiagCursor);
                break;
            default:
                break;
            }
            break;
        }
        case RECT_TOP: {
            switch (cropperShape) {
            case CropperShape::RECT:
            case CropperShape::ELLIPSE:
                setCursor(Qt::SizeVerCursor);
                break;
            default:
                break;
            }
            break;
        }
        case RECT_TOP_RIGHT: {
            switch (cropperShape) {
            case CropperShape::SQUARE:
            case CropperShape::CIRCLE:
            case CropperShape::RECT:
            case CropperShape::ELLIPSE:
                setCursor(Qt::SizeBDiagCursor);
                break;
            default:
                break;
            }
            break;
        }
        case RECT_INSIDE: {
            setCursor(Qt::SizeAllCursor);
            break;
        }
    }
}

/*****************************************************
 *
 *  Mouse Events
 *
*****************************************************/

void ImageCropperLabel::mousePressEvent(QMouseEvent *e) {
    currPos = lastPos = e->pos();
    isLButtonPressed = true;
}

void ImageCropperLabel::mouseMoveEvent(QMouseEvent *e) {
    currPos = e->pos();
    if (!isCursorPosCalculated) {
        cursorPosInCropperRect = getPosInCropperRect(currPos);
        changeCursor();
    }

    if (!isLButtonPressed)
        return;
    if (!imageRect.contains(currPos))
        return;

    isCursorPosCalculated = true;

    int xOffset = currPos.x() - lastPos.x();
    int yOffset = currPos.y() - lastPos.y();
    lastPos = currPos;

    int disX = 0;
    int disY = 0;

    // Move cropper
    switch (cursorPosInCropperRect) {
        case RECT_OUTSIZD:
            break;
        case RECT_BOTTOM_RIGHT: {
            disX = currPos.x() - cropperRect.left();
            disY = currPos.y() - cropperRect.top();
            switch (cropperShape) {
                case CropperShape::UNDEFINED:
                case CropperShape::FIXED_RECT:
                case CropperShape::FIXED_ELLIPSE:
                    break;
                case CropperShape::SQUARE:
                case CropperShape::CIRCLE:
                    setCursor(Qt::SizeFDiagCursor);
                    if (disX >= cropperMinimumWidth && disY >= cropperMinimumHeight) {
                        if (disX > disY && cropperRect.top() + disX <= imageRect.bottom()) {
                            cropperRect.setRight(currPos.x());
                            cropperRect.setBottom(cropperRect.top() + disX);
                            emit croppedImageChanged();
                        }
                        else if (disX <= disY && cropperRect.left() + disY <= imageRect.right()) {
                            cropperRect.setBottom(currPos.y());
                            cropperRect.setRight(cropperRect.left() + disY);
                            emit croppedImageChanged();
                        }
                    }
                    break;
                case CropperShape::RECT:
                case CropperShape::ELLIPSE:
                    setCursor(Qt::SizeFDiagCursor);
                    if (disX >= cropperMinimumWidth) {
                        cropperRect.setRight(currPos.x());
                        emit croppedImageChanged();
                    }
                    if (disY >= cropperMinimumHeight) {
                        cropperRect.setBottom(currPos.y());
                        emit croppedImageChanged();
                    }
                    break;
            }
            break;
        }
        case RECT_RIGHT: {
            disX = currPos.x() - cropperRect.left();
            switch (cropperShape) {
                case CropperShape::UNDEFINED:
                case CropperShape::FIXED_RECT:
                case CropperShape::FIXED_ELLIPSE:
                case CropperShape::SQUARE:
                case CropperShape::CIRCLE:
                    break;
                case CropperShape::RECT:
                case CropperShape::ELLIPSE:
                    if (disX >= cropperMinimumWidth) {
                        cropperRect.setRight(currPos.x());
                        emit croppedImageChanged();
                    }
                    break;
            }
            break;
        }
        case RECT_BOTTOM: {
            disY = currPos.y() - cropperRect.top();
            switch (cropperShape) {
                case CropperShape::UNDEFINED:
                case CropperShape::FIXED_RECT:
                case CropperShape::FIXED_ELLIPSE:
                case CropperShape::SQUARE:
                case CropperShape::CIRCLE:
                    break;
                case CropperShape::RECT:
                case CropperShape::ELLIPSE:
                    if (disY >= cropperMinimumHeight) {
                        cropperRect.setBottom(cropperRect.bottom() + yOffset);
                        emit croppedImageChanged();
                    }
                    break;
            }
            break;
        }
        case RECT_BOTTOM_LEFT: {
            disX = cropperRect.right() - currPos.x();
            disY = currPos.y() - cropperRect.top();
            switch (cropperShape) {
                case CropperShape::UNDEFINED:
                    break;
                case CropperShape::FIXED_RECT:
                case CropperShape::FIXED_ELLIPSE:
                case CropperShape::RECT:
                case CropperShape::ELLIPSE:
                    if (disX >= cropperMinimumWidth) {
                        cropperRect.setLeft(currPos.x());
                        emit croppedImageChanged();
                    }
                    if (disY >= cropperMinimumHeight) {
                        cropperRect.setBottom(currPos.y());
                        emit croppedImageChanged();
                    }
                    break;
                case CropperShape::SQUARE:
                case CropperShape::CIRCLE:
                    if (disX >= cropperMinimumWidth && disY >= cropperMinimumHeight) {
                        if (disX > disY && cropperRect.top() + disX <= imageRect.bottom()) {
                            cropperRect.setLeft(currPos.x());
                            cropperRect.setBottom(cropperRect.top() + disX);
                            emit croppedImageChanged();
                        }
                        else if (disX <= disY && cropperRect.right() - disY >= imageRect.left()) {
                            cropperRect.setBottom(currPos.y());
                            cropperRect.setLeft(cropperRect.right() - disY);
                            emit croppedImageChanged();
                        }
                    }
                    break;
            }
            break;
        }
        case RECT_LEFT: {
            disX = cropperRect.right() - currPos.x();
            switch (cropperShape) {
                case CropperShape::UNDEFINED:
                case CropperShape::FIXED_RECT:
                case CropperShape::FIXED_ELLIPSE:
                case CropperShape::SQUARE:
                case CropperShape::CIRCLE:
                    break;
                case CropperShape::RECT:
                case CropperShape::ELLIPSE:
                    if (disX >= cropperMinimumHeight) {
                        cropperRect.setLeft(cropperRect.left() + xOffset);
                        emit croppedImageChanged();
                    }
                    break;
            }
            break;
        }
        case RECT_TOP_LEFT: {
            disX = cropperRect.right() - currPos.x();
            disY = cropperRect.bottom() - currPos.y();
            switch (cropperShape) {
                case CropperShape::UNDEFINED:
                case CropperShape::FIXED_RECT:
                case CropperShape::FIXED_ELLIPSE:
                    break;
                case CropperShape::RECT:
                case CropperShape::ELLIPSE:
                    if (disX >= cropperMinimumWidth) {
                        cropperRect.setLeft(currPos.x());
                        emit croppedImageChanged();
                    }
                    if (disY >= cropperMinimumHeight) {
                        cropperRect.setTop(currPos.y());
                        emit croppedImageChanged();
                    }
                    break;
                case CropperShape::SQUARE:
                case CropperShape::CIRCLE:
                    if (disX >= cropperMinimumWidth && disY >= cropperMinimumHeight) {
                        if (disX > disY && cropperRect.bottom() - disX >= imageRect.top()) {
                            cropperRect.setLeft(currPos.x());
                            cropperRect.setTop(cropperRect.bottom() - disX);
                            emit croppedImageChanged();
                        }
                        else if (disX <= disY && cropperRect.right() - disY >= imageRect.left()) {
                            cropperRect.setTop(currPos.y());
                            cropperRect.setLeft(cropperRect.right() - disY);
                            emit croppedImageChanged();
                        }
                    }
                    break;
            }
            break;
        }
        case RECT_TOP: {
            disY = cropperRect.bottom() - currPos.y();
            switch (cropperShape) {
                case CropperShape::UNDEFINED:
                case CropperShape::FIXED_RECT:
                case CropperShape::FIXED_ELLIPSE:
                case CropperShape::SQUARE:
                case CropperShape::CIRCLE:
                    break;
                case CropperShape::RECT:
                case CropperShape::ELLIPSE:
                    if (disY >= cropperMinimumHeight) {
                        cropperRect.setTop(cropperRect.top() + yOffset);
                        emit croppedImageChanged();
                    }
                    break;
            }
            break;
        }
        case RECT_TOP_RIGHT: {
            disX = currPos.x() - cropperRect.left();
            disY = cropperRect.bottom() - currPos.y();
            switch (cropperShape) {
                case CropperShape::UNDEFINED:
                case CropperShape::FIXED_RECT:
                case CropperShape::FIXED_ELLIPSE:
                    break;
                case CropperShape::RECT:
                case CropperShape::ELLIPSE:
                    if (disX >= cropperMinimumWidth) {
                        cropperRect.setRight(currPos.x());
                        emit croppedImageChanged();
                    }
                    if (disY >= cropperMinimumHeight) {
                        cropperRect.setTop(currPos.y());
                        emit croppedImageChanged();
                    }
                    break;
                case CropperShape::SQUARE:
                case CropperShape::CIRCLE:
                    if (disX >= cropperMinimumWidth && disY >= cropperMinimumHeight) {
                        if (disX < disY && cropperRect.left() + disY <= imageRect.right()) {
                            cropperRect.setTop(currPos.y());
                            cropperRect.setRight(cropperRect.left() + disY);
                            emit croppedImageChanged();
                        }
                        else if (disX >= disY && cropperRect.bottom() - disX >= imageRect.top()) {
                            cropperRect.setRight(currPos.x());
                            cropperRect.setTop(cropperRect.bottom() - disX);
                            emit croppedImageChanged();
                        }
                    }
                    break;
            }
            break;
        }
        case RECT_INSIDE: {
            // Make sure the cropperRect is entirely inside the imageRecct
            if (xOffset > 0) {
                if (cropperRect.right() + xOffset > imageRect.right())
                    xOffset = 0;
            }
            else if (xOffset < 0) {
                if (cropperRect.left() + xOffset < imageRect.left())
                    xOffset = 0;
            }
            if (yOffset > 0) {
                if (cropperRect.bottom() + yOffset > imageRect.bottom())
                    yOffset = 0;
            }
            else if (yOffset < 0) {
                if (cropperRect.top() + yOffset < imageRect.top())
                    yOffset = 0;
            }
            cropperRect.moveTo(cropperRect.left() + xOffset, cropperRect.top() + yOffset);
            emit croppedImageChanged();
        }
        break;
    }

    repaint();
}

void ImageCropperLabel::mouseReleaseEvent(QMouseEvent *) {
    isLButtonPressed = false;
    isCursorPosCalculated = false;
    setCursor(Qt::ArrowCursor);
}
```

下面逐步讲解代码实现

### 枚举类型定义

```cpp
enum class CropperShape { … };
enum class OutputShape { … };
enum class SizeType { … };
```

- **CropperShape**：裁剪框的形状（矩形、正方形、椭圆、圆、以及固定尺寸的变种）。
- **OutputShape**：导出时输出的形状，仅矩形或椭圆两种。
- **SizeType**：内部用来控制当图片过大/过小时如何缩放至 Label 尺寸。

这些枚举让 API 更语义化、调用更直观。

### 类声明与成员变量

```cpp
class ImageCropperLabel : public QLabel {
    Q_OBJECT
public:
    ImageCropperLabel(int width, int height, QWidget* parent);
    // … 设置图片、设置裁剪形状、获取结果等方法 …

signals:
    void croppedImageChanged();

protected:
    // 重载绘制与鼠标事件函数

private:
    // 绘制辅助：drawFillRect、drawOpacity、drawRectOpacity 等
    // 工具方法：getPosInCropperRect、resetCropperPos、changeCursor 等

    // 状态变量
    QPixmap originalImage;     // 原始图片
    QPixmap tempImage;         // 缩放至 Label 尺寸后的临时位图

    bool isShowRectBorder = true;  // 是否画裁剪框边框
    QPen borderPen;                // 边框样式

    CropperShape cropperShape = CropperShape::UNDEFINED;
    OutputShape  outputShape  = OutputShape::RECT;

    QRect imageRect;        // 在 Label 中显示图片的区域（可能有留白）
    QRect cropperRect;      // 裁剪框在 Label 坐标系下的位置与大小
    QRect cropperRect_;     // “真实”像素尺寸下的参考矩形（仅固定尺寸时有效）
    double scaledRate = 1.0;

    // 拖拽、缩放交互相关
    bool isLButtonPressed = false;
    bool isCursorPosCalculated = false;
    int  cursorPosInCropperRect = 0;  // 用上述匿名 enum 表示鼠标在裁剪框哪个位置
    QPoint lastPos, currPos;

    // 拖拽控制点样式
    bool isShowDragSquare = true;
    int dragSquareEdge = 8;
    QColor dragSquareColor = Qt::white;

    int cropperMinimumWidth = dragSquareEdge * 2;
    int cropperMinimumHeight = dragSquareEdge * 2;

    // 半透明遮罩
    bool isShowOpacityEffect = false;
    double opacity = 0.6;
};
```

- **核心状态**：存了原图、临时图、裁剪框位置、缩放比例等。
- **交互状态**：鼠标按下／移动、在哪个拖拽点、是否在拖拽中。
- **可配置属性**：边框、拖拽手柄、最小尺寸、遮罩效果等，通过 public 方法暴露给外部。



### 构造函数（Label 初始化）

```cpp
ImageCropperLabel::ImageCropperLabel(int width, int height, QWidget* parent)
    : QLabel(parent)
{
    setFixedSize(width, height);
    setAlignment(Qt::AlignCenter);
    setMouseTracking(true);   // 即使不按按钮也能收到 mouseMove 事件

    borderPen.setWidth(1);
    borderPen.setColor(Qt::white);
    borderPen.setDashPattern(QVector<qreal>() << 3 << 3);  // 虚线
}
```

- **固定尺寸**：确保裁剪界面大小一致，不随容器拉伸。
- **居中显示**：图片展示时居中。
- **边框样式**：白色虚线。

------

### 加载并缩放原图

```cpp
void ImageCropperLabel::setOriginalImage(const QPixmap &pixmap) {
    originalImage = pixmap;

    // 计算在 label 里显示时的缩放比例和目标尺寸
    if (imgWidth * labelHeight < imgHeight * labelWidth) {
        scaledRate = labelHeight / double(imgHeight);
        … compute imgWidthInLabel, imageRect …
    } else {
        … 另一种缩放方式 …
    }

    tempImage = originalImage.scaled(imgWidthInLabel, imgHeightInLabel,
                                     Qt::KeepAspectRatio, Qt::SmoothTransformation);
    setPixmap(tempImage);

    // 如果是固定尺寸裁剪框，需要按同样比例缩放
    if (cropperShape >= CropperShape::FIXED_RECT) {
        cropperRect.setWidth(int(cropperRect_.width() * scaledRate));
        …
    }
    resetCropperPos();
}
```

- **按保持长宽比的方式**，把原图缩放到 Label 区域内（letterbox 模式）。
- **`imageRect`**：记录图像在 Label 坐标系下的实际绘制区域。
- **`tempImage`**：在 Label 上展示的图，用于用户交互。

![image-20250511114718983](https://cdn.llfc.club/img/image-20250511114718983.png)

------

![image-20250511115038528](https://cdn.llfc.club/img/image-20250511115038528.png)

### 裁剪形状设置与重置

```cpp
void ImageCropperLabel::setRectCropper()     { cropperShape = RECT; resetCropperPos(); }
… // 各种 setXXXCropper()

void ImageCropperLabel::resetCropperPos() {
    // 根据 cropperShape，计算初始的 cropperRect：
    // - 固定尺寸时居中铺满
    // - 可变尺寸时取图片较短边的 3/4，居中
}
```

- **统一调用**：每次改变 shape 或大小，都调用 `resetCropperPos()` 让裁剪框回到可见区域中央。

![image-20250511115825046](https://cdn.llfc.club/img/image-20250511115825046.png)

------



![image-20250511120205707](https://cdn.llfc.club/img/image-20250511120205707.png)

### 获取裁剪结果

```cpp
QPixmap ImageCropperLabel::getCroppedImage(OutputShape shape) {
    // 1. 根据缩放比例，把 cropperRect 从 Label 坐标系映射到原图坐标系：
    int startX = (cropperRect.left() - imageRect.left()) / scaledRate;
    … compute croppedWidth, croppedHeight …

    // 2. 从 originalImage 上 copy 出子图
    QPixmap resultImage = originalImage.copy(startX, startY, cw, ch);

    // 3. 如果输出椭圆，则用 QBitmap+setMask 做裁切
    if (shape == OutputShape::ELLIPSE) {
        QBitmap mask(size);
        QPainter p(&mask);
        p.fillRect(…, Qt::white);
        p.setBrush(Qt::black);
        p.drawRoundRect(0,0,w,h,99,99);
        resultImage.setMask(mask);
    }
    return resultImage;
}
```

![image-20250511120838187](https://cdn.llfc.club/img/image-20250511120838187.png)

- **核心思路**：先把用户框映射回原图，再按需求做矩形或椭圆裁剪。

**为什么要除以 `scaledRate`？**

1. **背景**：裁剪区域的坐标 (`cropperRect`) 和尺寸 (`cropperRect.width()`, `cropperRect.height()`) 都是相对于图像在显示中的位置和大小，而不是原始图像的大小。这意味着显示上的裁剪框可能已经被缩放过。因此，`scaledRate` 是一个缩放比例，用来将裁剪区域从显示坐标系统（可能已经缩放）转换回原始图像的坐标系统。

2. **代码解释**：

   - `cropperRect.left() - imageRect.left()` 表示裁剪框左边缘与原始图像左边缘的偏移量（即裁剪框相对于图像的起始位置）。
   - `scaledRate` 是图像在显示时的缩放比例（例如，显示的图像比原图小或大，`scaledRate` 可以是 1、0.5、2 等）。
   - 除以 `scaledRate` 就是将显示的坐标转换为原始图像的坐标。这样得到的是裁剪框在原始图像中的位置和大小。

   **例如**：假设 `scaledRate = 0.5`（显示图像是原图的 50%），则 `cropperRect` 表示的区域实际在原图中要乘以 2 才能得到正确的大小和位置。



**为什么椭圆要单独处理？**

裁剪区域的形状是矩形的，而图像本身可能要根据需求切割成不同的形状。如果要求裁剪区域是椭圆形状，那么矩形的裁剪区域必须通过遮罩（mask）来实现。

1. **遮罩的作用**：
   - 默认情况下，裁剪区域是矩形的。为了让裁剪后的图像呈现椭圆形状，我们需要用一个遮罩来过滤掉矩形区域之外的部分。
   - 通过绘制一个椭圆（在矩形区域内），并设置遮罩（`mask`），使得图像在该遮罩的范围内显示，超出范围的部分将变为透明。
2. **椭圆处理的步骤**：
   - 通过 `QBitmap mask(size)` 创建一个与裁剪区域大小相同的二值遮罩（黑白图像）。
   - 然后使用 `QPainter` 绘制一个椭圆形状。 `drawRoundRect` 方法画的其实是一个圆角矩形，但由于宽度和高度一样，且角的弯曲度非常高（`99, 99`），所以它的效果看起来是一个椭圆。
   - 最后，通过 `resultImage.setMask(mask)` 将这个椭圆形状应用到裁剪后的图像上，从而实现椭圆形的裁剪效果。

`painter.setBrush(QColor(0, 0, 0));` 在这里的唯一目的是往那个 **`QBitmap` 遮罩（mask）** 上「画」一个黑色的圆角矩形，用来告诉 Qt 哪一块区域要保留、哪一块区域要透明——它并不是在往你的 **`resultImage`** 上画黑色。

- **在 `mask` 上**：
  - 黑色 → 可见
  - 白色 → 透明

如果你不 `setBrush(QColor(0, 0, 0))` 去把圆角矩形「涂黑」，那么整张 `mask` 就只有白色（或只有透明），结果就是 **整张图片都被裁成透明了**，你看不见任何内容。

所以，`setBrush(QColor(0, 0, 0))` 的作用只是：

1. 在 `mask` 上，填充一个黑色的圆角矩形；
2. 当你调用 `resultImage.setMask(mask);` 时，Qt 会把这部分“黑色”区域映射为 **保留原图像素**，而把剩下的（白色）区域变成透明。

![image-20250511121434047](https://cdn.llfc.club/img/image-20250511121434047.png)

------

### 绘制与遮罩效果

```cpp
void ImageCropperLabel::paintEvent(QPaintEvent *event) {
    // 1. 先调用父类，实现原始图像的绘制
    QLabel::paintEvent(event);

    // 2. 根据当前裁剪形状，绘制不同的“半透明遮罩”或“高光边”
    switch (cropperShape) {
        case CropperShape::UNDEFINED:
            break;
        case CropperShape::FIXED_RECT:
            drawRectOpacity();
            break;
        case CropperShape::FIXED_ELLIPSE:
            drawEllipseOpacity();
            break;
        case CropperShape::RECT:
            drawRectOpacity();
            drawSquareEdge(!ONLY_FOUR_CORNERS);
            break;
        case CropperShape::SQUARE:
            drawRectOpacity();
            drawSquareEdge(ONLY_FOUR_CORNERS);
            break;
        case CropperShape::ELLIPSE:
            drawEllipseOpacity();
            drawSquareEdge(!ONLY_FOUR_CORNERS);
            break;
        case CropperShape::CIRCLE:
            drawEllipseOpacity();
            drawSquareEdge(ONLY_FOUR_CORNERS);
            break;
    }

    // 3. 如果需要，给裁剪框本身画一条边框
    if (isShowRectBorder) {
        QPainter painter(this);
        painter.setPen(borderPen);
        painter.drawRect(cropperRect);
    }
}

```

- **绘制原图**
   `QLabel::paintEvent(event)` 会根据当前设置的 pixmap 或者绘图内容，把“完整的”图像画到控件上。我们不做任何改动，保留原始像素。

  **叠加遮罩或高光边**
   根据 `cropperShape`（枚举当前选中的裁剪形状），有两类主要操作：

  - **`drawRectOpacity()` / `drawEllipseOpacity()`**：在裁剪框以外的区域绘制半透明黑色遮罩，突出裁剪区域本身。
  - **`drawSquareEdge(...)`**：在裁剪框的四条边或者四个角上绘制高对比度的“小方块”或“手柄”，以便用户拖动调整大小。

  **绘制裁剪框边线**
   如果 `isShowRectBorder==true`，再用 `borderPen`（一般是明亮的颜色或宽度可见的线条）精确地把 `cropperRect` 描边一次，让裁剪范围更清晰。



**半透明遮罩** 

```cpp
void ImageCropperLabel::drawOpacity(const QPainterPath& path) {
    QPainter painterOpac(this);
    painterOpac.setOpacity(opacity);            // 设定当前 painter 的透明度
    painterOpac.fillPath(path, QBrush(Qt::black)); // 用黑色填充整个 path 区域
}
```

- **`opacity`**：这是一个 `[0.0 … 1.0]` 之间的浮点值，控制遮罩的“浓度”。越接近 1.0，黑得越不透明；越接近 0.0，则越接近“无色”。
- **`fillPath(path, QBrush(Qt::black))`**：把传入的 `QPainterPath` 区域，用半透明的黑色一次性“盖”上去。



**`drawRectOpacity()`**

```cpp
void ImageCropperLabel::drawRectOpacity() {
    if (!isShowOpacityEffect) return;

    // 1. p1：整个图像区域
    QPainterPath p1;
    p1.addRect(imageRect);

    // 2. p2：裁剪框区域
    QPainterPath p2;
    p2.addRect(cropperRect);

    // 3. 求差集：p = p1 - p2
    QPainterPath p = p1.subtracted(p2);

    // 4. 对 p 区域绘制半透明黑色遮罩
    drawOpacity(p);
}
```

- **`imageRect`**：通常是整个图片在控件上的显示区域。
- **`cropperRect`**：用户定义的“裁剪框”矩形。
- **`p1.subtracted(p2)`**：把裁剪框内部切掉，结果 `p` 就是“图片区域减去裁剪框”的外部部分。
- **遮罩效果**：只有外部部分被半透明黑色盖住，裁剪框内——也就是用户关心的区域——保持原样未被遮盖。

**椭圆遮罩 —— `drawEllipseOpacity()`（原理同上）**

虽然你没贴出函数体，但它与 `drawRectOpacity()` 唯一区别就是把 `p2.addRect(cropperRect)` 换成：

```cpp
QPainterPath p2;
p2.addEllipse(cropperRect);
```

这样 `p1.subtracted(p2)` 就是“整张图片减去椭圆区域”，半透明遮罩会围着椭圆“环绕”绘制。

------

![image-20250511122719029](https://cdn.llfc.club/img/image-20250511122719029.png)

**“方块手柄”高光 —— `drawSquareEdge(bool onlyCorners)`**

``` cpp
void ImageCropperLabel::drawSquareEdge(bool onlyFourCorners) {
    if (!isShowDragSquare)
        return;

    // Four corners
    drawFillRect(cropperRect.topLeft(), dragSquareEdge, dragSquareColor);
    drawFillRect(cropperRect.topRight(), dragSquareEdge, dragSquareColor);
    drawFillRect(cropperRect.bottomLeft(), dragSquareEdge, dragSquareColor);
    drawFillRect(cropperRect.bottomRight(), dragSquareEdge, dragSquareColor);

    // Four edges
    if (!onlyFourCorners) {
        int centralX = cropperRect.left() + cropperRect.width() / 2;
        int centralY = cropperRect.top() + cropperRect.height() / 2;
        drawFillRect(QPoint(cropperRect.left(), centralY), dragSquareEdge, dragSquareColor);
        drawFillRect(QPoint(centralX, cropperRect.top()), dragSquareEdge, dragSquareColor);
        drawFillRect(QPoint(cropperRect.right(), centralY), dragSquareEdge, dragSquareColor);
        drawFillRect(QPoint(centralX, cropperRect.bottom()), dragSquareEdge, dragSquareColor);
    }
}
```

![image-20250511123344886](https://cdn.llfc.club/img/image-20250511123344886.png)

此函数通常会：

1. 在 `cropperRect` 的四条边（或四个角）各计算几个固定大小的小矩形位置。
2. 用不透明画刷（如白色或蓝色）绘制这些 “拖拽手柄”，让用户知道可以从这些点出发拖动调整大小。

`onlyCorners` 参数决定是只在四个角显示手柄，还是在四条边中央也显示。

### 手柄检测

**`isPosNearDragSquare(pt1, pt2)`：手柄附近检测**

```cpp
bool ImageCropperLabel::isPosNearDragSquare(const QPoint& pt1, const QPoint& pt2) {
    return abs(pt1.x() - pt2.x()) * 2 <= dragSquareEdge
        && abs(pt1.y() - pt2.y()) * 2 <= dragSquareEdge;
}
```

- **`pt1`**：当前鼠标点（或触点）坐标。
- **`pt2`**：某个拖拽手柄中心点坐标。
- **`dragSquareEdge`**：定义手柄大小（宽或高）的常量。

逻辑：如果鼠标点到手柄中心的水平距离和垂直距离都不超过 `dragSquareEdge/2`，就认为“在手柄区域内”。乘以 2 只是把“不超过半边”转成”两倍距离不超过边长“的判断。

------

**`getPosInCropperRect(pt)`：整体位置分类**

```cpp
int ImageCropperLabel::getPosInCropperRect(const QPoint &pt) {
    if (isPosNearDragSquare(pt, QPoint(cropperRect.right(), cropperRect.center().y())))
        return RECT_RIGHT;
    if (isPosNearDragSquare(pt, cropperRect.bottomRight()))
        return RECT_BOTTOM_RIGHT;
    if (isPosNearDragSquare(pt, QPoint(cropperRect.center().x(), cropperRect.bottom())))
        return RECT_BOTTOM;
    if (isPosNearDragSquare(pt, cropperRect.bottomLeft()))
        return RECT_BOTTOM_LEFT;
    if (isPosNearDragSquare(pt, QPoint(cropperRect.left(), cropperRect.center().y())))
        return RECT_LEFT;
    if (isPosNearDragSquare(pt, cropperRect.topLeft()))
        return RECT_TOP_LEFT;
    if (isPosNearDragSquare(pt, QPoint(cropperRect.center().x(), cropperRect.top())))
        return RECT_TOP;
    if (isPosNearDragSquare(pt, cropperRect.topRight()))
        return RECT_TOP_RIGHT;

    if (cropperRect.contains(pt, true))
        return RECT_INSIDE;

    return RECT_OUTSIZD;
}
```

按照顺序，它分别检测：

1. **右边中点 `RECT_RIGHT`**
    以 `(cropperRect.right(), cropperRect.center().y())` 为中心，看鼠标是否落在右侧手柄区域。
2. **右下角 `RECT_BOTTOM_RIGHT`**
    以 `cropperRect.bottomRight()` 为中心，看鼠标是否落在这个角的手柄。
3. **下边中点 `RECT_BOTTOM`**
    中点为 `(center.x(), bottom)`。
4. **左下角 `RECT_BOTTOM_LEFT`**
5. **左边中点 `RECT_LEFT`**
6. **左上角 `RECT_TOP_LEFT`**
7. **上边中点 `RECT_TOP`**
8. **右上角 `RECT_TOP_RIGHT`**

如果以上八个拖拽手柄区域都没有命中，接着：

- **`RECT_INSIDE`**：如果点严格落在 `cropperRect` 内部（第二个参数 `true` 表示内边缘也算），就返回“内部”标志。
- **`RECT_OUTSIZD`**：都不符合，则认为在裁剪框外。



**综合效果**

- 在 **鼠标按下** 或 **移动** 时，调用 `getPosInCropperRect(pt)`，能够快速定位出当前点相对于裁剪框的位置类型。
- 上层逻辑（如鼠标事件处理）根据这个返回值，决定要进行哪种操作：
  - 如果是某个角或边的手柄，就进入“调整大小”模式，且拖拽方向锁定；
  - 如果是 `RECT_INSIDE`，则进入“移动整个裁剪框”模式；
  - 如果是 `RECT_OUTSIZD`，则不做任何裁剪框相关的拖拽操作。

这样，就实现了一个用户友好的「拖拽四角/边来调整裁剪框大小，或者拖拽内部来移动框」的交互体验。

------

### 鼠标按下移动释放

**mousePressEvent**

```cpp
void ImageCropperLabel::mousePressEvent(QMouseEvent *e) {
    currPos = lastPos = e->pos();
    isLButtonPressed = true;
}
```

**功能**：当鼠标左键按下时调用。

**做了什么**：

1. 用 `e->pos()`（相对于控件左上角的坐标）初始化 `currPos`、`lastPos`，为后续移动计算做准备。
2. 将 `isLButtonPressed` 置为 `true`，开启拖动或缩放模式。

**mouseMoveEvent**

这是核心函数，处理移动和缩放。

``` cpp
void ImageCropperLabel::mouseMoveEvent(QMouseEvent *e) {
    currPos = e->pos();
    // 首次进入时，确定鼠标在哪个区域：边角、边缘、框内或框外
    if (!isCursorPosCalculated) {
        cursorPosInCropperRect = getPosInCropperRect(currPos);
        changeCursor();  // 根据区域切换不同形状的鼠标指针
    }

    // 如果左键没有按下或鼠标移出了图片范围，就不做任何处理
    if (!isLButtonPressed || !imageRect.contains(currPos))
        return;

    isCursorPosCalculated = true;  // 保证只计算一次区域
    // 计算本次移动增量
    int xOffset = currPos.x() - lastPos.x();
    int yOffset = currPos.y() - lastPos.y();
    lastPos = currPos;

    int disX = 0, disY = 0;  // 用于后续缩放计算

    // 根据鼠标所在区域，选择对应的移动/缩放逻辑
    switch (cursorPosInCropperRect) {
        case RECT_OUTSIZD:
            break;  // 在框外：不处理

        // —— 右下角 缩放 ——
        case RECT_BOTTOM_RIGHT: {
            disX = currPos.x() - cropperRect.left();
            disY = currPos.y() - cropperRect.top();
            switch (cropperShape) {
                // 固定模式：不允许缩放
                case CropperShape::UNDEFINED:
                case CropperShape::FIXED_RECT:
                case CropperShape::FIXED_ELLIPSE:
                    break;
                // 正方形／圆形：强制保持宽高一致
                case CropperShape::SQUARE:
                case CropperShape::CIRCLE:
                    setCursor(Qt::SizeFDiagCursor);
                    // 保证没有小于最小尺寸且不超出图片下/right 边
                    if (disX >= cropperMinimumWidth && disY >= cropperMinimumHeight) {
                        if (disX > disY && cropperRect.top() + disX <= imageRect.bottom()) {
                            // 宽度主导，伸长底边
                            cropperRect.setRight(currPos.x());
                            cropperRect.setBottom(cropperRect.top() + disX);
                        }
                        else if (disY >= disX && cropperRect.left() + disY <= imageRect.right()) {
                            // 高度主导，伸长右边
                            cropperRect.setBottom(currPos.y());
                            cropperRect.setRight(cropperRect.left() + disY);
                        }
                        emit croppedImageChanged();
                    }
                    break;
                // 普通矩形／椭圆：独立伸缩宽或高
                case CropperShape::RECT:
                case CropperShape::ELLIPSE:
                    setCursor(Qt::SizeFDiagCursor);
                    if (disX >= cropperMinimumWidth) {
                        cropperRect.setRight(currPos.x());
                        emit croppedImageChanged();
                    }
                    if (disY >= cropperMinimumHeight) {
                        cropperRect.setBottom(currPos.y());
                        emit croppedImageChanged();
                    }
                    break;
            }
            break;
        }

        // —— 右侧边 缩放 ——
        case RECT_RIGHT: {
            disX = currPos.x() - cropperRect.left();
            if (cropperShape==CropperShape::RECT||cropperShape==CropperShape::ELLIPSE) {
                if (disX >= cropperMinimumWidth) {
                    cropperRect.setRight(currPos.x());
                    emit croppedImageChanged();
                }
            }
            break;
        }

        // —— 底部边 缩放 ——
        case RECT_BOTTOM: {
            disY = currPos.y() - cropperRect.top();
            if (cropperShape==CropperShape::RECT||cropperShape==CropperShape::ELLIPSE) {
                if (disY >= cropperMinimumHeight) {
                    cropperRect.setBottom(cropperRect.bottom() + yOffset);
                    emit croppedImageChanged();
                }
            }
            break;
        }

        // —— 左下角、左侧、上边…… 各角/边 缩放逻辑同上 —— 
        // （代码中分别处理了 RECT_BOTTOM_LEFT、RECT_LEFT、RECT_TOP_LEFT、
        //  RECT_TOP、RECT_TOP_RIGHT，核心思想与右下相似：计算 disX/disY，
        //  判断形状、最小尺寸、边界，再更新对应边或角的坐标并 emit。）

        // —— 框内拖动 —— 
        case RECT_INSIDE: {
            // 先检测移动后是否会超出图片范围，将偏移量 xOffset/yOffset 裁剪到合法区间
            if (cropperRect.left() + xOffset < imageRect.left())    xOffset = imageRect.left() - cropperRect.left();
            if (cropperRect.right()+ xOffset > imageRect.right())   xOffset = imageRect.right() - cropperRect.right();
            if (cropperRect.top()  + yOffset < imageRect.top())     yOffset = imageRect.top() - cropperRect.top();
            if (cropperRect.bottom()+ yOffset > imageRect.bottom())  yOffset = imageRect.bottom() - cropperRect.bottom();
            // 移动整个裁剪框
            cropperRect.translate(xOffset, yOffset);
            emit croppedImageChanged();
            break;
        }
    }

    repaint();  // 触发重绘，及时在界面上更新新的裁剪框
}

```

**关键点总结**

1. **首次定位**
    当鼠标首次进入 `mouseMoveEvent`，用 `getPosInCropperRect(currPos)` 判断鼠标在裁剪框的哪个“热区”——外部、框内、四边、四角中的哪一个，并调用 `changeCursor()` 切换对应的鼠标指针样式（如移动箭头、水平/垂直/对角调整形状等），以提示用户下一步操作。

2. **左右、上下、四角缩放**

   - 对于矩形/椭圆，宽高可独立调整；
   - 对于正方形/圆，则保证 `width == height`，并根据位移量较大的一边来驱动另一边；
   - 对于“固定”模式，则完全不允许用户改变大小。

3. **边界与最小尺寸约束**

   - 缩放时先判断新的宽度/高度是否 ≥ `cropperMinimumWidth/Height`；
   - 再判断新坐标是否会跑出 `imageRect`（图片区域）之外；
   - 最后才更新 `cropperRect` 并发信号 `croppedImageChanged()` 以便上层 UI 或逻辑更新裁剪后的图像。

4. **拖动整个裁剪框**

   - 鼠标在框内部拖动（`RECT_INSIDE`），计算每次的偏移 `xOffset,yOffset`，
   - 并先“裁剪”偏移量，使整个框保持在图片范围内，
   - 最后调用 `translate()` 平移 `cropperRect`。

   

**`mouseReleaseEvent(QMouseEvent *)`**

```cpp
void ImageCropperLabel::mouseReleaseEvent(QMouseEvent *) {
    isLButtonPressed = false;
    isCursorPosCalculated = false;
    setCursor(Qt::ArrowCursor);
}
```

- **功能**：当鼠标左键松开时调用。
- **做了什么**：
  1. 将 `isLButtonPressed` 置为 `false`，停止后续的拖动/缩放处理。
  2. 重置 `isCursorPosCalculated = false`，下次再移动时会重新计算在哪个区域。
  3. 恢复默认箭头指针。



## 保存逻辑

``` cpp
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
```



1. **选择图片文件（支持多种格式）**

```cpp
QString filename = QFileDialog::getOpenFileName(
    this,
    tr("选择图片"),
    QString(),
    tr("图片文件 (*.png *.jpg *.jpeg *.bmp *.webp)")
);
if (filename.isEmpty())
    return;
```

- **功能**：当用户点击上传头像按钮时，弹出文件选择对话框（`QFileDialog`），允许用户选择图片文件。此对话框支持的文件格式包括 `.png`、`.jpg`、`.jpeg`、`.bmp` 和 `.webp`。如果用户没有选择文件（即点击了取消），则返回并不执行后续操作。

2. **加载图片文件**

```cpp
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
```

- **功能**：通过 `QPixmap` 类加载用户选定的图片文件。如果加载失败（如文件损坏、格式不支持等），则弹出错误对话框提示用户，并退出当前函数。

3. **裁剪图片**

```cpp
QPixmap image = ImageCropperDialog::getCroppedImage(filename, 600, 400, CropperShape::CIRCLE);
if (image.isNull())
    return;
```

- **功能**：调用 `ImageCropperDialog::getCroppedImage` 函数裁剪图片。这个函数会根据传入的文件路径（`filename`）、目标大小（600x400）和裁剪形状（此处是圆形 `CropperShape::CIRCLE`）返回一个裁剪后的图片 `QPixmap`。如果裁剪过程失败（即返回空 `QPixmap`），则函数直接退出。

4. **缩放图片到指定的 `QLabel` 大小**

```cpp
QPixmap scaledPixmap = image.scaled( ui->head_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
ui->head_lb->setPixmap(scaledPixmap);
ui->head_lb->setScaledContents(true);
```

- **功能**：将裁剪后的图片缩放到与界面上显示头像的 `QLabel`（`head_lb`）大小相匹配。使用 `scaled()` 方法，保持图片的宽高比 (`Qt::KeepAspectRatio`)，并且应用平滑的图像转换（`Qt::SmoothTransformation`），保证缩放后的图片质量尽可能高。最后，将缩放后的图片设置到 `QLabel` 上，并开启 `setScaledContents(true)`，使得 `QLabel` 自动调整内容大小以适应其尺寸。

5. **获取应用程序的存储目录**

```cpp
QString storageDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
```

- **功能**：通过 `QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)` 获取应用程序的可写数据存储目录。这个目录是操作系统为应用程序提供的一个常规存储路径，通常用于存储配置文件、数据文件等。

6. **创建头像存储目录**

```cpp
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
```

- **功能**：检查存储目录下是否已经存在一个名为 `avatars` 的子目录。如果不存在，则通过 `mkpath()` 创建该子目录。若创建失败，弹出警告对话框提示用户检查权限或磁盘空间。

7. **拼接最终的保存路径**

```cpp
QString filePath = dir.filePath("avatars/head.png");
```

- **功能**：拼接最终的文件路径，存储头像的文件名为 `head.png`，并位于 `avatars` 目录下。`filePath` 即为头像图片的完整存储路径。

8. **保存裁剪后的图片**

```cpp
if (!scaledPixmap.save(filePath, "PNG")) {
    QMessageBox::warning(
        this,
        tr("保存失败"),
        tr("头像保存失败，请检查权限或磁盘空间。")
    );
} else {
    qDebug() << "头像已保存到：" << filePath;
}
```

- **功能**：使用 `QPixmap::save()` 方法将裁剪并缩放后的图片保存到指定路径 `filePath`。保存格式为 `PNG`。如果保存失败，则弹出警告对话框提示用户；否则，输出日志，显示头像已成功保存的路径。



## 源码连接

https://gitee.com/secondtonone1/llfcchat
