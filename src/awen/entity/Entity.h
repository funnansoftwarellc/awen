#pragma once

#include <awen/entity/export.hxx>

#include <QObject>
#include <QString>
#include <QtQml>

namespace awen
{
    struct AWEN_ENTITY_EXPORT Entity
    {
        Q_GADGET
        Q_PROPERTY(QString name MEMBER name)
        Q_PROPERTY(qreal x MEMBER x)
        Q_PROPERTY(qreal y MEMBER y)
        Q_PROPERTY(qreal heading MEMBER heading)
        QML_VALUE_TYPE(entity)
        QML_STRUCTURED_VALUE

        QString name;
        qreal x;
        qreal y;
        qreal heading;
    };
}