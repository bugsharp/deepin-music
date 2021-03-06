/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "filter.h"

#include <QDebug>

#include <QTimer>
#include <QEvent>
#include <QApplication>
#include <QCursor>
#include <QWidget>

HoverFilter::HoverFilter(QObject *parent) : QObject(parent)
{

}

bool HoverFilter::eventFilter(QObject *obj, QEvent *event)
{
    switch (event->type()) {
    case QEvent::Enter:
        QApplication::setOverrideCursor(QCursor(Qt::PointingHandCursor));
        return true;
    case QEvent::Leave:
        QApplication::restoreOverrideCursor();
        return true;
    default:
        return QObject::eventFilter(obj, event);
    }
}


class HintFilterPrivate
{
public:
    HintFilterPrivate(HintFilter *parent) : q_ptr(parent) {}

    void showHint();

    QTimer  *delayShowTimer = nullptr;

    QWidget *parentWidget = nullptr;
    QWidget *hintWidget = nullptr;

    HintFilter *q_ptr;
    Q_DECLARE_PUBLIC(HintFilter)
};


void HintFilterPrivate::showHint()
{
    if (!parentWidget) {
        return;
    }

    auto w = parentWidget;
    if (hintWidget) {
        hintWidget->hide();
    }
    hintWidget = w->property("HintWidget").value<QWidget *>();
    if (!hintWidget) {
        return;
    }

    hintWidget->show();
    hintWidget->raise();

    auto centerPos = w->mapToGlobal(w->rect().center());
    auto sz = hintWidget->size();
    centerPos.setX(centerPos.x()  - sz.width() / 2);
    centerPos.setY(centerPos.y() - 32 - sz.height());
    centerPos = hintWidget->mapFromGlobal(centerPos);
    centerPos = hintWidget->mapToParent(centerPos);
    hintWidget->move(centerPos);
}


HintFilter::HintFilter(QObject *parent)  : QObject(parent), d_ptr(new HintFilterPrivate(this))
{
    Q_D(HintFilter);
    d->delayShowTimer = new QTimer;
    d->delayShowTimer->setInterval(2000);
    connect(d->delayShowTimer, &QTimer::timeout, this, [ = ]() {
        d->showHint();
        d->delayShowTimer->stop();
    });
}

HintFilter::~HintFilter()
{

}

bool HintFilter::eventFilter(QObject *obj, QEvent *event)
{
    Q_D(HintFilter);
    switch (event->type()) {
    case QEvent::Enter: {
        if (d->hintWidget) {
            d->hintWidget->hide();
        }

        auto w = qobject_cast<QWidget *>(obj);
        d->parentWidget = w;
        if (!w) {
            break;
        }

        d->hintWidget = w->property("HintWidget").value<QWidget *>();
        if (!d->hintWidget) {
            break;
        }

        d->delayShowTimer->stop();

        bool nodelayshow = d->hintWidget->property("NoDelayShow").toBool();
        if (nodelayshow) {
            d->showHint();
        } else {
            d->delayShowTimer->start();
        }

        QApplication::setOverrideCursor(QCursor(Qt::PointingHandCursor));
        break;
    }
    case QEvent::Leave:
        if (d->hintWidget) {
            if (!d->hintWidget->property("DelayHide").toBool()) {
                d->hintWidget->hide();
                d->delayShowTimer->stop();
            } else {
                QMetaObject::invokeMethod(d->hintWidget, "deleyHide", Qt::DirectConnection);
            }
        }
        QApplication::restoreOverrideCursor();
        break;
    case QEvent::MouseButtonPress:
        if (d->hintWidget) {
            d->hintWidget->hide();
            d->delayShowTimer->stop();
        }
    default:
        break;
    }
    return QObject::eventFilter(obj, event);
}
