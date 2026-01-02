#include "ClickableLabel.h"
#include <QMouseEvent>
#include <QPainter>
// ClickableLabel.cpp
ClickableLabel::ClickableLabel(QWidget* parent)
    : QLabel(parent)
    , m_showOverlay(false)
    , m_hovered(false)
{
    setCursor(Qt::PointingHandCursor);
    setMouseTracking(true);
}

void ClickableLabel::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit clicked();
    }
    QLabel::mousePressEvent(event);
}

void ClickableLabel::enterEvent(QEvent* event)
{
    m_hovered = true;
    update();
    QLabel::enterEvent(event);
}

void ClickableLabel::leaveEvent(QEvent* event)
{
    m_hovered = false;
    update();
    QLabel::leaveEvent(event);
}

void ClickableLabel::paintEvent(QPaintEvent* event)
{
    QLabel::paintEvent(event);

    if (m_showOverlay && !m_overlayIcon.isNull()) {
        QPainter painter(this);

        // 绘制半透明遮罩
        if (m_hovered) {
            painter.fillRect(rect(), QColor(0, 0, 0, 100));
        }
        else {
            painter.fillRect(rect(), QColor(0, 0, 0, 60));
        }

        // 绘制图标
        int iconSize = qMin(width(), height()) / 3; // 图标大小为图片的1/3
        QRect iconRect(
            (width() - iconSize) / 2,
            (height() - iconSize) / 2,
            iconSize,
            iconSize
        );

        m_overlayIcon.paint(&painter, iconRect);
    }
}

void ClickableLabel::setIconOverlay(const QIcon& icon)
{
    m_overlayIcon = icon;
    update();
}

void ClickableLabel::showIconOverlay(bool show)
{
    m_showOverlay = show;
    update();
}