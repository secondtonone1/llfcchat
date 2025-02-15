## 事件过滤器
我们为了实现点击界面某个位置判断是否隐藏搜索框的功能。我们期待当鼠标点击搜索列表之外的区域时显示隐藏搜索框恢复聊天界面。
点击搜索列表则不隐藏搜索框。可以通过重载ChatDialog的EventFilter函数实现点击功能
``` cpp
bool ChatDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
       QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
       handleGlobalMousePress(mouseEvent);
    }
    return QDialog::eventFilter(watched, event);
}
```
具体判断全局鼠标按下位置和功能
``` cpp
void ChatDialog::handleGlobalMousePress(QMouseEvent *event)
{
    // 实现点击位置的判断和处理逻辑
    // 先判断是否处于搜索模式，如果不处于搜索模式则直接返回
    if( _mode != ChatUIMode::SearchMode){
        return;
    }

    // 将鼠标点击位置转换为搜索列表坐标系中的位置
    QPoint posInSearchList = ui->search_list->mapFromGlobal(event->globalPos());
    // 判断点击位置是否在聊天列表的范围内
    if (!ui->search_list->rect().contains(posInSearchList)) {
        // 如果不在聊天列表内，清空输入框
        ui->search_edit->clear();
        ShowSearch(false);
    }
}
```
在ChatDialog构造函数中添加事件过滤器
``` cpp
//检测鼠标点击位置判断是否要清空搜索框
this->installEventFilter(this); // 安装事件过滤器

//设置聊天label选中状态
ui->side_chat_lb->SetSelected(true);
```
这样就可以实现在ChatDialog中点击其他位置隐藏SearchList列表了。

## 查找结果
在项目中添加FindSuccessDlg设计师界面类，其布局如下

![https://cdn.llfc.club/1719726260598.jpg](https://cdn.llfc.club/1719726260598.jpg)

属性管理界面如下

![https://cdn.llfc.club/1719726584051.jpg](https://cdn.llfc.club/1719726584051.jpg)

FindSuccessDlg声明如下
``` cpp
class FindSuccessDlg : public QDialog
{
    Q_OBJECT

public:
    explicit FindSuccessDlg(QWidget *parent = nullptr);
    ~FindSuccessDlg();
    void SetSearchInfo(std::shared_ptr<SearchInfo> si);
private slots:
    void on_add_friend_btn_clicked();

private:
    Ui::FindSuccessDlg *ui;
    QWidget * _parent;
    std::shared_ptr<SearchInfo> _si;
};
```
FindSuccessDlg实现如下
``` cpp
FindSuccessDlg::FindSuccessDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FindSuccessDlg)
{
    ui->setupUi(this);
    // 设置对话框标题
    setWindowTitle("添加");
    // 隐藏对话框标题栏
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    // 获取当前应用程序的路径
    QString app_path = QCoreApplication::applicationDirPath();
    QString pix_path = QDir::toNativeSeparators(app_path +
                             QDir::separator() + "static"+QDir::separator()+"head_1.jpg");
    QPixmap head_pix(pix_path);
    head_pix = head_pix.scaled(ui->head_lb->size(),
            Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->head_lb->setPixmap(head_pix);
    ui->add_friend_btn->SetState("normal","hover","press");
    this->setModal(true);
}

FindSuccessDlg::~FindSuccessDlg()
{
    qDebug()<<"FindSuccessDlg destruct";
    delete ui;
}

void FindSuccessDlg::SetSearchInfo(std::shared_ptr<SearchInfo> si)
{
    ui->name_lb->setText(si->_name);
    _si = si;
}

void FindSuccessDlg::on_add_friend_btn_clicked()
{
    //todo... 添加好友界面弹出
}
```
在SearchList 的slot_item_clicked函数中添加点击条目处理逻辑
``` cpp
void SearchList::slot_item_clicked(QListWidgetItem *item)
{
    QWidget *widget = this->itemWidget(item); //获取自定义widget对象
    if(!widget){
        qDebug()<< "slot item clicked widget is nullptr";
        return;
    }

    // 对自定义widget进行操作， 将item 转化为基类ListItemBase
    ListItemBase *customItem = qobject_cast<ListItemBase*>(widget);
    if(!customItem){
        qDebug()<< "slot item clicked widget is nullptr";
        return;
    }

    auto itemType = customItem->GetItemType();
    if(itemType == ListItemType::INVALID_ITEM){
        qDebug()<< "slot invalid item clicked ";
        return;
    }

    if(itemType == ListItemType::ADD_USER_TIP_ITEM){

        //todo ...
        _find_dlg = std::make_shared<FindSuccessDlg>(this);
        auto si = std::make_shared<SearchInfo>(0,"llfc","llfc","hello , my friend!",0);
        (std::dynamic_pointer_cast<FindSuccessDlg>(_find_dlg))->SetSearchInfo(si);
        _find_dlg->show();
        return;
    }

    //清楚弹出框
    CloseFindDlg();

}
```
这样我们在输入框输入文字，点击搜索列表中搜索添加好友的item，就能弹出搜索结果对话框了。这里只做界面演示，之后会改为像服务器发送请求获取搜索结果。
## pro的改写
我们对项目的pro做了调整，更新了static文件夹的拷贝以及编码utf-8的设定
``` bash
QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = llfcchat
TEMPLATE = app
RC_ICONS = icon.ico
DESTDIR = ./bin
DEFINES += QT_DEPRECATED_WARNINGS
CONFIG += c++11

SOURCES += \
        adduseritem.cpp \
        bubbleframe.cpp \
        chatdialog.cpp \
        chatitembase.cpp \
        chatpage.cpp \
        chatuserlist.cpp \
        chatuserwid.cpp \
        chatview.cpp \
        clickedbtn.cpp \
        clickedlabel.cpp \
        customizeedit.cpp \
        findsuccessdlg.cpp \
        global.cpp \
        httpmgr.cpp \
        listitembase.cpp \
        loadingdlg.cpp \
        logindialog.cpp \
        main.cpp \
        mainwindow.cpp \
        messagetextedit.cpp \
        picturebubble.cpp \
        registerdialog.cpp \
        resetdialog.cpp \
        searchlist.cpp \
        statewidget.cpp \
        tcpmgr.cpp \
        textbubble.cpp \
        timerbtn.cpp \
        userdata.cpp \
        usermgr.cpp

HEADERS += \
        adduseritem.h \
        bubbleframe.h \
        chatdialog.h \
        chatitembase.h \
        chatpage.h \
        chatuserlist.h \
        chatuserwid.h \
        chatview.h \
        clickedbtn.h \
        clickedlabel.h \
        customizeedit.h \
        findsuccessdlg.h \
        global.h \
        httpmgr.h \
        listitembase.h \
        loadingdlg.h \
        logindialog.h \
        mainwindow.h \
        messagetextedit.h \
        picturebubble.h \
        registerdialog.h \
        resetdialog.h \
        searchlist.h \
        singleton.h \
        statewidget.h \
        tcpmgr.h \
        textbubble.h \
        timerbtn.h \
        userdata.h \
        usermgr.h

FORMS += \
        adduseritem.ui \
        chatdialog.ui \
        chatpage.ui \
        chatuserwid.ui \
        findsuccessdlg.ui \
        loadingdlg.ui \
        logindialog.ui \
        mainwindow.ui \
        registerdialog.ui \
        resetdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    rc.qrc

DISTFILES += \
    config.ini


CONFIG(debug, debug | release) {
    #指定要拷贝的文件目录为工程目录下release目录下的所有dll、lib文件，例如工程目录在D:\QT\Test
    #PWD就为D:/QT/Test，DllFile = D:/QT/Test/release/*.dll
    TargetConfig = $${PWD}/config.ini
    #将输入目录中的"/"替换为"\"
    TargetConfig = $$replace(TargetConfig, /, \\)
    #将输出目录中的"/"替换为"\"
    OutputDir =  $${OUT_PWD}/$${DESTDIR}
    OutputDir = $$replace(OutputDir, /, \\)
    //执行copy命令
    QMAKE_POST_LINK += copy /Y \"$$TargetConfig\" \"$$OutputDir\" &

    # 首先，定义static文件夹的路径
    StaticDir = $${PWD}/static
    # 将路径中的"/"替换为"\"
    StaticDir = $$replace(StaticDir, /, \\)
    #message($${StaticDir})
    # 使用xcopy命令拷贝文件夹，/E表示拷贝子目录及其内容，包括空目录。/I表示如果目标不存在则创建目录。/Y表示覆盖现有文件而不提示。
    QMAKE_POST_LINK += xcopy /Y /E /I \"$$StaticDir\" \"$$OutputDir\\static\\\"

}else{
      #release
    message("release mode")
    #指定要拷贝的文件目录为工程目录下release目录下的所有dll、lib文件，例如工程目录在D:\QT\Test
    #PWD就为D:/QT/Test，DllFile = D:/QT/Test/release/*.dll
    TargetConfig = $${PWD}/config.ini
    #将输入目录中的"/"替换为"\"
    TargetConfig = $$replace(TargetConfig, /, \\)
    #将输出目录中的"/"替换为"\"
    OutputDir =  $${OUT_PWD}/$${DESTDIR}
    OutputDir = $$replace(OutputDir, /, \\)
    //执行copy命令
    QMAKE_POST_LINK += copy /Y \"$$TargetConfig\" \"$$OutputDir\"

    # 首先，定义static文件夹的路径
    StaticDir = $${PWD}/static
    # 将路径中的"/"替换为"\"
    StaticDir = $$replace(StaticDir, /, \\)
    #message($${StaticDir})
    # 使用xcopy命令拷贝文件夹，/E表示拷贝子目录及其内容，包括空目录。/I表示如果目标不存在则创建目录。/Y表示覆盖现有文件而不提示。
     QMAKE_POST_LINK += xcopy /Y /E /I \"$$StaticDir\" \"$$OutputDir\\static\\\"
}

win32-msvc*:QMAKE_CXXFLAGS += /wd"4819" /utf-8
```
## 视频

[https://www.bilibili.com/video/BV1ww4m1e72G/](https://www.bilibili.com/video/BV1ww4m1e72G/)

## 源码链接
[https://gitee.com/secondtonone1/llfcchat](https://gitee.com/secondtonone1/llfcchat) 