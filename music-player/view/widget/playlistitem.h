/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef PLAYLISTITEM_H
#define PLAYLISTITEM_H

#include <QFrame>

#include "../../core/playlist.h"

class QLineEdit;
class PlayListItem : public QFrame
{
    Q_OBJECT
public:
    explicit PlayListItem(PlaylistPtr playlist, QWidget *parent = 0);
    inline PlaylistPtr data() {return m_data;}

    void setActive(bool active);
    void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
signals:
    void rename(const QString &newNameA);
    void remove();
    void playall(PlaylistPtr playlist);

public slots:
    void showContextMenu(const QPoint &pos);

private:
    QLineEdit      *m_titleedit = nullptr;
    PlaylistPtr    m_data;
};

#endif // PLAYLISTITEM_H
