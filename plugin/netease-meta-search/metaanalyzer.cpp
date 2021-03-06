/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include <QDebug>
#include <QUrl>
#include <QByteArray>

#include "metaanalyzer.h"

static bool similarString(QString dest, QString orig)
{
    auto len = dest.length();

    if (len <= 2) {
        return true;
    }

    auto count = 0;
    for (auto c : orig) {
        auto index = dest.indexOf(c);
        if (dest.length() > index && index > 0) {
            dest.remove(index, 1);
            count ++;
        }
    }

    qDebug() << count * 100/ len;
    if (2 * count > len) {
        return true;
    } else {
        return false;
    }
}

MetaAnalyzer::MetaAnalyzer(const MusicMeta &meta, DMusic::Net::Geese *geese, QObject *parent) : QObject(parent)
{
    m_geese = geese;
    m_meta = meta;
    m_delayTimer.setInterval(500);
}

void MetaAnalyzer::onGetTitleResult(QList<NeteaseSong> songlist)
{
    m_titleResult = songlist;
    m_titleResultGet = true;
    analyzerResults();
}

void MetaAnalyzer::onGetAblumResult(QList<NeteaseSong> songlist)
{
    m_ablumResult = songlist;
    m_ablumResultGet = true;
    analyzerResults();
}

void MetaAnalyzer::analyzerResults()
{
    if (!m_titleResultGet || !m_ablumResultGet) {
        return;
    }

    bool find = false;

    NeteaseSong result;
    for (auto &titleResult : m_titleResult) {
        for (auto &albumResut : m_ablumResult) {
            if (titleResult.album.name == albumResut.album.name &&
                    titleResult.name == albumResut.name) {
                qDebug() << "check" << m_meta.title  << titleResult.name <<
                         similarString(m_meta.title, titleResult.name);
                if (similarString(m_meta.title, titleResult.name)) {
                    result = titleResult;
                    find = true;
                    break;
                }
            }
        }
        if (find) {
            break;
        }
    }

    if (!find) {
        m_titleResult = m_titleResult + m_ablumResult;
        for (auto &titleResult : m_titleResult) {
            qDebug() << "similarString" << m_meta.title << titleResult.name;
            if (similarString(m_meta.title, titleResult.name)) {
                result = titleResult;
                find = true;
                break;
            }
        }
    }


    if (!find && m_titleResult.length()) {
        result = m_titleResult.first();
        find = true;
    }

    qDebug() << "find" << result.name << result.album.name;
    qDebug() << "fetch cover url:" << result.album.coverUrl << result.name << m_meta.title;

//    connect(m_geese->getGoose(result.album.coverUrl), &DMusic::Net::Goose::arrive,
//    this, [ = ](int errCode, const QByteArray & data) {
//        qDebug() << "NeteaseMetaSearchEngine recive: " << errCode << data.length();
//        emit this->coverLoaded(m_meta, data);
//    });

    emit searchFinished(m_meta, result);
}




//if (m_titleResult.isEmpty() && !m_ablumResult.isEmpty()) {
//    qDebug() << "get m_titleResult";
//    auto song = m_ablumResult.first();
//    qDebug() << "fetch cover url:" << song.album.coverUrl;
//    connect(m_geese->getGoose(song.album.coverUrl), &DMusic::Net::Goose::arrive,
//    this, [ = ](int errCode, const QByteArray & data) {
//        qDebug() << "NeteaseMetaSearchEngine recive: " << errCode << data.length();
//        emit this->coverLoaded(m_meta, data);
//    });
//}

//if (!m_titleResult.isEmpty() && m_ablumResult.isEmpty()) {
//    qDebug() << "get m_ablumResult";
//    for (auto &song : m_titleResult) {
//        if (song.album.name == m_meta.album) {
//            qDebug() << "fetch cover url:" << song.album.coverUrl;
//            connect(m_geese->getGoose(song.album.coverUrl), &DMusic::Net::Goose::arrive,
//            this, [ = ](int errCode, const QByteArray & data) {
//                qDebug() << "NeteaseMetaSearchEngine recive: " << errCode << data.length();
//                emit this->coverLoaded(m_meta, data);
//            });
//            return;
//        }
//    }

//    auto song = m_titleResult.first();
//    qDebug() << "fetch cover url:" << song.album.coverUrl;
//    connect(m_geese->getGoose(song.album.coverUrl), &DMusic::Net::Goose::arrive,
//    this, [ = ](int errCode, const QByteArray & data) {
//        qDebug() << "NeteaseMetaSearchEngine recive: " << errCode << data.length();
//        emit this->coverLoaded(m_meta, data);
//    });
//    return;
//}

//qDebug() << "get all";
