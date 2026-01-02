#pragma once
#include <QLabel>
#include <QWidget>
#include <QIcon>

class ClickableLabel :
    public QLabel
{
    Q_OBJECT
public:
    explicit ClickableLabel(QWidget* parent = nullptr);
    void setIconOverlay(const QIcon& icon); //…Ë÷√’⁄’÷Õº±Í
    void showIconOverlay(bool show);  //œ‘ æ/“˛≤ÿ’⁄’÷Õº±Í
protected:
    void mousePressEvent(QMouseEvent* event) override;
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

signals:
    void clicked();

private:
    QIcon m_overlayIcon;
    bool m_showOverlay;
    bool m_hovered;
};

