## 侧边栏按钮

我们接下来实现侧边栏按钮功能，希望点击一个按钮，清空其他按钮的选中状态。而我们又希望按钮上面能在有新的通知的时候出现红点的图标，所以不能用简单的按钮，要用自定义的一个widget实现点击效果

我们自定义StateWidget ，声明如下

``` cpp
class StateWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StateWidget(QWidget *parent = nullptr);

    void SetState(QString normal="", QString hover="", QString press="",
                  QString select="", QString select_hover="", QString select_press="");

    ClickLbState GetCurState();
    void ClearState();

    void SetSelected(bool bselected);
    void AddRedPoint();
    void ShowRedPoint(bool show=true);

protected:
    void paintEvent(QPaintEvent* event);

    virtual void mousePressEvent(QMouseEvent *ev) override;
    virtual void mouseReleaseEvent(QMouseEvent *ev) override;
    virtual void enterEvent(QEvent* event) override;
    virtual void leaveEvent(QEvent* event) override;

private:

    QString _normal;
    QString _normal_hover;
    QString _normal_press;

    QString _selected;
    QString _selected_hover;
    QString _selected_press;

    ClickLbState _curstate;
    QLabel * _red_point;

signals:
    void clicked(void);

signals:

public slots:
};
```
接下来实现定义

``` cpp
StateWidget::StateWidget(QWidget *parent): QWidget(parent),_curstate(ClickLbState::Normal)
{
    setCursor(Qt::PointingHandCursor);
    //添加红点
    AddRedPoint();
}

void StateWidget::SetState(QString normal, QString hover, QString press, QString select, QString select_hover, QString select_press)
{
    _normal = normal;
    _normal_hover = hover;
    _normal_press = press;

    _selected = select;
    _selected_hover = select_hover;
    _selected_press = select_press;

    setProperty("state",normal);
    repolish(this);
}

ClickLbState StateWidget::GetCurState()
{
    return _curstate;
}

void StateWidget::ClearState()
{
    _curstate = ClickLbState::Normal;
    setProperty("state",_normal);
    repolish(this);
    update();
}

void StateWidget::SetSelected(bool bselected)
{
    if(bselected){
        _curstate = ClickLbState::Selected;
        setProperty("state",_selected);
        repolish(this);
        update();
        return;
    }

    _curstate = ClickLbState::Normal;
    setProperty("state",_normal);
    repolish(this);
    update();
    return;
}


void StateWidget::AddRedPoint()
{
    //添加红点示意图
    _red_point = new QLabel();
    _red_point->setObjectName("red_point");
    QVBoxLayout* layout2 = new QVBoxLayout;
    _red_point->setAlignment(Qt::AlignCenter);
    layout2->addWidget(_red_point);
    layout2->setMargin(0);
    this->setLayout(layout2);
    _red_point->setVisible(false);
}

void StateWidget::ShowRedPoint(bool show)
{
    _red_point->setVisible(true);
}

void StateWidget::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    return;
}

void StateWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if(_curstate == ClickLbState::Selected){
            qDebug()<<"PressEvent , already to selected press: "<< _selected_press;
            //emit clicked();
            // 调用基类的mousePressEvent以保证正常的事件处理
            QWidget::mousePressEvent(event);
            return;
        }

        if(_curstate == ClickLbState::Normal){
            qDebug()<<"PressEvent , change to selected press: "<< _selected_press;
            _curstate = ClickLbState::Selected;
            setProperty("state",_selected_press);
            repolish(this);
            update();
        }

        return;
    }
    // 调用基类的mousePressEvent以保证正常的事件处理
    QWidget::mousePressEvent(event);
}

void StateWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if(_curstate == ClickLbState::Normal){
            //qDebug()<<"ReleaseEvent , change to normal hover: "<< _normal_hover;
            setProperty("state",_normal_hover);
            repolish(this);
            update();

        }else{
            //qDebug()<<"ReleaseEvent , change to select hover: "<< _selected_hover;
            setProperty("state",_selected_hover);
            repolish(this);
            update();
        }
        emit clicked();
        return;
    }
    // 调用基类的mousePressEvent以保证正常的事件处理
    QWidget::mousePressEvent(event);
}

void StateWidget::enterEvent(QEvent *event)
{
    // 在这里处理鼠标悬停进入的逻辑
    if(_curstate == ClickLbState::Normal){
         //qDebug()<<"enter , change to normal hover: "<< _normal_hover;
        setProperty("state",_normal_hover);
        repolish(this);
        update();

    }else{
         //qDebug()<<"enter , change to selected hover: "<< _selected_hover;
        setProperty("state",_selected_hover);
        repolish(this);
        update();
    }

    QWidget::enterEvent(event);
}

void StateWidget::leaveEvent(QEvent *event)
{
    // 在这里处理鼠标悬停离开的逻辑
    if(_curstate == ClickLbState::Normal){
        // qDebug()<<"leave , change to normal : "<< _normal;
        setProperty("state",_normal);
        repolish(this);
        update();

    }else{
        // qDebug()<<"leave , change to select normal : "<< _selected;
        setProperty("state",_selected);
        repolish(this);
        update();
    }
    QWidget::leaveEvent(event);
}

```
为了让按钮好看一点，我们修改下qss文件
``` css
#chat_user_name {
    color:rgb(153,153,153);
    font-size: 14px;
    font-family: "Microsoft YaHei";
}

#side_chat_lb[state='normal']{
    border-image: url(:/res/chat_icon.png);
}

#side_chat_lb[state='hover']{
    border-image: url(:/res/chat_icon_hover.png);
}

#side_chat_lb[state='pressed']{
    border-image: url(:/res/chat_icon_press.png);
}

#side_chat_lb[state='selected_normal']{
    border-image: url(:/res/chat_icon_press.png);
}

#side_chat_lb[state='selected_hover']{
    border-image: url(:/res/chat_icon_press.png);
}

#side_chat_lb[state='selected_pressed']{
    border-image: url(:/res/chat_icon_press.png);
}

#side_contact_lb[state='normal']{
    border-image: url(:/res/contact_list.png);
}

#side_contact_lb[state='hover']{
    border-image: url(:/res/contact_list_hover.png);
}

#side_contact_lb[state='pressed']{
    border-image: url(:/res/contact_list_press.png);
}

#side_contact_lb[state='selected_normal']{
    border-image: url(:/res/contact_list_press.png);
}

#side_contact_lb[state='selected_hover']{
    border-image: url(:/res/contact_list_press.png);
}

#side_contact_lb[state='selected_pressed']{
    border-image: url(:/res/contact_list_press.png);
}
```

回到ChatDialog.ui中，将side_chat_lb改为StateWidget，side_contact_lb改为StateWidget。

![https://cdn.llfc.club/1719028635439.jpg](https://cdn.llfc.club/1719028635439.jpg)

接下来回到ChatDialog.cpp中构造函数中添加
``` cpp
   QPixmap pixmap(":/res/head_1.jpg");
    ui->side_head_lb->setPixmap(pixmap); // 将图片设置到QLabel上
    QPixmap scaledPixmap = pixmap.scaled( ui->side_head_lb->size(), Qt::KeepAspectRatio); // 将图片缩放到label的大小
    ui->side_head_lb->setPixmap(scaledPixmap); // 将缩放后的图片设置到QLabel上
    ui->side_head_lb->setScaledContents(true); // 设置QLabel自动缩放图片内容以适应大小

    ui->side_chat_lb->setProperty("state","normal");

    ui->side_chat_lb->SetState("normal","hover","pressed","selected_normal","selected_hover","selected_pressed");

    ui->side_contact_lb->SetState("normal","hover","pressed","selected_normal","selected_hover","selected_pressed");

    AddLBGroup(ui->side_chat_lb);
    AddLBGroup(ui->side_contact_lb);

    connect(ui->side_chat_lb, &StateWidget::clicked, this, &ChatDialog::slot_side_chat);
    connect(ui->side_contact_lb, &StateWidget::clicked, this, &ChatDialog::slot_side_contact);
```
切换函数中实现如下
``` cpp
void ChatDialog::slot_side_chat()
{
    qDebug()<< "receive side chat clicked";
    ClearLabelState(ui->side_chat_lb);
    ui->stackedWidget->setCurrentWidget(ui->chat_page);
    _state = ChatUIMode::ChatMode;
    ShowSearch(false);
}
```
上述函数我们实现了清楚其他标签选中状态，只将被点击的标签设置为选中的效果,核心功能是下面
``` cpp
void ChatDialog::ClearLabelState(StateWidget *lb)
{
    for(auto & ele: _lb_list){
        if(ele == lb){
            continue;
        }

        ele->ClearState();
    }
}
```
我们在构造函数里将要管理的标签通过AddGroup函数加入_lb_list实现管理
``` cpp
void ChatDialog::AddLBGroup(StateWidget *lb)
{
     _lb_list.push_back(lb);
}
```
## 搜索列表类
在pro中添加我们自定义一个搜索列表类
``` cpp
class SearchList: public QListWidget
{
    Q_OBJECT
public:
    SearchList(QWidget *parent = nullptr);
    void CloseFindDlg();
    void SetSearchEdit(QWidget* edit);
protected:
    bool eventFilter(QObject *watched, QEvent *event) override {
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

            return true; // 停止事件传递
        }

        return QListWidget::eventFilter(watched, event);
    }
private:
    void waitPending(bool pending = true);
    bool _send_pending;
    void addTipItem();
    std::shared_ptr<QDialog> _find_dlg;
    QWidget* _search_edit;
    LoadingDlg * _loadingDialog;
private slots:
    void slot_item_clicked(QListWidgetItem *item);
    void slot_user_search(std::shared_ptr<SearchInfo> si);
signals:

};
```
然后在构造函数中初始化条目列表
``` cpp
SearchList::SearchList(QWidget *parent):QListWidget(parent),_find_dlg(nullptr), _search_edit(nullptr), _send_pending(false)
{
    Q_UNUSED(parent);
     this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
     this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 安装事件过滤器
    this->viewport()->installEventFilter(this);
    //连接点击的信号和槽
    connect(this, &QListWidget::itemClicked, this, &SearchList::slot_item_clicked);
    //添加条目
    addTipItem();
    //连接搜索条目
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_user_search, this, &SearchList::slot_user_search);
}
```
addTipItem是用来添加一个一个条目的
``` cpp
void SearchList::addTipItem()
{
    auto *invalid_item = new QWidget();
    QListWidgetItem *item_tmp = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item_tmp->setSizeHint(QSize(250,10));
    this->addItem(item_tmp);
    invalid_item->setObjectName("invalid_item");
    this->setItemWidget(item_tmp, invalid_item);
    item_tmp->setFlags(item_tmp->flags() & ~Qt::ItemIsSelectable);


    auto *add_user_item = new AddUserItem();
    QListWidgetItem *item = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item->setSizeHint(add_user_item->sizeHint());
    this->addItem(item);
    this->setItemWidget(item, add_user_item);
}
```
sig_user_search可以先在TcpMgr中声明信号
``` cpp
 void sig_user_search(std::shared_ptr<SearchInfo>);
```
SearchInfo定义在userdata.h中
``` cpp
class SearchInfo {
public:
	SearchInfo(int uid, QString name, QString nick, QString desc, int sex);
	int _uid;
	QString _name;
	QString _nick;
	QString _desc;
	int _sex;
};
```
接下来实现我们自定义的AddUserItem, 在pro中添加qt设计师界面类AddUserItem
``` cpp
class AddUserItem : public ListItemBase
{
    Q_OBJECT

public:
    explicit AddUserItem(QWidget *parent = nullptr);
    ~AddUserItem();
    QSize sizeHint() const override {
        return QSize(250, 70); // 返回自定义的尺寸
    }
protected:

private:
    Ui::AddUserItem *ui;
};
```
实现
``` cpp
AddUserItem::AddUserItem(QWidget *parent) :
    ListItemBase(parent),
    ui(new Ui::AddUserItem)
{
    ui->setupUi(this);
    SetItemType(ListItemType::ADD_USER_TIP_ITEM);
}

AddUserItem::~AddUserItem()
{
    delete ui;
}
```
我们将ChatDialog.ui中将search_list升级为SearchList类型
## 美化界面
我们用qss美化界面
``` css
#search_edit {
    border: 2px solid #f1f1f1;
}

/* 搜索框列表*/
#search_list {
    background-color: rgb(247,247,248);
    border: none;
}

#search_list::item:selected {
    background-color: #d3d7d4;
    border: none;
    outline: none;
}

#search_list::item:hover {
    background-color: rgb(206,207,208);
    border: none;
    outline: none;
}

#search_list::focus {
    border: none;
    outline: none;
}

#invalid_item {
    background-color: #eaeaea;
    border: none;
}

#add_tip {
    border-image: url(:/res/addtip.png);
}

#right_tip{
 border-image: url(:/res/right_tip.png);
}

#message_tip{
  text-align: center;
  font-family: "Microsoft YaHei";
  font-size: 12pt;
}

```
我们在ChatDialog的构造函数中添加
``` cpp
//链接搜索框输入变化
    connect(ui->search_edit, &QLineEdit::textChanged, this, &ChatDialog::slot_text_changed);
```
slot_text_changed槽函数中实现
``` cpp
void ChatDialog::slot_text_changed(const QString &str)
{
    //qDebug()<< "receive slot text changed str is " << str;
    if (!str.isEmpty()) {
        ShowSearch(true);
    }
}
```
## 源码和视频

再次启动后在输入框输入文字，就会显示搜索框

![https://cdn.llfc.club/1719113143252.jpg](https://cdn.llfc.club/1719113143252.jpg)

## 视频

[https://www.bilibili.com/video/BV1uM4m1U7MP/?spm_id_from=333.999.0.0&vd_source=8be9e83424c2ed2c9b2a3ed1d01385e9](https://www.bilibili.com/video/BV1uM4m1U7MP/?spm_id_from=333.999.0.0&vd_source=8be9e83424c2ed2c9b2a3ed1d01385e9)

## 源码链接
[https://gitee.com/secondtonone1/llfcchat](https://gitee.com/secondtonone1/llfcchat) 