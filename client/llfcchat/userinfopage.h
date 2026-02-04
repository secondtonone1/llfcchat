#ifndef USERINFOPAGE_H
#define USERINFOPAGE_H

#include <QWidget>
#include <QLabel>

namespace Ui {
class UserInfoPage;
}

class UserInfoPage : public QWidget
{
    Q_OBJECT

public:
    explicit UserInfoPage(QWidget *parent = nullptr);
    ~UserInfoPage();
    void LoadHeadIcon(QString avatarPath, QLabel* icon_label, QString file_name, QString req_type);
protected:
private slots:
    void slot_up_load();

private:
    Ui::UserInfoPage *ui;

signals:
    void sig_reset_head();
};

#endif // USERINFOPAGE_H
