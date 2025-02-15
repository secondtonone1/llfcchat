## 搜索框
我们需要实现如下效果

![https://cdn.llfc.club/1717211817129.jpg](https://cdn.llfc.club/1717211817129.jpg)

输入框默认不显示关闭按钮，当输入文字后显示关闭按钮，点击关闭按钮清空文字

添加CustomizeEdit类，头文件

``` cpp
#ifndef CUSTOMIZEEDIT_H
#define CUSTOMIZEEDIT_H
#include <QLineEdit>
#include <QDebug>

class CustomizeEdit: public QLineEdit
{
    Q_OBJECT
public:
    CustomizeEdit(QWidget *parent = nullptr);
    void SetMaxLength(int maxLen);
protected:
    void focusOutEvent(QFocusEvent *event) override
    {
        // 执行失去焦点时的处理逻辑
        //qDebug() << "CustomizeEdit focusout";
        // 调用基类的focusOutEvent()方法，保证基类的行为得到执行
        QLineEdit::focusOutEvent(event);
        //发送失去焦点得信号
        emit sig_foucus_out();
    }
private:
    void limitTextLength(QString text) {
        if(_max_len <= 0){
            return;
        }

        QByteArray byteArray = text.toUtf8();

        if (byteArray.size() > _max_len) {
            byteArray = byteArray.left(_max_len);
            this->setText(QString::fromUtf8(byteArray));
        }
    }

    int _max_len;
signals:
    void sig_foucus_out();
};

#endif // CUSTOMIZEEDIT_H
```

源文件

``` cpp
#include "customizeedit.h"

CustomizeEdit::CustomizeEdit(QWidget *parent):QLineEdit (parent),_max_len(0)
{
    connect(this, &QLineEdit::textChanged, this, &CustomizeEdit::limitTextLength);
}

void CustomizeEdit::SetMaxLength(int maxLen)
{
    _max_len = maxLen;
}
```
设计师界面类里将`ui->search_edit`提升为CustomizeEdit

在ChatDialog的构造函数中设置输入的长度限制以及关闭等图标的配置

``` cpp
    QAction *searchAction = new QAction(ui->search_edit);
    searchAction->setIcon(QIcon(":/res/search.png"));
    ui->search_edit->addAction(searchAction,QLineEdit::LeadingPosition);
    ui->search_edit->setPlaceholderText(QStringLiteral("搜索"));


    // 创建一个清除动作并设置图标
    QAction *clearAction = new QAction(ui->search_edit);
    clearAction->setIcon(QIcon(":/res/close_transparent.png"));
    // 初始时不显示清除图标
    // 将清除动作添加到LineEdit的末尾位置
    ui->search_edit->addAction(clearAction, QLineEdit::TrailingPosition);

    // 当需要显示清除图标时，更改为实际的清除图标
    connect(ui->search_edit, &QLineEdit::textChanged, [clearAction](const QString &text) {
        if (!text.isEmpty()) {
            clearAction->setIcon(QIcon(":/res/close_search.png"));
        } else {
            clearAction->setIcon(QIcon(":/res/close_transparent.png")); // 文本为空时，切换回透明图标
        }

    });

    // 连接清除动作的触发信号到槽函数，用于清除文本
    connect(clearAction, &QAction::triggered, [this, clearAction]() {
        ui->search_edit->clear();
        clearAction->setIcon(QIcon(":/res/close_transparent.png")); // 清除文本后，切换回透明图标
        ui->search_edit->clearFocus();
        //清除按钮被按下则不显示搜索框
        //ShowSearch(false);
    });

    ui->search_edit->SetMaxLength(15);
```
stylesheet.qss 中修改样式
``` cpp
#search_wid{
     background-color:rgb(247,247,247);
}

#search_edit {
    border: 2px solid #f1f1f1;
}
```

## 聊天记录列表
创建C++ 类ChatUserList

``` cpp
#ifndef CHATUSERLIST_H
#define CHATUSERLIST_H
#include <QListWidget>
#include <QWheelEvent>
#include <QEvent>
#include <QScrollBar>
#include <QDebug>

class ChatUserList: public QListWidget
{
    Q_OBJECT
public:
    ChatUserList(QWidget *parent = nullptr);
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void sig_loading_chat_user();
};

#endif // CHATUSERLIST_H
```
实现

``` cpp
#include "chatuserlist.h"

ChatUserList::ChatUserList(QWidget *parent):QListWidget(parent)
{
    Q_UNUSED(parent);
     this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
     this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 安装事件过滤器
    this->viewport()->installEventFilter(this);
}

bool ChatUserList::eventFilter(QObject *watched, QEvent *event)
{
    // 检查事件是否是鼠标悬浮进入或离开
    if (watched == this->viewport()) {
        if (event->type() == QEvent::Enter) {
            // 鼠标悬浮，显示滚动条
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        } else if (event->type() == QEvent::Leave) {
            // 鼠标离开，隐藏滚动条
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }
    }

    // 检查事件是否是鼠标滚轮事件
    if (watched == this->viewport() && event->type() == QEvent::Wheel) {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
        int numDegrees = wheelEvent->angleDelta().y() / 8;
        int numSteps = numDegrees / 15; // 计算滚动步数

        // 设置滚动幅度
        this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() - numSteps);

        // 检查是否滚动到底部
        QScrollBar *scrollBar = this->verticalScrollBar();
        int maxScrollValue = scrollBar->maximum();
        int currentValue = scrollBar->value();
        //int pageSize = 10; // 每页加载的联系人数量

        if (maxScrollValue - currentValue <= 0) {
            // 滚动到底部，加载新的联系人
            qDebug()<<"load more chat user";
            //发送信号通知聊天界面加载更多聊天内容
            emit sig_loading_chat_user();
         }

        return true; // 停止事件传递
    }

    return QListWidget::eventFilter(watched, event);
}
```

在设计师界面类里提升`ui->chat_user_list`为ChatUserList

在ChatDialog构造函数和搜索清除按钮的回调中增加
``` cpp
 ShowSearch(false);
```
该函数的具体实现
``` cpp
void ChatDialog::ShowSearch(bool bsearch)
{
    if(bsearch){
        ui->chat_user_list->hide();
        ui->con_user_list->hide();
        ui->search_list->show();
        _mode = ChatUIMode::SearchMode;
    }else if(_state == ChatUIMode::ChatMode){
        ui->chat_user_list->show();
        ui->con_user_list->hide();
        ui->search_list->hide();
        _mode = ChatUIMode::ChatMode;
    }else if(_state == ChatUIMode::ContactMode){
        ui->chat_user_list->hide();
        ui->search_list->hide();
        ui->con_user_list->show();
        _mode = ChatUIMode::ContactMode;
    }
}
```
ChatDialog类中声明添加
``` cpp
    ChatUIMode _mode;
    ChatUIMode _state;
    bool _b_loading;
```
构造函数的初始化列表初始化这些模式和状态
``` cpp
ChatDialog::ChatDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChatDialog),_mode(ChatUIMode::ChatMode),
    _state(ChatUIMode::ChatMode),_b_loading(false){//...}
```
为了让用户聊天列表更美观，修改qss文件
``` cpp
#chat_user_wid{
    background-color:rgb(0,0,0);
}

#chat_user_list {
    background-color: rgb(247,247,248);
    border: none;
}


#chat_user_list::item:selected {
    background-color: #d3d7d4;
    border: none;
    outline: none;
}

#chat_user_list::item:hover {
    background-color: rgb(206,207,208);
    border: none;
    outline: none;
}

#chat_user_list::focus {
    border: none;
    outline: none;
}
```
## 添加聊天item

我们要为聊天列表添加item，每个item包含的样式为

![https://cdn.llfc.club/1717215988933.jpg](https://cdn.llfc.club/1717215988933.jpg)

对于这样的列表元素，我们采用设计师界面类设计非常方便, 新建设计师界面类ChatUserWid, 在ChatUserWid.ui中拖动布局如下

![https://cdn.llfc.club/1717217007100.jpg](https://cdn.llfc.club/1717217007100.jpg)

我们定义一个基类ListItemBase

``` cpp
#ifndef LISTITEMBASE_H
#define LISTITEMBASE_H
#include <QWidget>
#include "global.h"

class ListItemBase : public QWidget
{
    Q_OBJECT
public:
    explicit ListItemBase(QWidget *parent = nullptr);
    void SetItemType(ListItemType itemType);

    ListItemType GetItemType();

private:
    ListItemType _itemType;

public slots:

signals:


};

#endif // LISTITEMBASE_H
```
我们实现这个基类
``` cpp
#include "listitembase.h"

ListItemBase::ListItemBase(QWidget *parent) : QWidget(parent)
{

}

void ListItemBase::SetItemType(ListItemType itemType)
{
    _itemType = itemType;
}

ListItemType ListItemBase::GetItemType()
{
    return _itemType;
}
```
我们实现ChatUserWid

``` cpp
#ifndef CHATUSERWID_H
#define CHATUSERWID_H

#include <QWidget>
#include "listitembase.h"
namespace Ui {
class ChatUserWid;
}

class ChatUserWid : public ListItemBase
{
    Q_OBJECT

public:
    explicit ChatUserWid(QWidget *parent = nullptr);
    ~ChatUserWid();

    QSize sizeHint() const override {
        return QSize(250, 70); // 返回自定义的尺寸
    }

    void SetInfo(QString name, QString head, QString msg);

private:
    Ui::ChatUserWid *ui;
    QString _name;
    QString _head;
    QString _msg;
};

#endif // CHATUSERWID_H

```
具体实现
``` cpp
#include "chatuserwid.h"
#include "ui_chatuserwid.h"

ChatUserWid::ChatUserWid(QWidget *parent) :
    ListItemBase(parent),
    ui(new Ui::ChatUserWid)
{
    ui->setupUi(this);
    SetItemType(ListItemType::CHAT_USER_ITEM);
}

ChatUserWid::~ChatUserWid()
{
    delete ui;
}

void ChatUserWid::SetInfo(QString name, QString head, QString msg)
{
    _name = name;
    _head = head;
    _msg = msg;
    // 加载图片
    QPixmap pixmap(_head);

    // 设置图片自动缩放
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);

    ui->user_name_lb->setText(_name);
    ui->user_chat_lb->setText(_msg);
}
```
在ChatDialog里定义一些全局的变量用来做测试
``` cpp
std::vector<QString>  strs ={"hello world !",
                             "nice to meet u",
                             "New year，new life",
                            "You have to love yourself",
                            "My love is written in the wind ever since the whole world is you"};

std::vector<QString> heads = {
    ":/res/head_1.jpg",
    ":/res/head_2.jpg",
    ":/res/head_3.jpg",
    ":/res/head_4.jpg",
    ":/res/head_5.jpg"
};

std::vector<QString> names = {
    "llfc",
    "zack",
    "golang",
    "cpp",
    "java",
    "nodejs",
    "python",
    "rust"
};
```
这些数据只是测试数据，实际数据是后端传输过来的，我们目前只测试界面功能，用测试数据即可，写一个函数根据上面的数据添加13条item记录
``` cpp
void ChatDialog::addChatUserList()
{
    // 创建QListWidgetItem，并设置自定义的widget
    for(int i = 0; i < 13; i++){
        int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
        int str_i = randomValue%strs.size();
        int head_i = randomValue%heads.size();
        int name_i = randomValue%names.size();

        auto *chat_user_wid = new ChatUserWid();
        chat_user_wid->SetInfo(names[name_i], heads[head_i], strs[str_i]);
        QListWidgetItem *item = new QListWidgetItem;
        //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
        item->setSizeHint(chat_user_wid->sizeHint());
        ui->chat_user_list->addItem(item);
        ui->chat_user_list->setItemWidget(item, chat_user_wid);
    }
}
```
在ChatDialog构造函数中添加
``` cpp
addChatUserList();
```
完善界面效果，新增qss
``` css
ChatUserWid {
    background-color:rgb(247,247,247);
    border: none;
}

#user_chat_lb{
    color:rgb(153,153,153);
    font-size: 12px;
    font-family: "Microsoft YaHei";
}

#user_name_lb{
    color:rgb(0,0,0);
    font-size: 14px;
    font-weight: normal;
    font-family: "Microsoft YaHei";
}

#time_wid #time_lb{
   color:rgb(140,140,140);
   font-size: 12px;
   font-family: "Microsoft YaHei";
}

QScrollBar:vertical {
    background: transparent; /* 将轨道背景设置为透明 */
    width: 8px; /* 滚动条宽度，根据需要调整 */
    margin: 0px; /* 移除滚动条与滑块之间的间距 */
}

QScrollBar::handle:vertical {
    background: rgb(173,170,169); /* 滑块颜色 */
    min-height: 10px; /* 滑块最小高度，根据需要调整 */
    border-radius: 4px; /* 滑块边缘圆角，调整以形成椭圆形状 */
}

QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    height: 0px; /* 移除上下按钮 */
    border: none; /* 移除边框 */
    background: transparent; /* 背景透明 */
}

QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
    background: none; /* 页面滚动部分背景透明 */
}
```

## 测试效果

![https://cdn.llfc.club/1717218961063.jpg](https://cdn.llfc.club/1717218961063.jpg)

## 源码链接

[https://gitee.com/secondtonone1/llfcchat](https://gitee.com/secondtonone1/llfcchat)

## 视频链接

[https://www.bilibili.com/video/BV13Z421W7WA/?spm_id_from=333.788&vd_source=8be9e83424c2ed2c9b2a3ed1d01385e9](https://www.bilibili.com/video/BV13Z421W7WA/?spm_id_from=333.788&vd_source=8be9e83424c2ed2c9b2a3ed1d01385e9)

