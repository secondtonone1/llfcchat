#ifndef CUSTOMIZETEXTEDIT_H
#define CUSTOMIZETEXTEDIT_H
#include <QTextEdit>

class CustomizeTextEdit:public QTextEdit
{
    Q_OBJECT
public:
    CustomizeTextEdit(QWidget *parent = nullptr);

protected:
    void focusOutEvent(QFocusEvent *event) override
    {
        // 执行失去焦点时的处理逻辑
        //qDebug() << "CustomizeEdit focusout";
        // 调用基类的focusOutEvent()方法，保证基类的行为得到执行
        QTextEdit::focusOutEvent(event);
        //发送失去焦点得信号
        emit sig_foucus_out();
    }

signals:
    void sig_foucus_out();
};

#endif // CUSTOMIZETEXTEDIT_H
