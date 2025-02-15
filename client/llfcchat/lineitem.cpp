#include "lineitem.h"
#include "ui_lineitem.h"

LineItem::LineItem(QWidget *parent) :
    ListItemBase(parent),
    ui(new Ui::LineItem)
{
    ui->setupUi(this);
    SetItemType(ListItemType::LINE_ITEM);
}

LineItem::~LineItem()
{
    delete ui;
}

QSize LineItem::sizeHint() const
{
   return QSize(250,5); // 返回自定义的尺寸
}
