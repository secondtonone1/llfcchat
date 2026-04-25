## 增加定时按钮
点击获取验证码后需要让按钮显示倒计时，然后倒计时结束后再次可点击。 
添加TimberBtn类
``` cpp
#ifndef TIMERBTN_H
#define TIMERBTN_H
#include <QPushButton>
#include <QTimer>

class TimerBtn : public QPushButton
{
public:
    TimerBtn(QWidget *parent = nullptr);
    ~ TimerBtn();

    // 重写mouseReleaseEvent
    virtual void mouseReleaseEvent(QMouseEvent *e) override;
private:
    QTimer  *_timer;
    int _counter;
};

#endif // TIMERBTN_H
```
添加实现
``` cpp
#include "timerbtn.h"
#include <QMouseEvent>
#include <QDebug>

TimerBtn::TimerBtn(QWidget *parent):QPushButton(parent),_counter(10)
{
    _timer = new QTimer(this);

    connect(_timer, &QTimer::timeout, [this](){
        _counter--;
        if(_counter <= 0){
            _timer->stop();
            _counter = 10;
            this->setText("获取");
            this->setEnabled(true);
            return;
        }
        this->setText(QString::number(_counter));
    });
}

TimerBtn::~TimerBtn()
{
    _timer->stop();
}

void TimerBtn::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        // 在这里处理鼠标左键释放事件
        qDebug() << "MyButton was released!";
        this->setEnabled(false);
         this->setText(QString::number(_counter));
        _timer->start(1000);
        emit clicked();
    }
    // 调用基类的mouseReleaseEvent以确保正常的事件处理（如点击效果）
    QPushButton::mouseReleaseEvent(e);
}
```
然后将注册界面获取按钮升级为TimerBtn

## 调整输入框错误提示

在RegisterDialog构造函数中删除原来的输入框editing信号和逻辑，添加editingFinished信号和处理逻辑。
``` cpp
//day11 设定输入框输入后清空字符串
ui->err_tip->clear();

connect(ui->user_edit,&QLineEdit::editingFinished,this,[this](){
    checkUserValid();
});

connect(ui->email_edit, &QLineEdit::editingFinished, this, [this](){
    checkEmailValid();
});

connect(ui->pass_edit, &QLineEdit::editingFinished, this, [this](){
    checkPassValid();
});

connect(ui->confirm_edit, &QLineEdit::editingFinished, this, [this](){
    checkConfirmValid();
});

connect(ui->varify_edit, &QLineEdit::editingFinished, this, [this](){
        checkVarifyValid();
});
```
global.h中添加TipErr定义
``` cpp
enum TipErr{
    TIP_SUCCESS = 0,
    TIP_EMAIL_ERR = 1,
    TIP_PWD_ERR = 2,
    TIP_CONFIRM_ERR = 3,
    TIP_PWD_CONFIRM = 4,
    TIP_VARIFY_ERR = 5,
    TIP_USER_ERR = 6
};
```
RegisterDialog声明中添加
``` cpp
QMap<TipErr, QString> _tip_errs;
```
_tip_errs用来缓存各个输入框输入完成后提示的错误，如果该输入框错误清除后就显示剩余的错误，每次只显示一条

实现添加错误和删除错误
``` cpp
void ResetDialog::AddTipErr(TipErr te, QString tips)
{
    _tip_errs[te] = tips;
    showTip(tips, false);
}

void ResetDialog::DelTipErr(TipErr te)
{
    _tip_errs.remove(te);
    if(_tip_errs.empty()){
      ui->err_tip->clear();
      return;
    }

    showTip(_tip_errs.first(), false);
}
```
实现错误检测
``` cpp
bool ResetDialog::checkUserValid()
{
    if(ui->user_edit->text() == ""){
        AddTipErr(TipErr::TIP_USER_ERR, tr("用户名不能为空"));
        return false;
    }

    DelTipErr(TipErr::TIP_USER_ERR);
    return true;
}


bool ResetDialog::checkPassValid()
{
    auto pass = ui->pwd_edit->text();

    if(pass.length() < 6 || pass.length()>15){
        //提示长度不准确
        AddTipErr(TipErr::TIP_PWD_ERR, tr("密码长度应为6~15"));
        return false;
    }

    // 创建一个正则表达式对象，按照上述密码要求
    // 这个正则表达式解释：
    // ^[a-zA-Z0-9!@#$%^&*]{6,15}$ 密码长度至少6，可以是字母、数字和特定的特殊字符
    QRegularExpression regExp("^[a-zA-Z0-9!@#$%^&*]{6,15}$");
    bool match = regExp.match(pass).hasMatch();
    if(!match){
        //提示字符非法
        AddTipErr(TipErr::TIP_PWD_ERR, tr("不能包含非法字符"));
        return false;;
    }

    DelTipErr(TipErr::TIP_PWD_ERR);

    return true;
}



bool ResetDialog::checkEmailValid()
{
    //验证邮箱的地址正则表达式
    auto email = ui->email_edit->text();
    // 邮箱地址的正则表达式
    QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
    bool match = regex.match(email).hasMatch(); // 执行正则表达式匹配
    if(!match){
        //提示邮箱不正确
        AddTipErr(TipErr::TIP_EMAIL_ERR, tr("邮箱地址不正确"));
        return false;
    }

    DelTipErr(TipErr::TIP_EMAIL_ERR);
    return true;
}

bool ResetDialog::checkVarifyValid()
{
    auto pass = ui->varify_edit->text();
    if(pass.isEmpty()){
        AddTipErr(TipErr::TIP_VARIFY_ERR, tr("验证码不能为空"));
        return false;
    }

    DelTipErr(TipErr::TIP_VARIFY_ERR);
    return true;
}
```
除此之外修改之前点击确认按钮的逻辑，改为检测所有条件成立后再发送请求
``` cpp
void ResetDialog::on_sure_btn_clicked()
{
    bool valid = checkUserValid();
    if(!valid){
        return;
    }

    valid = checkEmailValid();
    if(!valid){
        return;
    }

    valid = checkPassValid();
    if(!valid){
        return;
    }

    valid = checkVarifyValid();
    if(!valid){
        return;
    }

    //发送http重置用户请求
    QJsonObject json_obj;
    json_obj["user"] = ui->user_edit->text();
    json_obj["email"] = ui->email_edit->text();
    json_obj["passwd"] = xorString(ui->pwd_edit->text());
    json_obj["varifycode"] = ui->varify_edit->text();
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix+"/reset_pwd"),
                 json_obj, ReqId::ID_RESET_PWD,Modules::RESETMOD);
}
```
## 隐藏和显示密码
我们在输入密码时希望能通过点击可见还是不可见，显示密码和隐藏密码，这里先添加图片放入资源中，然后在Register.ui中添加两个label，分别命名为pass_visible和confirm_visible, 用来占据位置。

因为我们要做的点击后图片要有状态切换，以及浮动显示不一样的效果等，所以我们重写ClickedLabel,继承自QLabel.
``` cpp
#ifndef CLICKEDLABEL_H
#define CLICKEDLABEL_H
#include <QLabel>
#include "global.h"

class ClickedLabel:public QLabel
{
    Q_OBJECT
public:
    ClickedLabel(QWidget* parent);
    virtual void mousePressEvent(QMouseEvent *ev) override;
    virtual void enterEvent(QEvent* event) override;
    virtual void leaveEvent(QEvent* event) override;
    void SetState(QString normal="", QString hover="", QString press="",
                  QString select="", QString select_hover="", QString select_press="");

    ClickLbState GetCurState();
protected:

private:
    QString _normal;
    QString _normal_hover;
    QString _normal_press;

    QString _selected;
    QString _selected_hover;
    QString _selected_press;

    ClickLbState _curstate;
signals:
    void clicked(void);

};

#endif // CLICKEDLABEL_H
```
一个Label有六种状态，普通状态，普通的悬浮状态，普通的点击状态，选中状态，选中的悬浮状态，选中的点击状态。

当Label处于普通状态，被点击后，切换为选中状态，再次点击又切换为普通状态。

ClickLbState定义在global.h中，包含两种状态一个是普通状态，一个是选中状态。而Label中的六种状态就是基于这两种状态嵌套实现的。
``` cpp
enum ClickLbState{
    Normal = 0,
    Selected = 1
};
```
六种状态用qss写好，这样我们只需要根据鼠标事件切换不同的qss就可以实现样式变换。
``` css
#pass_visible[state='unvisible']{
   border-image: url(:/res/unvisible.png);
}

#pass_visible[state='unvisible_hover']{
   border-image: url(:/res/unvisible_hover.png);
}

#pass_visible[state='visible']{
   border-image: url(:/res/visible.png);
}

#pass_visible[state='visible_hover']{
   border-image: url(:/res/visible_hover.png);
}


#confirm_visible[state='unvisible']{
   border-image: url(:/res/unvisible.png);
}

#confirm_visible[state='unvisible_hover']{
   border-image: url(:/res/unvisible_hover.png);
}

#confirm_visible[state='visible']{
   border-image: url(:/res/visible.png);
}

#confirm_visible[state='visible_hover']{
   border-image: url(:/res/visible_hover.png);
}
```
我们实现ClickedLabel功能
``` cpp
#include "clickedlabel.h"
#include <QMouseEvent>
ClickedLabel::ClickedLabel(QWidget* parent):QLabel (parent),_curstate(ClickLbState::Normal)
{

}


// 处理鼠标点击事件
void ClickedLabel::mousePressEvent(QMouseEvent* event)  {
    if (event->button() == Qt::LeftButton) {
        if(_curstate == ClickLbState::Normal){
              qDebug()<<"clicked , change to selected hover: "<< _selected_hover;
            _curstate = ClickLbState::Selected;
            setProperty("state",_selected_hover);
            repolish(this);
            update();

        }else{
               qDebug()<<"clicked , change to normal hover: "<< _normal_hover;
            _curstate = ClickLbState::Normal;
            setProperty("state",_normal_hover);
            repolish(this);
            update();
        }
        emit clicked();
    }
    // 调用基类的mousePressEvent以保证正常的事件处理
    QLabel::mousePressEvent(event);
}

// 处理鼠标悬停进入事件
void ClickedLabel::enterEvent(QEvent* event) {
    // 在这里处理鼠标悬停进入的逻辑
    if(_curstate == ClickLbState::Normal){
         qDebug()<<"enter , change to normal hover: "<< _normal_hover;
        setProperty("state",_normal_hover);
        repolish(this);
        update();

    }else{
         qDebug()<<"enter , change to selected hover: "<< _selected_hover;
        setProperty("state",_selected_hover);
        repolish(this);
        update();
    }

    QLabel::enterEvent(event);
}

// 处理鼠标悬停离开事件
void ClickedLabel::leaveEvent(QEvent* event){
    // 在这里处理鼠标悬停离开的逻辑
    if(_curstate == ClickLbState::Normal){
         qDebug()<<"leave , change to normal : "<< _normal;
        setProperty("state",_normal);
        repolish(this);
        update();

    }else{
         qDebug()<<"leave , change to normal hover: "<< _selected;
        setProperty("state",_selected);
        repolish(this);
        update();
    }
    QLabel::leaveEvent(event);
}

void ClickedLabel::SetState(QString normal, QString hover, QString press,
                            QString select, QString select_hover, QString select_press)
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

ClickLbState ClickedLabel::GetCurState(){
    return _curstate;
}
```
将label升级为ClickedLabel，然后在RegisterDialog的构造函数中添加label点击的响应函数
``` cpp

//设置浮动显示手形状
ui->pass_visible->setCursor(Qt::PointingHandCursor);
ui->confirm_visible->setCursor(Qt::PointingHandCursor);

ui->pass_visible->SetState("unvisible","unvisible_hover","","visible",
                            "visible_hover","");

ui->confirm_visible->SetState("unvisible","unvisible_hover","","visible",
                                "visible_hover","");
//连接点击事件

connect(ui->pass_visible, &ClickedLabel::clicked, this, [this]() {
    auto state = ui->pass_visible->GetCurState();
    if(state == ClickLbState::Normal){
        ui->pass_edit->setEchoMode(QLineEdit::Password);
    }else{
            ui->pass_edit->setEchoMode(QLineEdit::Normal);
    }
    qDebug() << "Label was clicked!";
});

connect(ui->confirm_visible, &ClickedLabel::clicked, this, [this]() {
    auto state = ui->confirm_visible->GetCurState();
    if(state == ClickLbState::Normal){
        ui->confirm_edit->setEchoMode(QLineEdit::Password);
    }else{
            ui->confirm_edit->setEchoMode(QLineEdit::Normal);
    }
    qDebug() << "Label was clicked!";
});
```
这样就实现了通过点击切换密码的显示和隐藏。

## 注册成功提示页面
注册成功后要切换到提示页面，所以在initHandlers函数内实现收到服务器注册回复的请求
``` cpp
//注册注册用户回包逻辑
_handlers.insert(ReqId::ID_REG_USER, [this](QJsonObject jsonObj){
    int error = jsonObj["error"].toInt();
    if(error != ErrorCodes::SUCCESS){
        showTip(tr("参数错误"),false);
        return;
    }
    auto email = jsonObj["email"].toString();
    showTip(tr("用户注册成功"), true);
    qDebug()<< "email is " << email ;
    qDebug()<< "user uuid is " <<  jsonObj["uuid"].toString();
    ChangeTipPage();
});
```
页面切换逻辑
``` cpp
void RegisterDialog::ChangeTipPage()
{
    _countdown_timer->stop();
    ui->stackedWidget->setCurrentWidget(ui->page_2);

    // 启动定时器，设置间隔为1000毫秒（1秒）
    _countdown_timer->start(1000);
}
```
在RegisterDialog.ui中stackwidget的page2添加标签和返回按钮

![https://cdn.llfc.club/1712821604946.jpg](https://cdn.llfc.club/1712821604946.jpg)

在RegisterDialog构造函数中添加定时器回调
``` cpp
// 创建定时器
_countdown_timer = new QTimer(this);
// 连接信号和槽
connect(_countdown_timer, &QTimer::timeout, [this](){
    if(_countdown==0){
        _countdown_timer->stop();
        emit sigSwitchLogin();
        return;
    }
    _countdown--;
    auto str = QString("注册成功，%1 s后返回登录").arg(_countdown);
    ui->tip_lb->setText(str);
});
```
除此之外在返回按钮的槽函数中停止定时器并发送切换登录的信号
``` cpp
void RegisterDialog::on_return_btn_clicked()
{
    _countdown_timer->stop();
    emit sigSwitchLogin();
}
```
取消注册也发送切换登录信号
``` cpp
void RegisterDialog::on_cancel_btn_clicked()
{
    _countdown_timer->stop();
    emit sigSwitchLogin();
}
```

## 界面跳转
回到mainwindow，构造函数简化，只做登录界面初始化
``` cpp
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //创建一个CentralWidget, 并将其设置为MainWindow的中心部件
    _login_dlg = new LoginDialog(this);
    _login_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    setCentralWidget(_login_dlg);

    //连接登录界面注册信号
    connect(_login_dlg, &LoginDialog::switchRegister, this, &MainWindow::SlotSwitchReg);
    //连接登录界面忘记密码信号
    connect(_login_dlg, &LoginDialog::switchReset, this, &MainWindow::SlotSwitchReset);
}
```
在点击注册按钮的槽函数中
``` cpp
void MainWindow::SlotSwitchReg()
{
    _reg_dlg = new RegisterDialog(this);
    _reg_dlg->hide();

    _reg_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);

     //连接注册界面返回登录信号
    connect(_reg_dlg, &RegisterDialog::sigSwitchLogin, this, &MainWindow::SlotSwitchLogin);
    setCentralWidget(_reg_dlg);
    _login_dlg->hide();
    _reg_dlg->show();
}
```


切换登录界面
``` cpp
//从注册界面返回登录界面
void MainWindow::SlotSwitchLogin()
{
    //创建一个CentralWidget, 并将其设置为MainWindow的中心部件
    _login_dlg = new LoginDialog(this);
    _login_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    setCentralWidget(_login_dlg);

   _reg_dlg->hide();
    _login_dlg->show();
    //连接登录界面注册信号
    connect(_login_dlg, &LoginDialog::switchRegister, this, &MainWindow::SlotSwitchReg);
    //连接登录界面忘记密码信号
    connect(_login_dlg, &LoginDialog::switchReset, this, &MainWindow::SlotSwitchReset);
}
```

这样登录界面和注册界面的切换逻辑就写完了。