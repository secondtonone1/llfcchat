#ifndef APPLYFRIEND_H
#define APPLYFRIEND_H

#include <QDialog>
#include "clickedlabel.h"
#include "friendlabel.h"

namespace Ui {
class ApplyFriend;
}

class ApplyFriend : public QDialog
{
    Q_OBJECT

public:
    explicit ApplyFriend(QWidget *parent = nullptr);
    ~ApplyFriend();
    void InitTestLbs();
    bool AddTestLbs(QString str, QPoint cur_point, QPoint &next_point);

private:
    void resetLabels();
    Ui::ApplyFriend *ui;
    QMap<QString, ClickedLabel*> _add_labels;
    std::vector<QString> _add_label_keys;
    QPoint _label_point;
    QMap<QString, FriendLabel*> _friend_labels;
    std::vector<QString> _friend_label_keys;
    void addLabel(QString name);
public slots:
    void ShowMoreLabel();
    void SlotLabelEnter();
    void SlotRemoveFriendLabel(QString);
    void SlotChangeFriendLabelByTip(QString, ClickLbState);
    void SlotLabelTextChange(const QString& text);
    void SlotLabelEditFinished();
};

#endif // APPLYFRIEND_H
