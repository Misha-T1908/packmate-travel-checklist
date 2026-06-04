#pragma once

#include "Models.h"

#include <QMainWindow>

class QLabel;
class QCloseEvent;
class QProgressBar;
class QPushButton;
class QStackedWidget;
class QTableWidget;
class QComboBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void buildUi();
    QWidget *createDashboardPage();
    QWidget *createPackingListPage();
    QWidget *createAboutPage();
    void createMenus();

    void loadData();
    bool saveData();
    void createTrip();
    void editTrip();
    void addItem();
    void editSelectedItem();
    void deleteSelectedItem();
    void toggleSelectedPacked();

    void refreshAll();
    void refreshDashboard();
    void refreshTable();
    void setDirty(bool dirty);

    Trip *selectedTrip();
    PackingItem *selectedItem();
    QVector<PackingItem *> filteredItems();
    int totalItemCount() const;
    int packedItemCount() const;
    int selectedTableRow() const;

    QVector<Trip> trips;
    QVector<PackingItem> items;
    AppState state;

    QStackedWidget *stack = nullptr;
    QLabel *tripTitleLabel = nullptr;
    QLabel *tripDatesLabel = nullptr;
    QLabel *totalItemsLabel = nullptr;
    QLabel *packedItemsLabel = nullptr;
    QLabel *progressLabel = nullptr;
    QProgressBar *progressBar = nullptr;
    QTableWidget *itemsTable = nullptr;
    QComboBox *filterCombo = nullptr;
    QPushButton *openListButton = nullptr;
    QPushButton *editTripButton = nullptr;
    QPushButton *addButton = nullptr;
    QPushButton *editButton = nullptr;
    QPushButton *deleteButton = nullptr;
    QPushButton *togglePackedButton = nullptr;
};
