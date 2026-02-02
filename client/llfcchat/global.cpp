#include "global.h"
#include <QEventLoop>
#include <QTimer>
#include <QUuid>
#include <QPainter>

std::function<void(QWidget*)> repolish =[](QWidget *w){
    w->style()->unpolish(w);
    w->style()->polish(w);
};

std::function<QString(QString)> xorString = [](QString input){
    QString result = input; // 复制原始字符串，以便进行修改
    int length = input.length(); // 获取字符串的长度
    ushort xor_code = length % 255;
    for (int i = 0; i < length; ++i) {
        // 对每个字符进行异或操作
        // 注意：这里假设字符都是ASCII，因此直接转换为QChar
        result[i] = QChar(static_cast<ushort>(input[i].unicode() ^ xor_code));
    }
    return result;
};

QString gate_url_prefix = "";

void delay_run(int msecs) {
    QEventLoop loop;
    // singleShot 到时后会触发 loop.quit()，从而退出事件循环
    QTimer::singleShot(msecs, &loop, &QEventLoop::quit);
    loop.exec();
}

QString generateUniqueFileName(const QString& originalName){

     QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
     QFileInfo fileInfo(originalName);
     QString extension = fileInfo.suffix();
     return uuid + (extension.isEmpty() ? "" : "." + extension);
}

QString generateUniqueIconName(){
    QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    return uuid + ".png";
}

QString calculateFileHash(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return QString();

    QCryptographicHash hash(QCryptographicHash::Md5);

    // 分块计算哈希，避免大文件占用过多内存
    const qint64 chunkSize = 1024 * 1024; // 1MB
    while (!file.atEnd())
    {
        hash.addData(file.read(chunkSize));
    }
    file.close();

    return hash.result().toHex();
}

QPixmap CreateLoadingPlaceholder(int width, int height ) {
    QPixmap placeholder(width, height);
    placeholder.fill(QColor(240, 240, 240)); // 浅灰色背景

    QPainter painter(&placeholder);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制边框
    painter.setPen(QPen(QColor(200, 200, 200), 2));
    painter.drawRect(1, 1, width - 2, height - 2);

    // 绘制加载图标（简单的旋转圆圈或文字）
    QFont font;
    font.setPointSize(12);
    painter.setFont(font);
    painter.setPen(QColor(150, 150, 150));
    painter.drawText(placeholder.rect(), Qt::AlignCenter, "加载中...");

    // 可选：添加图片图标
    painter.setPen(QColor(180, 180, 180));
    QRect iconRect(width / 2 - 20, height / 2 - 40, 40, 30);
    painter.drawRect(iconRect);
    painter.drawLine(iconRect.topLeft(), iconRect.bottomRight());
    painter.drawLine(iconRect.topRight(), iconRect.bottomLeft());

    return placeholder;
}