## 简介

今日实现界面效果

![https://cdn.llfc.club/1721547194830.jpg](https://cdn.llfc.club/1721547194830.jpg)

## 联系人列表
我们自定义一个ChatUserList类，用来管理聊天列表。其声明如下：
``` cpp
class ContactUserList : public QListWidget
{
    Q_OBJECT
public:
    ContactUserList(QWidget *parent = nullptr);
    void ShowRedPoint(bool bshow = true);
protected:
    bool eventFilter(QObject *watched, QEvent *event) override ;

private:
    void addContactUserList();

public slots:
     void slot_item_clicked(QListWidgetItem *item);
//     void slot_add_auth_firend(std::shared_ptr<AuthInfo>);
//     void slot_auth_rsp(std::shared_ptr<AuthRsp>);
signals:
    void sig_loading_contact_user();
    void sig_switch_apply_friend_page();
    void sig_switch_friend_info_page();
private:
    ConUserItem* _add_friend_item;
    QListWidgetItem * _groupitem;
};

```
具体实现
``` cpp
ContactUserList::ContactUserList(QWidget *parent)
{
    Q_UNUSED(parent);
     this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
     this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 安装事件过滤器
     this->viewport()->installEventFilter(this);

    //模拟从数据库或者后端传输过来的数据,进行列表加载
    addContactUserList();
    //连接点击的信号和槽
    connect(this, &QListWidget::itemClicked, this, &ContactUserList::slot_item_clicked);
//    //链接对端同意认证后通知的信号
//    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_add_auth_friend,this,
//            &ContactUserList::slot_add_auth_firend);

//    //链接自己点击同意认证后界面刷新
//    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_auth_rsp,this,
//            &ContactUserList::slot_auth_rsp);
}

void ContactUserList::ShowRedPoint(bool bshow /*= true*/)
{
    _add_friend_item->ShowRedPoint(bshow);
}

void ContactUserList::addContactUserList()
{
    auto * groupTip = new GroupTipItem();
    QListWidgetItem *item = new QListWidgetItem;
    item->setSizeHint(groupTip->sizeHint());
    this->addItem(item);
    this->setItemWidget(item, groupTip);
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);

    _add_friend_item = new ConUserItem();
    _add_friend_item->setObjectName("new_friend_item");
    _add_friend_item->SetInfo(0,tr("新的朋友"),":/res/add_friend.png");
    _add_friend_item->SetItemType(ListItemType::APPLY_FRIEND_ITEM);

    QListWidgetItem *add_item = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    add_item->setSizeHint(_add_friend_item->sizeHint());
    this->addItem(add_item);
    this->setItemWidget(add_item, _add_friend_item);

    //默认设置新的朋友申请条目被选中
    this->setCurrentItem(add_item);

    auto * groupCon = new GroupTipItem();
    groupCon->SetGroupTip(tr("联系人"));
    _groupitem = new QListWidgetItem;
    _groupitem->setSizeHint(groupCon->sizeHint());
    this->addItem(_groupitem);
    this->setItemWidget(_groupitem, groupCon);
    _groupitem->setFlags(_groupitem->flags() & ~Qt::ItemIsSelectable);


    // 创建QListWidgetItem，并设置自定义的widget
    for(int i = 0; i < 13; i++){
        int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
        int str_i = randomValue%strs.size();
        int head_i = randomValue%heads.size();
        int name_i = randomValue%names.size();

        auto *con_user_wid = new ConUserItem();
        con_user_wid->SetInfo(0,names[name_i], heads[head_i]);
        QListWidgetItem *item = new QListWidgetItem;
        //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
        item->setSizeHint(con_user_wid->sizeHint());
        this->addItem(item);
        this->setItemWidget(item, con_user_wid);
    }
}

bool ContactUserList::eventFilter(QObject *watched, QEvent *event)
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
            qDebug()<<"load more contact user";
            //发送信号通知聊天界面加载更多聊天内容
            emit sig_loading_contact_user();
         }

        return true; // 停止事件传递
    }

    return QListWidget::eventFilter(watched, event);

}

void ContactUserList::slot_item_clicked(QListWidgetItem *item)
{
    QWidget *widget = this->itemWidget(item); // 获取自定义widget对象
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
    if(itemType == ListItemType::INVALID_ITEM
            || itemType == ListItemType::GROUP_TIP_ITEM){
        qDebug()<< "slot invalid item clicked ";
        return;
    }

   if(itemType == ListItemType::APPLY_FRIEND_ITEM){

       // 创建对话框，提示用户
       qDebug()<< "apply friend item clicked ";
       //跳转到好友申请界面
       emit sig_switch_apply_friend_page();
       return;
   }

   if(itemType == ListItemType::CONTACT_USER_ITEM){
       // 创建对话框，提示用户
       qDebug()<< "contact user item clicked ";
       //跳转到好友申请界面
       emit sig_switch_friend_info_page();
       return;
   }
}

```
构造函数中关闭了滚动条的显示，重写了事件过滤器，实现了根据鼠标区域判断是否显示滚动条的功能。

并且实现了点击其中某个item响应对应的功能。并根据不同的item类型跳转不同的页面。

## 联系人item

因为每一个item都是我们自己定义的，所以我们添加设计师界面类，界面布局如下所示

![https://cdn.llfc.club/1721544014771.jpg](https://cdn.llfc.club/1721544014771.jpg)

类的声明如下
``` cpp
class ConUserItem : public ListItemBase
{
    Q_OBJECT

public:
    explicit ConUserItem(QWidget *parent = nullptr);
    ~ConUserItem();
    QSize sizeHint() const override;
    void SetInfo(std::shared_ptr<AuthInfo> auth_info);
    void SetInfo(std::shared_ptr<AuthRsp> auth_rsp);
    void SetInfo(int uid, QString name, QString icon);
    void ShowRedPoint(bool show = false);
private:
    Ui::ConUserItem *ui;
    std::shared_ptr<UserInfo> _info;
};
```
具体实现
``` cpp
ConUserItem::ConUserItem(QWidget *parent) :
    ListItemBase(parent),
    ui(new Ui::ConUserItem)
{
    ui->setupUi(this);
    SetItemType(ListItemType::CONTACT_USER_ITEM);
    ui->red_point->raise();
    ShowRedPoint(true);
}

ConUserItem::~ConUserItem()
{
    delete ui;
}

QSize ConUserItem::sizeHint() const
{
    return QSize(250, 70); // 返回自定义的尺寸
}

void ConUserItem::SetInfo(std::shared_ptr<AuthInfo> auth_info)
{
    _info = std::make_shared<UserInfo>(auth_info);
    // 加载图片
    QPixmap pixmap(_info->_icon);

    // 设置图片自动缩放
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);

    ui->user_name_lb->setText(_info->_name);
}

void ConUserItem::SetInfo(int uid, QString name, QString icon)
{
     _info = std::make_shared<UserInfo>(uid,name, icon);

     // 加载图片
     QPixmap pixmap(_info->_icon);

     // 设置图片自动缩放
     ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
     ui->icon_lb->setScaledContents(true);

     ui->user_name_lb->setText(_info->_name);
}

void ConUserItem::SetInfo(std::shared_ptr<AuthRsp> auth_rsp){
    _info = std::make_shared<UserInfo>(auth_rsp);

    // 加载图片
    QPixmap pixmap(_info->_icon);

    // 设置图片自动缩放
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);

    ui->user_name_lb->setText(_info->_name);
}

void ConUserItem::ShowRedPoint(bool show)
{
    if(show){
        ui->red_point->show();
    }else{
        ui->red_point->hide();
    }

}
```
这样我们启动程序就能看到模拟的联系人列表被加载进来了。

## 申请列表

申请页面ui布局如下

![https://cdn.llfc.club/1721545292540.jpg](https://cdn.llfc.club/1721545292540.jpg)

我们新增ApplyFriendPage类，用来显示申请列表
``` cpp
class ApplyFriendPage : public QWidget
{
    Q_OBJECT

public:
    explicit ApplyFriendPage(QWidget *parent = nullptr);
    ~ApplyFriendPage();
    void AddNewApply(std::shared_ptr<AddFriendApply> apply);
protected:
    void paintEvent(QPaintEvent *event);
private:
    void loadApplyList();
    Ui::ApplyFriendPage *ui;
    std::unordered_map<int, ApplyFriendItem*> _unauth_items;
public slots:
    void slot_auth_rsp(std::shared_ptr<AuthRsp> );
signals:
    void sig_show_search(bool);
};
```
具体实现
``` cpp
ApplyFriendPage::ApplyFriendPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ApplyFriendPage)
{
    ui->setupUi(this);
    connect(ui->apply_friend_list, &ApplyFriendList::sig_show_search, this, &ApplyFriendPage::sig_show_search);
    loadApplyList();
    //接受tcp传递的authrsp信号处理
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_auth_rsp, this, &ApplyFriendPage::slot_auth_rsp);
}

ApplyFriendPage::~ApplyFriendPage()
{
    delete ui;
}

void ApplyFriendPage::AddNewApply(std::shared_ptr<AddFriendApply> apply)
{
    //先模拟头像随机，以后头像资源增加资源服务器后再显示
    int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
    int head_i = randomValue % heads.size();
    auto* apply_item = new ApplyFriendItem();
    auto apply_info = std::make_shared<ApplyInfo>(apply->_from_uid,
             apply->_name, apply->_desc,heads[head_i], apply->_name, 0, 0);
    apply_item->SetInfo( apply_info);
    QListWidgetItem* item = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item->setSizeHint(apply_item->sizeHint());
    item->setFlags(item->flags() & ~Qt::ItemIsEnabled & ~Qt::ItemIsSelectable);
    ui->apply_friend_list->insertItem(0,item);
    ui->apply_friend_list->setItemWidget(item, apply_item);
    apply_item->ShowAddBtn(true);
    //收到审核好友信号
    connect(apply_item, &ApplyFriendItem::sig_auth_friend, [this](std::shared_ptr<ApplyInfo> apply_info) {
//        auto* authFriend = new AuthenFriend(this);
//        authFriend->setModal(true);
//        authFriend->SetApplyInfo(apply_info);
//        authFriend->show();
        });
}

void ApplyFriendPage::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void ApplyFriendPage::loadApplyList()
{
    //添加好友申请
    auto apply_list = UserMgr::GetInstance()->GetApplyList();
    for(auto &apply: apply_list){
        int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
        int head_i = randomValue % heads.size();
        auto* apply_item = new ApplyFriendItem();
        apply->SetIcon(heads[head_i]);
        apply_item->SetInfo(apply);
        QListWidgetItem* item = new QListWidgetItem;
        //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
        item->setSizeHint(apply_item->sizeHint());
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled & ~Qt::ItemIsSelectable);
        ui->apply_friend_list->insertItem(0,item);
        ui->apply_friend_list->setItemWidget(item, apply_item);
        if(apply->_status){
            apply_item->ShowAddBtn(false);
        }else{
             apply_item->ShowAddBtn(true);
             auto uid = apply_item->GetUid();
             _unauth_items[uid] = apply_item;
        }

        //收到审核好友信号
        connect(apply_item, &ApplyFriendItem::sig_auth_friend, [this](std::shared_ptr<ApplyInfo> apply_info) {
//            auto* authFriend = new AuthenFriend(this);
//            authFriend->setModal(true);
//            authFriend->SetApplyInfo(apply_info);
//            authFriend->show();
            });
    }

    // 模拟假数据，创建QListWidgetItem，并设置自定义的widget
    for(int i = 0; i < 13; i++){
        int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
        int str_i = randomValue%strs.size();
        int head_i = randomValue%heads.size();
        int name_i = randomValue%names.size();

        auto *apply_item = new ApplyFriendItem();
        auto apply = std::make_shared<ApplyInfo>(0, names[name_i], strs[str_i],
                                    heads[head_i], names[name_i], 0, 1);
        apply_item->SetInfo(apply);
        QListWidgetItem *item = new QListWidgetItem;
        //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
        item->setSizeHint(apply_item->sizeHint());
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled & ~Qt::ItemIsSelectable);
        ui->apply_friend_list->addItem(item);
        ui->apply_friend_list->setItemWidget(item, apply_item);
        //收到审核好友信号
        connect(apply_item, &ApplyFriendItem::sig_auth_friend, [this](std::shared_ptr<ApplyInfo> apply_info){
//            auto *authFriend =  new AuthenFriend(this);
//            authFriend->setModal(true);
//            authFriend->SetApplyInfo(apply_info);
//            authFriend->show();
        });
    }
}

void ApplyFriendPage::slot_auth_rsp(std::shared_ptr<AuthRsp> auth_rsp)
{
    auto uid = auth_rsp->_uid;
    auto find_iter = _unauth_items.find(uid);
    if (find_iter == _unauth_items.end()) {
        return;
    }

    find_iter->second->ShowAddBtn(false);
}
```
因为每个item自定义，所以我们新增设计师界面类ApplyFriendItem

界面布局

![https://cdn.llfc.club/1721546273709.jpg](https://cdn.llfc.club/1721546273709.jpg)

类的声明如下：
``` cpp
class ApplyFriendItem : public ListItemBase
{
    Q_OBJECT

public:
    explicit ApplyFriendItem(QWidget *parent = nullptr);
    ~ApplyFriendItem();
    void SetInfo(std::shared_ptr<ApplyInfo> apply_info);
    void ShowAddBtn(bool bshow);
    QSize sizeHint() const override {
        return QSize(250, 80); // 返回自定义的尺寸
    }
    int GetUid();
private:
    Ui::ApplyFriendItem *ui;
    std::shared_ptr<ApplyInfo> _apply_info;
    bool _added;
signals:
    void sig_auth_friend(std::shared_ptr<ApplyInfo> apply_info);
};
```
以下为具体实现
``` cpp
ApplyFriendItem::ApplyFriendItem(QWidget *parent) :
    ListItemBase(parent), _added(false),
    ui(new Ui::ApplyFriendItem)
{
    ui->setupUi(this);
    SetItemType(ListItemType::APPLY_FRIEND_ITEM);
    ui->addBtn->SetState("normal","hover", "press");
    ui->addBtn->hide();
    connect(ui->addBtn, &ClickedBtn::clicked,  [this](){
        emit this->sig_auth_friend(_apply_info);
    });
}

ApplyFriendItem::~ApplyFriendItem()
{
    delete ui;
}

void ApplyFriendItem::SetInfo(std::shared_ptr<ApplyInfo> apply_info)
{
    _apply_info = apply_info;
    // 加载图片
    QPixmap pixmap(_apply_info->_icon);

    // 设置图片自动缩放
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);

    ui->user_name_lb->setText(_apply_info->_name);
    ui->user_chat_lb->setText(_apply_info->_desc);
}

void ApplyFriendItem::ShowAddBtn(bool bshow)
{
    if (bshow) {
        ui->addBtn->show();
        ui->already_add_lb->hide();
        _added = false;
    }
    else {
        ui->addBtn->hide();
        ui->already_add_lb->show();
        _added = true;
    }
}

int ApplyFriendItem::GetUid() {
    return _apply_info->_uid;
}
```
申请列表类ApplyFriendList的声明如下
``` cpp
class ApplyFriendList: public QListWidget
{
    Q_OBJECT
public:
    ApplyFriendList(QWidget *parent = nullptr);
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:

signals:
    void sig_show_search(bool);
};
```
具体实现
``` cpp
ApplyFriendList::ApplyFriendList(QWidget *parent)
{
    Q_UNUSED(parent);
     this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
     this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 安装事件过滤器
    this->viewport()->installEventFilter(this);
}

bool ApplyFriendList::eventFilter(QObject *watched, QEvent *event)
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

    if (watched == this->viewport()) {
        if (event->type() == QEvent::MouseButtonPress) {
            emit sig_show_search(false);
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
```
然后在ChatDialog的stackedWidget中将friend_apply_page升级为ApplyFriendPage.

这样我们启动程序就能看到联系人列表和申请列表了。

下一步还需要写QSS美化以下
``` css
#con_user_list {
    background-color: rgb(247,247,248);
    border: none;
}

#con_user_list::item:selected {
    background-color: #d3d7d4;
    border: none;
    outline: none;
}

#con_user_list::item:hover {
    background-color: rgb(206,207,208);
    border: none;
    outline: none;
}

#con_user_list::focus {
    border: none;
    outline: none;
}

#GroupTipItem {
    background-color: #eaeaea;
    border: none;
}

#GroupTipItem QLabel{
    color: #2e2f30;
    font-size: 12px; /* 设置字体大小 */
    font-family: "Microsoft YaHei"; /* 设置字体 */
    border: none;
}

#new_friend_item {
     border-bottom: 1px solid #eaeaea;
}

#LineItem {
    background-color:rgb(247,247,247);
    border: none;
}

#friend_apply_lb {
    font-family: "Microsoft YaHei";
    font-size: 18px;
    font-weight: normal;
}

#friend_apply_wid {
    background-color: #f1f2f3;
    border-bottom: 1px solid #ede9e7;
}

#apply_friend_list {
    background-color: #f1f2f3;
    border-left: 1px solid #ede9e7;
    border-top: none;
    border-right: none;
    border-bottom: none;
}

ApplyFriendItem {
    background-color: #f1f2f3;
    border-bottom: 2px solid #dbd9d9;
}

ApplyFriendItem #user_chat_lb{
    color: #a2a2a2;
    font-size: 14px; /* 设置字体大小 */
    font-family: "Microsoft YaHei"; /* 设置字体 */
}

ApplyFriendItem #addBtn[state='normal'] {
    background-color: #d3d7d4;
    color: #2cb46e;
    font-size: 16px; /* 设置字体大小 */
    font-family: "Microsoft YaHei"; /* 设置字体 */
    border-radius: 20px; /* 设置圆角 */
}

ApplyFriendItem #addBtn[state='hover'] {
    background-color: #D3D3D3;
    color: #2cb46e;
    font-size: 16px; /* 设置字体大小 */
    font-family: "Microsoft YaHei"; /* 设置字体 */
    border-radius: 20px; /* 设置圆角 */
}

ApplyFriendItem #addBtn[state='press'] {
    background-color: #BEBEBE;
    color: #2cb46e;
    font-size: 16px; /* 设置字体大小 */
    font-family: "Microsoft YaHei"; /* 设置字体 */
    border-radius: 20px; /* 设置圆角 */
}

#already_add_lb{
    color:rgb(153,153,153);
    font-size: 12px;
    font-family: "Microsoft YaHei";
}

#user_name_lb{
    color:rgb(0,0,0);
    font-size: 16px;
    font-weight: normal;
    font-family: "Microsoft YaHei";
}
```

## 源码连接
[https://gitee.com/secondtonone1/llfcchat](https://gitee.com/secondtonone1/llfcchat) 

## 视频连接
[https://www.bilibili.com/video/BV1SS42197Yo/?vd_source=8be9e83424c2ed2c9b2a3ed1d01385e9](https://www.bilibili.com/video/BV1SS42197Yo/?vd_source=8be9e83424c2ed2c9b2a3ed1d01385e9)