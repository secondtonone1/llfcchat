## 简介
本文介绍如何实现好友申请界面, 其效果如下图所示

![https://cdn.llfc.club/1720423649556.jpg](https://cdn.llfc.club/1720423649556.jpg)

在此之前我们需要先定义一个ClickedOnceLabel类，支持点击一次的label功能。

接着新增一个ClickedOnceLabel类
``` cpp
class ClickedOnceLabel : public QLabel
{
    Q_OBJECT
public:
    ClickedOnceLabel(QWidget *parent=nullptr);
    virtual void mouseReleaseEvent(QMouseEvent *ev) override;
signals:
    void clicked(QString);
};
```
实现
``` cpp
ClickedOnceLabel::ClickedOnceLabel(QWidget *parent):QLabel(parent)
{
    setCursor(Qt::PointingHandCursor);
}


void ClickedOnceLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit clicked(this->text());
        return;
    }
    // 调用基类的mousePressEvent以保证正常的事件处理
    QLabel::mousePressEvent(event);
}
```
## 完善ClickedLabel
我们之前实现了ClickedLabel类，接下来修改下clicked信号，使其携带参数
``` cpp
 void clicked(QString, ClickLbState);
```
然后在其实现的鼠标释放事件的逻辑中添加
``` cpp
void ClickedLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if(_curstate == ClickLbState::Normal){
             // qDebug()<<"ReleaseEvent , change to normal hover: "<< _normal_hover;
            setProperty("state",_normal_hover);
            repolish(this);
            update();

        }else{
             //  qDebug()<<"ReleaseEvent , change to select hover: "<< _selected_hover;
            setProperty("state",_selected_hover);
            repolish(this);
            update();
        }
        emit clicked(this->text(), _curstate);
        return;
    }
    // 调用基类的mousePressEvent以保证正常的事件处理
    QLabel::mousePressEvent(event);
}
```
## 好友申请
好友申请界面和逻辑，我们可以创建一个设计师界面类叫做ApplyFriend类，我们在类的声明中添加如下成员。

``` cpp
class ApplyFriend : public QDialog
{
    Q_OBJECT

public:
    explicit ApplyFriend(QWidget *parent = nullptr);
    ~ApplyFriend();
    void InitTipLbs();
    void AddTipLbs(ClickedLabel*, QPoint cur_point, QPoint &next_point, int text_width, int text_height);
    bool eventFilter(QObject *obj, QEvent *event);
    void SetSearchInfo(std::shared_ptr<SearchInfo> si);

private:
    Ui::ApplyFriend *ui;
    void resetLabels();

    //已经创建好的标签
    QMap<QString, ClickedLabel*> _add_labels;
    std::vector<QString> _add_label_keys;
    QPoint _label_point;
    //用来在输入框显示添加新好友的标签
    QMap<QString, FriendLabel*> _friend_labels;
    std::vector<QString> _friend_label_keys;
    void addLabel(QString name);
    std::vector<QString> _tip_data;
    QPoint _tip_cur_point;
    std::shared_ptr<SearchInfo> _si;
public slots:
    //显示更多label标签
    void ShowMoreLabel();
    //输入label按下回车触发将标签加入展示栏
    void SlotLabelEnter();
    //点击关闭，移除展示栏好友便签
    void SlotRemoveFriendLabel(QString);
    //通过点击tip实现增加和减少好友便签
    void SlotChangeFriendLabelByTip(QString, ClickLbState);
    //输入框文本变化显示不同提示
    void SlotLabelTextChange(const QString& text);
    //输入框输入完成
    void SlotLabelEditFinished();
   //输入标签显示提示框，点击提示框内容后添加好友便签
    void SlotAddFirendLabelByClickTip(QString text);
    //处理确认回调
    void SlotApplySure();
    //处理取消回调
    void SlotApplyCancel();
};
```
接下来我们修改ui使其变成如下布局

![https://cdn.llfc.club/1720423721312.jpg](https://cdn.llfc.club/1720423721312.jpg)

然后我们逐个实现功能,构造函数分别实现信号的链接和成员初始化，析构函数回收必要的资源。
``` cpp
ApplyFriend::ApplyFriend(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ApplyFriend),_label_point(2,6)
{
    ui->setupUi(this);
    // 隐藏对话框标题栏
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    this->setObjectName("ApplyFriend");
    this->setModal(true);
    ui->name_ed->setPlaceholderText(tr("恋恋风辰"));
    ui->lb_ed->setPlaceholderText("搜索、添加标签");
    ui->back_ed->setPlaceholderText("燃烧的胸毛");

    ui->lb_ed->SetMaxLength(21);
    ui->lb_ed->move(2, 2);
    ui->lb_ed->setFixedHeight(20);
    ui->lb_ed->setMaxLength(10);
    ui->input_tip_wid->hide();

    _tip_cur_point = QPoint(5, 5);

    _tip_data = { "同学","家人","菜鸟教程","C++ Primer","Rust 程序设计",
                             "父与子学Python","nodejs开发指南","go 语言开发指南",
                                "游戏伙伴","金融投资","微信读书","拼多多拼友" };

    connect(ui->more_lb, &ClickedOnceLabel::clicked, this, &ApplyFriend::ShowMoreLabel);
    InitTipLbs();
    //链接输入标签回车事件
    connect(ui->lb_ed, &CustomizeEdit::returnPressed, this, &ApplyFriend::SlotLabelEnter);
    connect(ui->lb_ed, &CustomizeEdit::textChanged, this, &ApplyFriend::SlotLabelTextChange);
    connect(ui->lb_ed, &CustomizeEdit::editingFinished, this, &ApplyFriend::SlotLabelEditFinished);
    connect(ui->tip_lb, &ClickedOnceLabel::clicked, this, &ApplyFriend::SlotAddFirendLabelByClickTip);

    ui->scrollArea->horizontalScrollBar()->setHidden(true);
    ui->scrollArea->verticalScrollBar()->setHidden(true);
    ui->scrollArea->installEventFilter(this);
    ui->sure_btn->SetState("normal","hover","press");
    ui->cancel_btn->SetState("normal","hover","press");
    //连接确认和取消按钮的槽函数
    connect(ui->cancel_btn, &QPushButton::clicked, this, &ApplyFriend::SlotApplyCancel);
    connect(ui->sure_btn, &QPushButton::clicked, this, &ApplyFriend::SlotApplySure);
}

ApplyFriend::~ApplyFriend()
{
    qDebug()<< "ApplyFriend destruct";
    delete ui;
}
```
因为此时还未与服务器联调数据，此时我们写一个InitLabel的函数模拟创建多个标签展示
``` cpp
void ApplyFriend::InitTipLbs()
{
    int lines = 1;
    for(int i = 0; i < _tip_data.size(); i++){

        auto* lb = new ClickedLabel(ui->lb_list);
        lb->SetState("normal", "hover", "pressed", "selected_normal",
            "selected_hover", "selected_pressed");
        lb->setObjectName("tipslb");
        lb->setText(_tip_data[i]);
        connect(lb, &ClickedLabel::clicked, this, &ApplyFriend::SlotChangeFriendLabelByTip);

        QFontMetrics fontMetrics(lb->font()); // 获取QLabel控件的字体信息
        int textWidth = fontMetrics.width(lb->text()); // 获取文本的宽度
        int textHeight = fontMetrics.height(); // 获取文本的高度

        if (_tip_cur_point.x() + textWidth + tip_offset > ui->lb_list->width()) {
            lines++;
            if (lines > 2) {
                delete lb;
                return;
            }

            _tip_cur_point.setX(tip_offset);
            _tip_cur_point.setY(_tip_cur_point.y() + textHeight + 15);

        }

       auto next_point = _tip_cur_point;

       AddTipLbs(lb, _tip_cur_point,next_point, textWidth, textHeight);

       _tip_cur_point = next_point;
    }

}
```
下面这个函数是将标签添加到展示区
``` cpp
void ApplyFriend::AddTipLbs(ClickedLabel* lb, QPoint cur_point, QPoint& next_point, int text_width, int text_height)
{
    lb->move(cur_point);
    lb->show();
    _add_labels.insert(lb->text(), lb);
    _add_label_keys.push_back(lb->text());
    next_point.setX(lb->pos().x() + text_width + 15);
    next_point.setY(lb->pos().y());
}
```
重写事件过滤器展示滑动条
``` cpp
bool ApplyFriend::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->scrollArea && event->type() == QEvent::Enter)
    {
        ui->scrollArea->verticalScrollBar()->setHidden(false);
    }
    else if (obj == ui->scrollArea && event->type() == QEvent::Leave)
    {
        ui->scrollArea->verticalScrollBar()->setHidden(true);
    }
    return QObject::eventFilter(obj, event);
}
```
后期搜索用户功能用户数据会从服务器传回来，所以写了下面的接口
``` cpp
void ApplyFriend::SetSearchInfo(std::shared_ptr<SearchInfo> si)
{
    _si = si;
    auto applyname = UserMgr::GetInstance()->GetName();
    auto bakname = si->_name;
    ui->name_ed->setText(applyname);
    ui->back_ed->setText(bakname);
}
```
当点击按钮，可展示更多标签的功能。
``` cpp
void ApplyFriend::ShowMoreLabel()
{
    qDebug()<< "receive more label clicked";
    ui->more_lb_wid->hide();

    ui->lb_list->setFixedWidth(325);
    _tip_cur_point = QPoint(5, 5);
    auto next_point = _tip_cur_point;
    int textWidth;
    int textHeight;
    //重拍现有的label
    for(auto & added_key : _add_label_keys){
        auto added_lb = _add_labels[added_key];

        QFontMetrics fontMetrics(added_lb->font()); // 获取QLabel控件的字体信息
        textWidth = fontMetrics.width(added_lb->text()); // 获取文本的宽度
        textHeight = fontMetrics.height(); // 获取文本的高度

        if(_tip_cur_point.x() +textWidth + tip_offset > ui->lb_list->width()){
            _tip_cur_point.setX(tip_offset);
            _tip_cur_point.setY(_tip_cur_point.y()+textHeight+15);
        }
        added_lb->move(_tip_cur_point);

        next_point.setX(added_lb->pos().x() + textWidth + 15);
        next_point.setY(_tip_cur_point.y());

        _tip_cur_point = next_point;

    }

    //添加未添加的
    for(int i = 0; i < _tip_data.size(); i++){
        auto iter = _add_labels.find(_tip_data[i]);
        if(iter != _add_labels.end()){
            continue;
        }

        auto* lb = new ClickedLabel(ui->lb_list);
        lb->SetState("normal", "hover", "pressed", "selected_normal",
            "selected_hover", "selected_pressed");
        lb->setObjectName("tipslb");
        lb->setText(_tip_data[i]);
        connect(lb, &ClickedLabel::clicked, this, &ApplyFriend::SlotChangeFriendLabelByTip);

        QFontMetrics fontMetrics(lb->font()); // 获取QLabel控件的字体信息
        int textWidth = fontMetrics.width(lb->text()); // 获取文本的宽度
        int textHeight = fontMetrics.height(); // 获取文本的高度

        if (_tip_cur_point.x() + textWidth + tip_offset > ui->lb_list->width()) {

            _tip_cur_point.setX(tip_offset);
            _tip_cur_point.setY(_tip_cur_point.y() + textHeight + 15);

        }

         next_point = _tip_cur_point;

        AddTipLbs(lb, _tip_cur_point, next_point, textWidth, textHeight);

        _tip_cur_point = next_point;

    }

   int diff_height = next_point.y() + textHeight + tip_offset - ui->lb_list->height();
   ui->lb_list->setFixedHeight(next_point.y() + textHeight + tip_offset);

    //qDebug()<<"after resize ui->lb_list size is " <<  ui->lb_list->size();
    ui->scrollcontent->setFixedHeight(ui->scrollcontent->height()+diff_height);
}
```
重排好友标签编辑栏的标签
``` cpp
void ApplyFriend::resetLabels()
{
    auto max_width = ui->gridWidget->width();
    auto label_height = 0;
    for(auto iter = _friend_labels.begin(); iter != _friend_labels.end(); iter++){
        //todo... 添加宽度统计
        if( _label_point.x() + iter.value()->width() > max_width) {
            _label_point.setY(_label_point.y()+iter.value()->height()+6);
            _label_point.setX(2);
        }

        iter.value()->move(_label_point);
        iter.value()->show();

        _label_point.setX(_label_point.x()+iter.value()->width()+2);
        _label_point.setY(_label_point.y());
        label_height = iter.value()->height();
    }

    if(_friend_labels.isEmpty()){
         ui->lb_ed->move(_label_point);
         return;
    }

    if(_label_point.x() + MIN_APPLY_LABEL_ED_LEN > ui->gridWidget->width()){
        ui->lb_ed->move(2,_label_point.y()+label_height+6);
    }else{
         ui->lb_ed->move(_label_point);
    }
}
```
添加好友标签编辑栏的标签
``` cpp
void ApplyFriend::addLabel(QString name)
{
    if (_friend_labels.find(name) != _friend_labels.end()) {
        return;
    }

    auto tmplabel = new FriendLabel(ui->gridWidget);
    tmplabel->SetText(name);
    tmplabel->setObjectName("FriendLabel");

    auto max_width = ui->gridWidget->width();
    //todo... 添加宽度统计
    if (_label_point.x() + tmplabel->width() > max_width) {
        _label_point.setY(_label_point.y() + tmplabel->height() + 6);
        _label_point.setX(2);
    }
    else {

    }


    tmplabel->move(_label_point);
    tmplabel->show();
    _friend_labels[tmplabel->Text()] = tmplabel;
    _friend_label_keys.push_back(tmplabel->Text());

    connect(tmplabel, &FriendLabel::sig_close, this, &ApplyFriend::SlotRemoveFriendLabel);

    _label_point.setX(_label_point.x() + tmplabel->width() + 2);

    if (_label_point.x() + MIN_APPLY_LABEL_ED_LEN > ui->gridWidget->width()) {
        ui->lb_ed->move(2, _label_point.y() + tmplabel->height() + 2);
    }
    else {
        ui->lb_ed->move(_label_point);
    }

    ui->lb_ed->clear();

    if (ui->gridWidget->height() < _label_point.y() + tmplabel->height() + 2) {
        ui->gridWidget->setFixedHeight(_label_point.y() + tmplabel->height() * 2 + 2);
    }
}
```
点击回车后，在好友标签编辑栏添加标签，在标签展示栏添加标签
``` cpp
void ApplyFriend::SlotLabelEnter()
{
    if(ui->lb_ed->text().isEmpty()){
        return;
    }

    auto text = ui->lb_ed->text();
    addLabel(ui->lb_ed->text());

    ui->input_tip_wid->hide();
    auto find_it = std::find(_tip_data.begin(), _tip_data.end(), text);
    //找到了就只需设置状态为选中即可
    if (find_it == _tip_data.end()) {
        _tip_data.push_back(text);
    }

    //判断标签展示栏是否有该标签
    auto find_add = _add_labels.find(text);
    if (find_add != _add_labels.end()) {
        find_add.value()->SetCurState(ClickLbState::Selected);
        return;
    }

    //标签展示栏也增加一个标签, 并设置绿色选中
    auto* lb = new ClickedLabel(ui->lb_list);
    lb->SetState("normal", "hover", "pressed", "selected_normal",
        "selected_hover", "selected_pressed");
    lb->setObjectName("tipslb");
    lb->setText(text);
    connect(lb, &ClickedLabel::clicked, this, &ApplyFriend::SlotChangeFriendLabelByTip);
    qDebug() << "ui->lb_list->width() is " << ui->lb_list->width();
    qDebug() << "_tip_cur_point.x() is " << _tip_cur_point.x();

    QFontMetrics fontMetrics(lb->font()); // 获取QLabel控件的字体信息
    int textWidth = fontMetrics.width(lb->text()); // 获取文本的宽度
    int textHeight = fontMetrics.height(); // 获取文本的高度
    qDebug() << "textWidth is " << textWidth;

    if (_tip_cur_point.x() + textWidth + tip_offset + 3 > ui->lb_list->width()) {

        _tip_cur_point.setX(5);
        _tip_cur_point.setY(_tip_cur_point.y() + textHeight + 15);

    }

    auto next_point = _tip_cur_point;

    AddTipLbs(lb, _tip_cur_point, next_point, textWidth, textHeight);
    _tip_cur_point = next_point;

    int diff_height = next_point.y() + textHeight + tip_offset - ui->lb_list->height();
    ui->lb_list->setFixedHeight(next_point.y() + textHeight + tip_offset);

    lb->SetCurState(ClickLbState::Selected);

    ui->scrollcontent->setFixedHeight(ui->scrollcontent->height() + diff_height);
}
```
当我们点击好友标签编辑栏的标签的关闭按钮时会调用下面的槽函数
``` cpp
void ApplyFriend::SlotRemoveFriendLabel(QString name)
{
    qDebug() << "receive close signal";

    _label_point.setX(2);
    _label_point.setY(6);

   auto find_iter = _friend_labels.find(name);

   if(find_iter == _friend_labels.end()){
       return;
   }

   auto find_key = _friend_label_keys.end();
   for(auto iter = _friend_label_keys.begin(); iter != _friend_label_keys.end();
       iter++){
       if(*iter == name){
           find_key = iter;
           break;
       }
   }

   if(find_key != _friend_label_keys.end()){
      _friend_label_keys.erase(find_key);
   }


   delete find_iter.value();

   _friend_labels.erase(find_iter);

   resetLabels();

   auto find_add = _add_labels.find(name);
   if(find_add == _add_labels.end()){
        return;
   }

   find_add.value()->ResetNormalState();
}
```
当我们点击标签展示栏的标签，可以实现标签添加和删除
``` cpp
//点击标已有签添加或删除新联系人的标签
void ApplyFriend::SlotChangeFriendLabelByTip(QString lbtext, ClickLbState state)
{
    auto find_iter = _add_labels.find(lbtext);
    if(find_iter == _add_labels.end()){
        return;
    }

    if(state == ClickLbState::Selected){
        //编写添加逻辑
        addLabel(lbtext);
        return;
    }

    if(state == ClickLbState::Normal){
        //编写删除逻辑
        SlotRemoveFriendLabel(lbtext);
        return;
    }

}
```
当标签文本变化时，下面提示框的文本跟随变化
``` cpp
void ApplyFriend::SlotLabelTextChange(const QString& text)
{
    if (text.isEmpty()) {
        ui->tip_lb->setText("");
        ui->input_tip_wid->hide();
        return;
    }

    auto iter = std::find(_tip_data.begin(), _tip_data.end(), text);
    if (iter == _tip_data.end()) {
        auto new_text = add_prefix + text;
        ui->tip_lb->setText(new_text);
        ui->input_tip_wid->show();
        return;
    }
    ui->tip_lb->setText(text);
    ui->input_tip_wid->show();
}
```
如果编辑完成，则隐藏编辑框
``` cpp
void ApplyFriend::SlotLabelEditFinished()
{
    ui->input_tip_wid->hide();
}
```
点击提示框，也会添加标签，功能如下
``` cpp
void ApplyFriend::SlotAddFirendLabelByClickTip(QString text)
{
    int index = text.indexOf(add_prefix);
    if (index != -1) {
        text = text.mid(index + add_prefix.length());
    }
    addLabel(text);

    auto find_it = std::find(_tip_data.begin(), _tip_data.end(), text);
    //找到了就只需设置状态为选中即可
    if (find_it == _tip_data.end()) {
        _tip_data.push_back(text);
    }
   
    //判断标签展示栏是否有该标签
    auto find_add = _add_labels.find(text);
    if (find_add != _add_labels.end()) {
        find_add.value()->SetCurState(ClickLbState::Selected);
        return;
    }
     
    //标签展示栏也增加一个标签, 并设置绿色选中
	auto* lb = new ClickedLabel(ui->lb_list);
	lb->SetState("normal", "hover", "pressed", "selected_normal",
		"selected_hover", "selected_pressed");
	lb->setObjectName("tipslb");
	lb->setText(text);
	connect(lb, &ClickedLabel::clicked, this, &ApplyFriend::SlotChangeFriendLabelByTip);
    qDebug() << "ui->lb_list->width() is " << ui->lb_list->width();
    qDebug() << "_tip_cur_point.x() is " << _tip_cur_point.x();
   
	QFontMetrics fontMetrics(lb->font()); // 获取QLabel控件的字体信息
	int textWidth = fontMetrics.width(lb->text()); // 获取文本的宽度
	int textHeight = fontMetrics.height(); // 获取文本的高度
    qDebug() << "textWidth is " << textWidth;

	if (_tip_cur_point.x() + textWidth+ tip_offset+3 > ui->lb_list->width()) {

		_tip_cur_point.setX(5);
		_tip_cur_point.setY(_tip_cur_point.y() + textHeight + 15);

	}

	auto next_point = _tip_cur_point;

	 AddTipLbs(lb, _tip_cur_point, next_point, textWidth,textHeight);
	_tip_cur_point = next_point;

    int diff_height = next_point.y() + textHeight + tip_offset - ui->lb_list->height();
    ui->lb_list->setFixedHeight(next_point.y() + textHeight + tip_offset);

    lb->SetCurState(ClickLbState::Selected);

    ui->scrollcontent->setFixedHeight(ui->scrollcontent->height()+ diff_height );
}
```
确认申请和取消申请只是打印了对应信息，并且回收界面
``` cpp
void ApplyFriend::SlotApplyCancel()
{
    qDebug() << "Slot Apply Cancel";
    this->hide();
    deleteLater();
}

void ApplyFriend::SlotApplySure()
{
    qDebug()<<"Slot Apply Sure called" ;
    this->hide();
    deleteLater();
}
```
## 美化界面

添加如下qss文件美化界面
``` qss
#ApplyFriend{
    border: 2px solid #f1f1f1;
    font-size: 14px;
    background: #f7f7f8;
}

#scrollArea{
    background: #f7f7f8;
    border: none;
}

#scrollcontent{
     background: #f7f7f8;
}

#scrollcontent #apply_lb{
    font-family: "Microsoft YaHei";
    font-size: 16px;
    font-weight: normal;
}

#apply_wid QLabel{
    color:rgb(140,140,140);
    font-size: 14px;
    font-family: "Microsoft YaHei";
    height: 25px;
}

#apply_wid #name_ed, #apply_wid #back_ed{
    border: 1px solid #f7f7f8;
    font-size: 14px;
    font-family: "Microsoft YaHei";
}

#apply_wid #lb_ed {
       border: none;
       font-size: 14px;
       font-family: "Microsoft YaHei";
}

#apply_wid #more_lb{
    border-image: url(:/res/arowdown.png);
}

#apply_wid #tipslb[state='normal'] {
    padding: 2px;
    background: #e1e1e1;
    color: #1e1e1e;
    border-radius: 10px;
}

#apply_wid #tipslb[state='hover'] {
    padding: 2px;
    background: #e1e1e1;
    color: #1e1e1e;
    border-radius: 10px;
}

#apply_wid #tipslb[state='pressed'] {
    padding: 2px;
    background: #e1e1e1;
    color: #48bf56;
    border-radius: 10px;
}

#apply_wid #tipslb[state='selected_normal'] {
    padding: 2px;
    background: #e1e1e1;
    color: #48bf56;
    border-radius: 10px;
}

#apply_wid #tipslb[state='selected_hover'] {
    padding: 2px;
    background: #e1e1e1;
    color: #48bf56;
    border-radius: 10px;
}

#apply_wid #tipslb[state='selected_pressed'] {
    padding: 2px;
    background: #e1e1e1;
    color: #1e1e1e;
    border-radius: 10px;
}

#input_tip_wid {
    background: #d3eaf8;
}

#apply_wid #FriendLabel {
    background: #daf6e7;
    color: #48bf56;
    border-radius: 10px;
}

#apply_wid #tip_lb {
    padding-left: 2px;
    color:rgb(153,153,153);
    font-size: 14px;
    font-family: "Microsoft YaHei";
}

#gridWidget {
   background: #fdfdfd;
}

#close_lb[state='normal'] {
    border-image: url(:/res/tipclose.png);
}

#close_lb[state='hover'] {
    border-image: url(:/res/tipclose.png);
}

#close_lb[state='pressed'] {
    border-image: url(:/res/tipclose.png);
}

#close_lb[state='select_normal'] {
    border-image: url(:/res/tipclose.png);
}

#close_lb[state='select_hover'] {
    border-image: url(:/res/tipclose.png);
}

#close_lb[state='select_pressed'] {
    border-image: url(:/res/tipclose.png);
}

#apply_sure_wid #sure_btn[state='normal']  {
    background: #f0f0f0;
    color: #2cb46e;
    font-size: 16px; /* 设置字体大小 */
    font-family: "Microsoft YaHei"; /* 设置字体 */
    border-radius: 20px; /* 设置圆角 */
}

#apply_sure_wid #sure_btn[state='hover'] {
    background: #d2d2d2;
    color: #2cb46e;
    font-size: 16px; /* 设置字体大小 */
    font-family: "Microsoft YaHei"; /* 设置字体 */
    border-radius: 20px; /* 设置圆角 */
}

#apply_sure_wid #sure_btn[state='press'] {
    background: #c6c6c6;
    color: #2cb46e;
    font-size: 16px; /* 设置字体大小 */
    font-family: "Microsoft YaHei"; /* 设置字体 */
    border-radius: 20px; /* 设置圆角 */
}

#apply_sure_wid #cancel_btn[state='normal']  {
    background: #f0f0f0;
    color: #2e2f30;
    font-size: 16px; /* 设置字体大小 */
    font-family: "Microsoft YaHei"; /* 设置字体 */
    border-radius: 20px; /* 设置圆角 */
}

#apply_sure_wid #cancel_btn[state='hover'] {
    background: #d2d2d2;
    color: #2e2f30;
    font-size: 16px; /* 设置字体大小 */
    font-family: "Microsoft YaHei"; /* 设置字体 */
    border-radius: 20px; /* 设置圆角 */
}

#apply_sure_wid #cancel_btn[state='press'] {
    background: #c6c6c6;
    color: #2e2f30;
    font-size: 16px; /* 设置字体大小 */
    font-family: "Microsoft YaHei"; /* 设置字体 */
    border-radius: 20px; /* 设置圆角 */
}
```

## 视频

[https://www.bilibili.com/video/BV1ZM4m127z8/?vd_source=8be9e83424c2ed2c9b2a3ed1d01385e9](https://www.bilibili.com/video/BV1ZM4m127z8/?vd_source=8be9e83424c2ed2c9b2a3ed1d01385e9)

## 源码链接
[https://gitee.com/secondtonone1/llfcchat](https://gitee.com/secondtonone1/llfcchat) 

