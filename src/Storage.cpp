#include "Storage.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

QString Storage::dataFilePath()
{
    return QCoreApplication::applicationDirPath() + "/data/packing_data.json";
}

PersistedData Storage::load(QString *errorMessage)
{
    PersistedData data;

    QFile file(dataFilePath());
    if (!file.exists()) {
        qInfo() << "Data file not found, starting first launch:" << dataFilePath();
        return data;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = file.errorString();
        }
        qWarning() << "Failed to open data file:" << dataFilePath() << file.errorString();
        return data;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        if (errorMessage) {
            *errorMessage = parseError.errorString();
        }
        qWarning() << "Malformed JSON data file:" << dataFilePath() << parseError.errorString();
        return data;
    }

    if (!document.isObject()) {
        if (errorMessage) {
            *errorMessage = "JSON root is not an object.";
        }
        qWarning() << "Invalid JSON data file root:" << dataFilePath();
        return data;
    }

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

    qInfo() << "Loaded app data:" << data.trips.size() << "trips," << data.items.size() << "items";
    return data;
}

bool Storage::save(const PersistedData &data, QString *errorMessage)
{
    QDir dir(QCoreApplication::applicationDirPath());
    if (!dir.exists("data") && !dir.mkpath("data")) {
        if (errorMessage) {
            *errorMessage = "не вдалося створити папку data.";
        }
        qWarning() << "Failed to create data directory:" << dir.absoluteFilePath("data");
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
        qWarning() << "Failed to open data file for writing:" << dataFilePath() << file.errorString();
        return false;
    }

    const QByteArray json = QJsonDocument(root).toJson(QJsonDocument::Indented);
    const qint64 written = file.write(json);
    if (written != json.size()) {
        if (errorMessage) {
            const QString writeError = file.errorString();
            *errorMessage = writeError.isEmpty() ? QString("не вдалося повністю записати файл.") : writeError;
        }
        qWarning() << "Failed to write complete data file:" << dataFilePath() << "written" << written << "expected" << json.size();
        return false;
    }

    qInfo() << "Saved app data:" << data.trips.size() << "trips," << data.items.size() << "items";
    return true;
}
