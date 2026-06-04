#include "Models.h"

#include <QUuid>

QString categoryToString(Category category)
{
    switch (category) {
    case Category::Documents:
        return "Documents";
    case Category::Clothes:
        return "Clothes";
    case Category::Electronics:
        return "Electronics";
    case Category::Hygiene:
        return "Hygiene";
    case Category::Medicine:
        return "Medicine";
    case Category::Other:
        return "Other";
    }

    return "Other";
}

QString categoryDisplayString(Category category)
{
    switch (category) {
    case Category::Documents:
        return "Документи";
    case Category::Clothes:
        return "Одяг";
    case Category::Electronics:
        return "Електроніка";
    case Category::Hygiene:
        return "Гігієна";
    case Category::Medicine:
        return "Ліки";
    case Category::Other:
        return "Інше";
    }

    return "Інше";
}

Category categoryFromString(const QString &value)
{
    if (value == "Documents" || value == "Документи") {
        return Category::Documents;
    }
    if (value == "Clothes" || value == "Одяг") {
        return Category::Clothes;
    }
    if (value == "Electronics" || value == "Електроніка") {
        return Category::Electronics;
    }
    if (value == "Hygiene" || value == "Гігієна") {
        return Category::Hygiene;
    }
    if (value == "Medicine" || value == "Ліки") {
        return Category::Medicine;
    }
    return Category::Other;
}

QStringList categoryFilterLabels()
{
    return {"Усі", "Документи", "Одяг", "Електроніка", "Гігієна", "Ліки", "Інше"};
}

QStringList categoryLabels()
{
    return {"Документи", "Одяг", "Електроніка", "Гігієна", "Ліки", "Інше"};
}

QString priorityToString(Priority priority)
{
    switch (priority) {
    case Priority::Low:
        return "Low";
    case Priority::Normal:
        return "Normal";
    case Priority::High:
        return "High";
    }

    return "Normal";
}

QString priorityDisplayString(Priority priority)
{
    switch (priority) {
    case Priority::Low:
        return "Низька";
    case Priority::Normal:
        return "Звичайна";
    case Priority::High:
        return "Висока";
    }

    return "Звичайна";
}

Priority priorityFromString(const QString &value)
{
    if (value == "Low" || value == "Низька") {
        return Priority::Low;
    }
    if (value == "High" || value == "Висока") {
        return Priority::High;
    }
    return Priority::Normal;
}

QStringList priorityLabels()
{
    return {"Низька", "Звичайна", "Висока"};
}

QJsonObject tripToJson(const Trip &trip)
{
    QJsonObject object;
    object["id"] = trip.id;
    object["title"] = trip.title;
    object["destination"] = trip.destination;
    object["startDate"] = trip.startDate.toString(Qt::ISODate);
    object["endDate"] = trip.endDate.toString(Qt::ISODate);
    object["createdAt"] = trip.createdAt.toString(Qt::ISODate);
    return object;
}

Trip tripFromJson(const QJsonObject &object)
{
    Trip trip;
    trip.id = object["id"].toString();
    trip.title = object["title"].toString();
    trip.destination = object["destination"].toString();
    trip.startDate = QDate::fromString(object["startDate"].toString(), Qt::ISODate);
    trip.endDate = QDate::fromString(object["endDate"].toString(), Qt::ISODate);
    trip.createdAt = QDateTime::fromString(object["createdAt"].toString(), Qt::ISODate);
    return trip;
}

QJsonObject itemToJson(const PackingItem &item)
{
    QJsonObject object;
    object["id"] = item.id;
    object["tripId"] = item.tripId;
    object["name"] = item.name;
    object["category"] = categoryToString(item.category);
    object["quantity"] = item.quantity;
    object["priority"] = priorityToString(item.priority);
    object["isPacked"] = item.isPacked;
    object["note"] = item.note;
    return object;
}

PackingItem itemFromJson(const QJsonObject &object)
{
    PackingItem item;
    item.id = object["id"].toString();
    item.tripId = object["tripId"].toString();
    item.name = object["name"].toString();
    item.category = categoryFromString(object["category"].toString());
    item.quantity = object["quantity"].toInt(1);
    item.priority = priorityFromString(object["priority"].toString());
    item.isPacked = object["isPacked"].toBool(false);
    item.note = object["note"].toString();
    return item;
}

QString createId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}
