#ifndef INVALIDITEM_H
#define INVALIDITEM_H
#include <QWidget>

class InvalidItem : public QWidget
{
    Q_OBJECT
public:
    explicit InvalidItem(QWidget *parent = nullptr);
    QSize sizeHint() const override {
        return QSize(250, 20); // 返回自定义的尺寸
    }
signals:

public slots:
};

#endif // INVALIDITEM_H
