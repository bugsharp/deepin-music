/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "musicitemdelegate.h"

#include <QDebug>
#include <QFont>
#include <QPainter>
#include <QStandardItemModel>

#include <dthememanager.h>
#include <musicmeta.h>

#include "musiclistview.h"
#include "musicitemdelegate_p.h"

DWIDGET_USE_NAMESPACE

const int MusicItemLeftMargin = 15;
const int MusicItemRightMargin = 20;
const int MusicItemNumberMargin = 10;

static inline QString numberString(int row, const QStyleOptionViewItem &option)
{
    auto listview = qobject_cast<const MusicListView *>(option.widget);
    auto itemCount = listview->model()->rowCount();
    auto itemCountString = QString("%1").arg(itemCount);
    return QString("%1").arg(int(row), itemCountString.length(), 10, QChar('0'));
}

static inline int pixel2point(int pixel)
{
    return pixel * 96 / 72;
}

inline int headerPointWidth(const QStyleOptionViewItem &option, const QModelIndex &index)
{
    QFont measuringFont(option.font);
    QFontMetrics fm(measuringFont);
    auto headerWith = fm.width(QString("%1").arg(index.row()));
    return pixel2point(headerWith) + MusicItemLeftMargin + MusicItemNumberMargin;
}

inline int tailPointWidth(const QStyleOptionViewItem &option)
{
    QFont measuringFont(option.font);
    QFontMetrics fm(measuringFont);
    return pixel2point(fm.width("00:00")) + MusicItemRightMargin;
}

MusicItemDelegatePrivate::MusicItemDelegatePrivate(MusicItemDelegate *parent):
    QWidget(nullptr), q_ptr(parent)
{
    setObjectName("MusicItem");

    playingAnimation = new Dtk::Widget::DPictureSequenceView;
    QStringList urls;
    auto urlTemp = QString(":/light/animation/playing/%1.png");
    for (int i = 0; i < 94; ++i) {
        urls << urlTemp.arg(i);
    }
    playingAnimation->setPictureSequence(urls);
    playingAnimation->play();

    D_THEME_INIT_WIDGET(MusicItem);
}

QColor MusicItemDelegatePrivate::textColor() const
{
    return m_textColor;
}
QColor MusicItemDelegatePrivate::titleColor() const
{
    return m_numberColor;
}
QColor MusicItemDelegatePrivate::highlightText() const
{
    return m_highlightText;
}
QColor MusicItemDelegatePrivate::background() const
{
    return m_background;
}
QColor MusicItemDelegatePrivate::alternateBackground() const
{
    return m_alternateBackground;
}
QColor MusicItemDelegatePrivate::highlightedBackground() const
{
    return m_highlightedBackground;
}
void MusicItemDelegatePrivate::setTextColor(QColor textColor)
{
    m_textColor = textColor;
}
void MusicItemDelegatePrivate::setTitleColor(QColor numberColor)
{
    m_numberColor = numberColor;
}
void MusicItemDelegatePrivate::setHighlightText(QColor highlightText)
{
    m_highlightText = highlightText;
}
void MusicItemDelegatePrivate::setBackground(QColor background)
{
    m_background = background;
}
void MusicItemDelegatePrivate::setAlternateBackground(QColor alternateBackground)
{
    m_alternateBackground = alternateBackground;
}
void MusicItemDelegatePrivate::setHighlightedBackground(QColor highlightedBackground)
{
    m_highlightedBackground = highlightedBackground;
}

QColor MusicItemDelegatePrivate::foreground(int col, const QStyleOptionViewItem &option) const
{
    if (option.state & QStyle::State_Selected) {
        return highlightText();
    }

    auto emCol = static_cast<MusicItemDelegate::MusicColumn>(col);
    switch (emCol) {
    case MusicItemDelegate::Number:
    case MusicItemDelegate::Artist:
    case MusicItemDelegate::Album:
    case MusicItemDelegate::Length:
        return textColor();
    case MusicItemDelegate::Title:
        return titleColor();
    case MusicItemDelegate::ColumnButt:
        break;
    }
    return textColor();
}

inline int MusicItemDelegatePrivate::timePropertyWidth(const QStyleOptionViewItem &option) const
{
    static auto width  = tailPointWidth(option);
    return width;
}

static inline QFlags<Qt::AlignmentFlag> alignmentFlag(int col)
{
    auto emCol = static_cast<MusicItemDelegate::MusicColumn>(col);
    switch (emCol) {
    case MusicItemDelegate::Number:
    case MusicItemDelegate::Title:
    case MusicItemDelegate::Artist:
    case MusicItemDelegate::Album:
        return (Qt::AlignLeft | Qt::AlignVCenter);
    case MusicItemDelegate::Length:
        return (Qt::AlignRight | Qt::AlignVCenter);
    case MusicItemDelegate::ColumnButt:
        break;
    }
    return (Qt::AlignLeft | Qt::AlignVCenter);;
}

static inline QRect colRect(int col, const QStyleOptionViewItem &option)
{
    static auto tailwidth  = tailPointWidth(option) + 20;
    auto w = option.rect.width() - 40 - tailwidth;

    auto emCol = static_cast<MusicItemDelegate::MusicColumn>(col);
    switch (emCol) {
    case MusicItemDelegate::Number:
        return QRect(0, option.rect.y(), 40, option.rect.height());
    case MusicItemDelegate::Title:
        return QRect(40, option.rect.y(), w / 2 -20, option.rect.height());
    case MusicItemDelegate::Artist:
        return QRect(40 + w / 2, option.rect.y(), w / 4 -20, option.rect.height());
    case MusicItemDelegate::Album:
        return QRect(40 + w / 2 + w / 4, option.rect.y(), w / 4 -20, option.rect.height());
    case MusicItemDelegate::Length:
        return QRect(40 + w, option.rect.y(), tailwidth - 20, option.rect.height());
    case MusicItemDelegate::ColumnButt:
        break;
    }
    return option.rect.marginsRemoved(QMargins(0, 0, 0, 0));
}


void MusicItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    Q_D(const MusicItemDelegate);

    painter->save();

    QFont font11 = option.font;
    font11.setPixelSize(11);
    QFont font12 = option.font;
    font12.setPixelSize(12);

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::HighQualityAntialiasing);

    auto background = (index.row() % 2) == 0 ? d->background() : d->alternateBackground();

    if (option.state & QStyle::State_Selected) {
        background = d->highlightedBackground();
    }

    painter->fillRect(option.rect, background);
//    painter->setPen(Qt::red);
//    painter->drawRect(option.rect);

    auto meta = index.data().value<MusicMeta>();
    for (int col = 0; col < ColumnButt; ++col) {
        auto textColor = d->foreground(col, option);
        auto flag = alignmentFlag(col);
        auto rect = colRect(col, option);
        painter->setPen(textColor);
        switch (col) {
        case Number: {
            auto *w = const_cast<QWidget *>(option.widget);
            bool hideAnimation = false;
            if (!d->playingIndex.isValid()) {
                hideAnimation = true;
            } else {
                auto listview = qobject_cast<MusicListView *>(w);
                auto viewRect = QRect(QPoint(0, 0), listview->viewport()->size());
                if (!viewRect.intersects(listview->visualRect(d->playingIndex))) {
                    hideAnimation = true;
                }
            }
            if (hideAnimation) {
                d->playingAnimation->hide();
            }

            if (meta.invalid) {
                auto icon = QPixmap(":/light/image/musiclist_warning.png");
                auto centerF = QRectF(rect).center();
                auto iconRect = QRect(centerF.x() - icon.width()/2,
                                      centerF.y() - icon.height()/2,
                                      icon.width(), icon.height());
                painter->drawPixmap(iconRect,icon);
                d->playingAnimation->hide();
                break;
            }

            if (d->playingIndex == index && !hideAnimation) {
                d->playingAnimation->setParent(w);
                d->playingAnimation->raise();
                d->playingAnimation->play();
                d->playingAnimation->show();
                auto center = rect.center();
                auto aniSize = d->playingAnimation->size();
                d->playingAnimation->move(center.x() - aniSize.width() / 2, center.y() - aniSize.height() / 2);
            } else {
//                auto num = numberString(index.row() + 1, option);
//                painter->drawText(rect, flag, num);
            }
            break;
        }
        case Title:
            painter->setFont(font12);
            painter->drawText(rect, flag, meta.title);
            break;
        case Artist: {
            painter->setFont(font11);
            auto str = meta.artist.isEmpty()?
                        MusicListView::tr("Unknow Artist"):
                        meta.artist;
            painter->drawText(rect, flag, str);
            break;
        }
        case Album: {
            painter->setFont(font11);
            auto str = meta.album.isEmpty()?
                        MusicListView::tr("Unknow Album"):
                        meta.album;
            painter->drawText(rect, flag, str);
            break;
        }
        case Length:
            painter->setFont(font11);
            painter->drawText(rect, flag, lengthString(meta.length));
            break;
        default:
            break;
        }
    }
    painter->restore();
}

QSize MusicItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const
{
    Q_D(const MusicItemDelegate);

    auto baseSize = QStyledItemDelegate::sizeHint(option, index);
    return  QSize(baseSize.width() / 5, baseSize.height());
//    auto headerWidth = headerPointWidth(option, index);
    auto headerWidth = 17 + 10 + 10 + 4;
    auto tialWidth = d->timePropertyWidth(option);
    auto w = option.widget->width() - headerWidth - tialWidth;
    Q_ASSERT(w > 0);
    switch (index.column()) {
    case 0:
        return  QSize(headerWidth, baseSize.height());
    case 1:
        return  QSize(w / 2, baseSize.height());
    case 2:
    case 3:
        return  QSize(w / 4, baseSize.height());
    case 4:
        return  QSize(tialWidth, baseSize.height());
    }

    return baseSize;
}

QWidget *MusicItemDelegate::createEditor(QWidget *parent,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const

{
    return QStyledItemDelegate::createEditor(parent, option, index);
}

void MusicItemDelegate::setEditorData(QWidget *editor,
                                      const QModelIndex &index) const
{

    QStyledItemDelegate::setEditorData(editor, index);

}

void MusicItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                     const QModelIndex &index) const
{
    QStyledItemDelegate::setModelData(editor, model, index);
}

MusicItemDelegate::MusicItemDelegate(QWidget *parent)
    : QStyledItemDelegate(parent), d_ptr(new MusicItemDelegatePrivate(this))
{

}

MusicItemDelegate::~MusicItemDelegate()
{
}

void MusicItemDelegate::setPlayingIndex(const QModelIndex &index)
{
    Q_D(MusicItemDelegate);
    d->playingIndex = index;
}

void MusicItemDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    Q_D(const MusicItemDelegate);
    QStyledItemDelegate::initStyleOption(option, index);
    option->state = option->state & ~QStyle::State_HasFocus;

}
