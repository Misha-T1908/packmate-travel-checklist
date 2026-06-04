#pragma once

#include <QDate>
#include <QDateTime>
#include <QJsonObject>
#include <QString>
#include <QStringList>
#include <QVector>

enum class Category {
    Documents,
    Clothes,
    Electronics,
    Hygiene,
    Medicine,
    Other
};

enum class Priority {
    Low,
    Normal,
    High
};

struct Trip {
    QString id;
    QString title;
    QString destination;
    QDate startDate;
    QDate endDate;
    QDateTime createdAt;
};

struct PackingItem {
    QString id;
    QString tripId;
    QString name;
    Category category = Category::Other;
    int quantity = 1;
    Priority priority = Priority::Normal;
    bool isPacked = false;
    QString note;
};

struct AppState {
    QString selectedTripId;
    QString selectedItemId;
    QString activeCategoryFilter = "All";
    bool hasUnsavedChanges = false;
};

QString categoryToString(Category category);
QString categoryDisplayString(Category category);
Category categoryFromString(const QString &value);
QStringList categoryFilterLabels();
QStringList categoryLabels();

QString priorityToString(Priority priority);
QString priorityDisplayString(Priority priority);
Priority priorityFromString(const QString &value);
QStringList priorityLabels();

QJsonObject tripToJson(const Trip &trip);
Trip tripFromJson(const QJsonObject &object);

QJsonObject itemToJson(const PackingItem &item);
PackingItem itemFromJson(const QJsonObject &object);

QString createId();
