#include "Storage.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

QString Storage::dataFilePath()
{
    return QCoreApplication::applicationDirPath() + "/data/packing_data.json";
}

PersistedData Storage::load()
{
    PersistedData data;

    QFile file(dataFilePath());
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return data;
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    const QJsonObject root = document.object();

    const QJsonArray trips = root["trips"].toArray();
    for (const QJsonValue &value : trips) {
        data.trips.append(tripFromJson(value.toObject()));
    }

    const QJsonArray items = root["items"].toArray();
    for (const QJsonValue &value : items) {
        data.items.append(itemFromJson(value.toObject()));
    }

    const QJsonObject state = root["state"].toObject();
    data.state.selectedTripId = state["selectedTripId"].toString();
    data.state.selectedItemId = state["selectedItemId"].toString();
    data.state.activeCategoryFilter = state["activeCategoryFilter"].toString("Усі");
    data.state.hasUnsavedChanges = false;

    return data;
}

bool Storage::save(const PersistedData &data, QString *errorMessage)
{
    QDir dir(QCoreApplication::applicationDirPath());
    if (!dir.exists("data") && !dir.mkpath("data")) {
        if (errorMessage) {
            *errorMessage = "не вдалося створити папку data.";
        }
        return false;
    }

    QJsonObject root;

    QJsonArray trips;
    for (const Trip &trip : data.trips) {
        trips.append(tripToJson(trip));
    }
    root["trips"] = trips;

    QJsonArray items;
    for (const PackingItem &item : data.items) {
        items.append(itemToJson(item));
    }
    root["items"] = items;

    QJsonObject state;
    state["selectedTripId"] = data.state.selectedTripId;
    state["selectedItemId"] = data.state.selectedItemId;
    state["activeCategoryFilter"] = data.state.activeCategoryFilter;
    root["state"] = state;

    QFile file(dataFilePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = file.errorString();
        }
        return false;
    }

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}
