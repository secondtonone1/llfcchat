#include "global.h"
#include <QEventLoop>
#include <QTimer>
#include <QUuid>

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
