## 聊天界面整体展示
我们先看下整体的聊天界面，方便以后逐个功能添加
![https://cdn.llfc.club/1716523002662.jpg](https://cdn.llfc.club/1716523002662.jpg)

QT Designer中 ui 设计如下
![https://cdn.llfc.club/1716528347175.jpg](https://cdn.llfc.club/1716528347175.jpg)

将聊天对话框分为几个部分：

1 处为左侧功能切换区域，包括聊天，联系人等。

2 处为搜索区域，可以搜索联系人，聊天记录等， 搜索框后面的按钮是快速拉人创建群聊的功能。

3 处为近期聊天联系列表

4 处为搜索结果列表，包括匹配到的联系人，聊天记录，以及添加联系人的按钮。

5  处为聊天对象名称和头像的显示区域，这里先做简单演示写死。

6  处为聊天记录区域。

7  处为工具栏区域。  

8  处为输入区域

9  处为发送按钮区域。

大家可以按照这个布局拖动和修改，达到我的布局效果。

## 创建ChatDialog

右键项目，选择创建设计师界面类，选择创建QDialog without buttons。对话框的名字为ChatDialog

创建完成后，在之前登录成功的回调里，跳转到这个对话框。在MainWindow里添加槽函数

``` cpp
void MainWindow::SlotSwitchChat()
{
    _chat_dlg = new ChatDialog();
    _chat_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    setCentralWidget(_chat_dlg);
    _chat_dlg->show();
    _login_dlg->hide();
    this->setMinimumSize(QSize(1050,900));
    this->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
}
```
在MainWindow的构造函数中添加信号和槽的链接
``` cpp
    //连接创建聊天界面信号
    connect(TcpMgr::GetInstance().get(),&TcpMgr::sig_swich_chatdlg, this, &MainWindow::SlotSwitchChat);

```
并且在TcpMgr中添加信号
``` cpp
void sig_swich_chatdlg();
```
为了方便测试，我们在MainWindow的构造函数中直接发送sig_switch_chatdlg信号，这样程序运行起来就会直接跳转到聊天界面

## ChatDialog ui文件
ui文件的结构可以参考我的源码中的结构，这里我们可以简单看下

![https://cdn.llfc.club/1716603750779.jpg](https://cdn.llfc.club/1716603750779.jpg)

按照这个布局拖动并设置宽高即可，接下来需要设置下qss调整颜色

``` css
#side_bar{
  background-color:rgb(46,46,46);
}
```
## 重写点击按钮
为了实现点击效果，我们继承QPushButton实现按钮的点击效果，包括普通状态，悬浮状态，以及按下状态
``` cpp
class ClickedBtn:public QPushButton
{
    Q_OBJECT
public:
    ClickedBtn(QWidget * parent = nullptr);
    ~ClickedBtn();
    void SetState(QString nomal, QString hover, QString press);
protected:
    virtual void enterEvent(QEvent *event) override; // 鼠标进入
     virtual void leaveEvent(QEvent *event) override;// 鼠标离开
    virtual void mousePressEvent(QMouseEvent *event) override; // 鼠标按下
    virtual void mouseReleaseEvent(QMouseEvent *event) override; // 鼠标释放
private:
    QString _normal;
    QString _hover;
    QString _press;
};
```
接下来实现其按下，离开进入等资源加载，并且重写这些事件
``` cpp
ClickedBtn::ClickedBtn(QWidget *parent):QPushButton (parent)
{
      setCursor(Qt::PointingHandCursor); // 设置光标为小手
}

ClickedBtn::~ClickedBtn(){

}

void ClickedBtn::SetState(QString normal, QString hover, QString press)
{
    _hover = hover;
    _normal = normal;
    _press = press;
    setProperty("state",normal);
    repolish(this);
    update();
}

void ClickedBtn::enterEvent(QEvent *event)
{
    setProperty("state",_hover);
    repolish(this);
    update();
    QPushButton::enterEvent(event);
}

void ClickedBtn::mousePressEvent(QMouseEvent *event)
{
    setProperty("state",_press);
    repolish(this);
    update();
    QPushButton::mousePressEvent(event);
}

void ClickedBtn::mouseReleaseEvent(QMouseEvent *event)
{
    setProperty("state",_hover);
    repolish(this);
    update();
    QPushButton::mouseReleaseEvent(event);
}
```
回到chatdialog.ui文件，将add_btn升级为ClickedBtn

接着在qss文件中添加样式

``` css
#add_btn[state='normal']{
    border-image: url(:/res/add_friend_normal.png);
}

#add_btn[state='hover']{
    border-image: url(:/res/add_friend_hover.png);

}

#add_btn[state='press']{
    border-image: url(:/res/add_friend_hover.png);
}
```
add_btn的样式一定要显示设置一下三个样式，所以我们回到ChatDialog的构造函数中设置样式
``` cpp
 ui->add_btn->SetState("normal","hover","press");
```
再次启动运行，可以看到添加群组的按钮样式和sidebar的样式生效了。

为了美观显示，我们去mainwindow.ui中移除状态栏和菜单栏。



