#include "adduseritem.h"
#include "ui_adduseritem.h"

AddUserItem::AddUserItem(QWidget *parent) :
    ListItemBase(parent),
    ui(new Ui::AddUserItem)
{
    ui->setupUi(this);
    SetItemType(ListItemType::AddUserTipItem);
}

AddUserItem::~AddUserItem()
{
    delete ui;
}
