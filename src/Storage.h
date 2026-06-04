#pragma once

#include "Models.h"

#include <QString>

struct PersistedData {
    QVector<Trip> trips;
    QVector<PackingItem> items;
    AppState state;
};

class Storage
{
public:
    static QString dataFilePath();
    static PersistedData load();
    static bool save(const PersistedData &data, QString *errorMessage = nullptr);
};
