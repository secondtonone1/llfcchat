## 滚动聊天布局设计
我们的聊天布局如下图
最外层的是一个chatview（黑色）， chatview内部在添加一个MainLayout（蓝色），MainLayout内部添加一个scrollarea(红色)，scrollarea内部包含一个widget（绿色），同时也包含一个HLayout（紫色）用来浮动显示滚动条。widget内部包含一个垂直布局Vlayout(黄色),黄色布局内部包含一个粉色的widget，widget占据拉伸比一万，保证充满整个布局。

![https://cdn.llfc.club/layoutpic.png](https://cdn.llfc.club/layoutpic.png)

## 代码实现

我们对照上面的图手写代码，在项目中添加ChatView类，然后先实现类的声明
``` cpp
class ChatView: public QWidget
{
    Q_OBJECT
public:
    ChatView(QWidget *parent = Q_NULLPTR);
    void appendChatItem(QWidget *item);                 //尾插
    void prependChatItem(QWidget *item);                //头插
    void insertChatItem(QWidget *before, QWidget *item);//中间插
protected:
    bool eventFilter(QObject *o, QEvent *e) override;
    void paintEvent(QPaintEvent *event) override;
private slots:
    void onVScrollBarMoved(int min, int max);
private:
    void initStyleSheet();
private:
    //QWidget *m_pCenterWidget;
    QVBoxLayout *m_pVl;
    QScrollArea *m_pScrollArea;
    bool isAppended;
};
```
接下来实现其函数定义, 先实现构造函数
``` cpp
ChatView::ChatView(QWidget *parent)  : QWidget(parent)
  , isAppended(false)
{
    QVBoxLayout *pMainLayout = new QVBoxLayout();
    this->setLayout(pMainLayout);
    pMainLayout->setMargin(0);

    m_pScrollArea = new QScrollArea();
    m_pScrollArea->setObjectName("chat_area");
    pMainLayout->addWidget(m_pScrollArea);

    QWidget *w = new QWidget(this);
    w->setObjectName("chat_bg");
    w->setAutoFillBackground(true);

    QVBoxLayout *pVLayout_1 = new QVBoxLayout();
    pVLayout_1->addWidget(new QWidget(), 100000);
    w->setLayout(pVLayout_1);
    m_pScrollArea->setWidget(w);

    m_pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QScrollBar *pVScrollBar = m_pScrollArea->verticalScrollBar();
    connect(pVScrollBar, &QScrollBar::rangeChanged,this, &ChatView::onVScrollBarMoved);

    //把垂直ScrollBar放到上边 而不是原来的并排
    QHBoxLayout *pHLayout_2 = new QHBoxLayout();
    pHLayout_2->addWidget(pVScrollBar, 0, Qt::AlignRight);
    pHLayout_2->setMargin(0);
    m_pScrollArea->setLayout(pHLayout_2);
    pVScrollBar->setHidden(true);

    m_pScrollArea->setWidgetResizable(true);
    m_pScrollArea->installEventFilter(this);
    initStyleSheet();
}
```
再实现添加条目到聊天背景
``` cpp
void ChatView::appendChatItem(QWidget *item)
{
    QVBoxLayout *vl = qobject_cast<QVBoxLayout *>(m_pScrollArea->widget()->layout());
    vl->insertWidget(vl->count()-1, item);
    isAppended = true;
}
```
重写事件过滤器
``` cpp
bool ChatView::eventFilter(QObject *o, QEvent *e)
{
    /*if(e->type() == QEvent::Resize && o == )
    {

    }
    else */if(e->type() == QEvent::Enter && o == m_pScrollArea)
    {
        m_pScrollArea->verticalScrollBar()->setHidden(m_pScrollArea->verticalScrollBar()->maximum() == 0);
    }
    else if(e->type() == QEvent::Leave && o == m_pScrollArea)
    {
         m_pScrollArea->verticalScrollBar()->setHidden(true);
    }
    return QWidget::eventFilter(o, e);
}
```
重写paintEvent支持子类绘制
``` cpp
void ChatView::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
```
监听滚动区域变化的槽函数
``` cpp
void ChatView::onVScrollBarMoved(int min, int max)
{
    if(isAppended) //添加item可能调用多次
    {
        QScrollBar *pVScrollBar = m_pScrollArea->verticalScrollBar();
        pVScrollBar->setSliderPosition(pVScrollBar->maximum());
        //500毫秒内可能调用多次
        QTimer::singleShot(500, [this]()
        {
            isAppended = false;
        });
    }
}
```
本节先到这里，完成聊天布局基本的构造

## 视频链接
[https://www.bilibili.com/video/BV1xz421h7Ad/?vd_source=8be9e83424c2ed2c9b2a3ed1d01385e9](https://www.bilibili.com/video/BV1xz421h7Ad/?vd_source=8be9e83424c2ed2c9b2a3ed1d01385e9)

## 源码链接
[https://gitee.com/secondtonone1/llfcchat](https://gitee.com/secondtonone1/llfcchat)
