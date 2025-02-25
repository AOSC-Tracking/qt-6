// Copyright (C) 2023 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef CHATMESSAGEMODEL_H
#define CHATMESSAGEMODEL_H

#include <QAbstractListModel>
#include <memory>

#include "simplechat.qpb.h"

class ChatMessageModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ChatMessageModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;
    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;

    void append(const QList<qtgrpc::examples::chat::ChatMessage> &messages);

private:
    QList<qtgrpc::examples::chat::ChatMessage> m_container;
};

#endif // CHATMESSAGEMODEL_H
