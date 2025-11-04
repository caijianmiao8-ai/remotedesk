#pragma once

#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

namespace Protocol {

inline constexpr auto kApiBase = "https://ruoshui.fun.vercel.app";
inline constexpr auto kInputChannelName = "input";

inline QJsonObject makeMouseMovePayload(double x, double y)
{
    QJsonObject obj;
    obj.insert(QStringLiteral("t"), QStringLiteral("move"));
    obj.insert(QStringLiteral("x"), x);
    obj.insert(QStringLiteral("y"), y);
    return obj;
}

inline QJsonObject makeMouseClickPayload(double x, double y, int button)
{
    QJsonObject obj;
    obj.insert(QStringLiteral("t"), QStringLiteral("click"));
    obj.insert(QStringLiteral("x"), x);
    obj.insert(QStringLiteral("y"), y);
    obj.insert(QStringLiteral("button"), button);
    return obj;
}

inline QJsonObject makeMouseWheelPayload(double deltaY)
{
    QJsonObject obj;
    obj.insert(QStringLiteral("t"), QStringLiteral("wheel"));
    obj.insert(QStringLiteral("deltaY"), deltaY);
    return obj;
}

inline QJsonObject makeKeyPayload(const QString &code, const QString &type)
{
    QJsonObject obj;
    obj.insert(QStringLiteral("t"), QStringLiteral("key"));
    obj.insert(QStringLiteral("k"), code);
    obj.insert(QStringLiteral("type"), type);
    return obj;
}

inline QByteArray toJson(const QJsonObject &object)
{
    return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

} // namespace Protocol
