#ifndef INVALIDITEM_H
#define INVALIDITEM_H
#include <QWidget>
#include "listitembase.h"

class InvalidItem : public ListItemBase
{
    Q_OBJECT
public:
    explicit InvalidItem(QWidget *parent = nullptr);
    QSize sizeHint() const override {
        return QSize(250,10); // 返回自定义的尺寸
    }
signals:

public slots:
};

#endif // INVALIDITEM_H
