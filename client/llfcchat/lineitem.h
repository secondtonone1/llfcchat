#ifndef LINEITEM_H
#define LINEITEM_H

#include <QWidget>
#include "listitembase.h"

namespace Ui {
class LineItem;
}

class LineItem : public ListItemBase
{
    Q_OBJECT

public:
    explicit LineItem(QWidget *parent = nullptr);
    ~LineItem();
    QSize sizeHint() const override;
private:
    Ui::LineItem *ui;
};

#endif // LINEITEM_H
