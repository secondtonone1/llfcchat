#include "MessageTextEdit.h"
#include <QDebug>
#include <QMessageBox>
#include "global.h"

MessageTextEdit::MessageTextEdit(QWidget *parent)
    : QTextEdit(parent)
{

    //this->setStyleSheet("border: none;");
    this->setMaximumHeight(60);

//    connect(this,SIGNAL(textChanged()),this,SLOT(textEditChanged()));

}

MessageTextEdit::~MessageTextEdit()
{

}

QVector<std::shared_ptr<MsgInfo>> MessageTextEdit::getMsgList()
{
    _total_msg_list.clear();
    QString doc = this->document()->toPlainText();
    QString text="";//存储文本信息
    int indexUrl = 0;
    //这个是存储的富文本信息，包括图片的url以及文件的url
    int count = _img_or_file_list.size();

    for(int index=0; index<doc.size(); index++)
    {
        //如果遇到替换符，说明后面的是图片或者文件的url
        if(doc[index]==QChar::ObjectReplacementCharacter)
        {
            if(!text.isEmpty())
            {
                insertMsgList(_total_msg_list, MsgType::TEXT_MSG, text, QPixmap(),"",0,"");
                text.clear();
            }
            while(indexUrl<count)
            {
                std::shared_ptr<MsgInfo> msg = _img_or_file_list[indexUrl];
                if(this->document()->toHtml().contains(msg->_text_or_url,Qt::CaseSensitive))
                {
                    indexUrl++;
                    _total_msg_list.append(msg);
                    break;
                }
                indexUrl++;
            }
        }
        else
        {
            //追加字符到文本消息中
            text.append(doc[index]);
        }
    }
    if(!text.isEmpty())
    {
        insertMsgList(_total_msg_list, MsgType::TEXT_MSG, text, QPixmap(), "", 0, "");
        text.clear();
    }
    _img_or_file_list.clear();
    this->clear();
    return _total_msg_list;
}

void MessageTextEdit::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->source()==this)
        event->ignore();
    else
        event->accept();
}

void MessageTextEdit::dropEvent(QDropEvent *event)
{
    insertFromMimeData(event->mimeData());
    event->accept();
}

void MessageTextEdit::keyPressEvent(QKeyEvent *e)
{
    if((e->key()==Qt::Key_Enter||e->key()==Qt::Key_Return)&& !(e->modifiers() & Qt::ShiftModifier))
    {
        emit send();
        return;
    }
    QTextEdit::keyPressEvent(e);
}

void MessageTextEdit::insertFileFromUrl(const QStringList &urls)
{
    if(urls.isEmpty())
        return;

    foreach (QString url, urls){
         if(isImage(url))
             insertImages(url);
         else
             insertFiles(url);
    }
}

void MessageTextEdit::insertImages(const QString &url)
{
    //文件信息
    QFileInfo  fileInfo(url);
    if (fileInfo.isDir())
    {
        QMessageBox::information(this, "提示", "只允许拖拽单个文件!");
        return;
    }

    auto total_size = fileInfo.size();
    
    qint64 max_size = qint64(2) * 1024 * 1024 * 1024;

    if (total_size > max_size)
    {
        QMessageBox::information(this, "提示", "发送的文件大小不能大于2G");
        return;
    }

    // 计算文件MD5
    QString fileMd5 = calculateFileHash(url);

    if (fileMd5.isEmpty())
    {
        QMessageBox::warning(this, "错误", "无法计算文件MD5");
        return;
    }


    QImage image(url);
    //按比例缩放图片
    if(image.width()>120||image.height()>80)
    {
        if(image.width()>image.height())
        {
          image =  image.scaledToWidth(120,Qt::SmoothTransformation);
        }
        else
            image = image.scaledToHeight(80,Qt::SmoothTransformation);
    }
    QTextCursor cursor = this->textCursor();
    // QTextDocument *document = this->document();
    // document->addResource(QTextDocument::ImageResource, QUrl(url), QVariant(image));
    cursor.insertImage(image,url);
    QString origin_name = fileInfo.fileName();
    QString unique_name = generateUniqueFileName(origin_name);
    insertMsgList(_img_or_file_list, MsgType::IMG_MSG, url, QPixmap::fromImage(image), unique_name,
        total_size, fileMd5);
}

void MessageTextEdit::insertFiles(const QString& url) {
    QFileInfo fileInfo(url);
    if (fileInfo.isDir())
    {
        QMessageBox::information(this, "提示", "只允许拖拽单个文件!");
        return;
    }

    auto total_size = fileInfo.size();

    qint64 max_size = qint64(2) * 1024 * 1024 * 1024;

    if (total_size > max_size)
    {
        QMessageBox::information(this, "提示", "发送的文件大小不能大于100M");
        return;
    }

    // 计算文件MD5
    QString fileMd5 = calculateFileHash(url);

    if (fileMd5.isEmpty())
    {
        QMessageBox::warning(this, "错误", "无法计算文件MD5");
        return;
    }

    QPixmap pix = getFileIconPixmap(url);
    QTextCursor cursor = this->textCursor();
    cursor.insertImage(pix.toImage(), url);

    QString origin_name = fileInfo.fileName();
    QString unique_name = generateUniqueFileName(origin_name);
    insertMsgList(_img_or_file_list, MsgType::FILE_MSG, url, pix, unique_name,
        total_size, fileMd5);
}



bool MessageTextEdit::canInsertFromMimeData(const QMimeData *source) const
{
    return QTextEdit::canInsertFromMimeData(source);
}

void MessageTextEdit::insertFromMimeData(const QMimeData *source)
{
    QStringList urls = getUrl(source->text());

    if(urls.isEmpty())
        return;

    foreach (QString url, urls)
    {
         if(isImage(url))
             insertImages(url);
         else
             insertFiles(url);
    }
}

bool MessageTextEdit::isImage(QString url)
{
    QString imageFormat = "bmp,jpg,png,tif,gif,pcx,tga,exif,fpx,svg,psd,cdr,pcd,dxf,ufo,eps,ai,raw,wmf,webp";
    QStringList imageFormatList = imageFormat.split(",");
    QFileInfo fileInfo(url);
    QString suffix = fileInfo.suffix();
    if(imageFormatList.contains(suffix,Qt::CaseInsensitive)){
        return true;
    }
    return false;
}

void MessageTextEdit::insertMsgList(QVector<std::shared_ptr<MsgInfo>> &list, MsgType msgtype,
    QString text_or_url, QPixmap preview_pix,
    QString unique_name, uint64_t total_size, QString md5) {

    auto msg_info = std::make_shared<MsgInfo>(msgtype, text_or_url, preview_pix, unique_name, total_size, md5);
    list.append(msg_info);

}



QStringList MessageTextEdit::getUrl(QString text)
{
    QStringList urls;
    if(text.isEmpty()) return urls;

    QStringList list = text.split("\n");
    foreach (QString url, list) {
        if(!url.isEmpty()){
            QStringList str = url.split("///");
            if(str.size()>=2)
                urls.append(str.at(1));
        }
    }
    return urls;
}

QPixmap MessageTextEdit::getFileIconPixmap(const QString &url)
{
    QFileIconProvider provder;
    QFileInfo fileinfo(url);
    QIcon icon = provder.icon(fileinfo);

    QString strFileSize = getFileSize(fileinfo.size());
    //qDebug() << "FileSize=" << fileinfo.size();

    QFont font(QString("宋体"),10,QFont::Normal,false);
    QFontMetrics fontMetrics(font);
    QSize textSize = fontMetrics.size(Qt::TextSingleLine, fileinfo.fileName());

    QSize FileSize = fontMetrics.size(Qt::TextSingleLine, strFileSize);
    int maxWidth = textSize.width() > FileSize.width() ? textSize.width() :FileSize.width();
    QPixmap pix(50 + maxWidth + 10, 50);
    pix.fill();

    QPainter painter;
   // painter.setRenderHint(QPainter::Antialiasing, true);
    //painter.setFont(font);
    painter.begin(&pix);
    // 文件图标
    QRect rect(0, 0, 50, 50);
    painter.drawPixmap(rect, icon.pixmap(40,40));
    painter.setPen(Qt::black);
    // 文件名称
    QRect rectText(50+10, 3, textSize.width(), textSize.height());
    painter.drawText(rectText, fileinfo.fileName());
    // 文件大小
    QRect rectFile(50+10, textSize.height()+5, FileSize.width(), FileSize.height());
    painter.drawText(rectFile, strFileSize);
    painter.end();
    return pix;
}

QString MessageTextEdit::getFileSize(qint64 size)
{
    QString Unit;
    double num;
    if(size < 1024){
        num = size;
        Unit = "B";
    }
    else if(size < 1024 * 1224){
        num = size / 1024.0;
        Unit = "KB";
    }
    else if(size <  1024 * 1024 * 1024){
        num = size / 1024.0 / 1024.0;
        Unit = "MB";
    }
    else{
        num = size / 1024.0 / 1024.0/ 1024.0;
        Unit = "GB";
    }
    return QString::number(num,'f',2) + " " + Unit;
}

void MessageTextEdit::textEditChanged()
{
    //qDebug() << "text changed!" << endl;
}
