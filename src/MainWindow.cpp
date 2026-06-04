#include "MainWindow.h"

#include "AddEditItemDialog.h"
#include "Storage.h"

#include <QAction>
#include <QAbstractButton>
#include <QAbstractItemView>
#include <QCloseEvent>
#include <QComboBox>
#include <QDateEdit>
#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFont>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QItemSelectionModel>
#include <QObject>
#include <QProgressBar>
#include <QPushButton>
#include <QSignalBlocker>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QtMath>

namespace {
constexpr int DashboardPage = 0;
constexpr int PackingListPage = 1;
constexpr int AboutPage = 2;

void showSimpleMessage(QWidget *parent, QMessageBox::Icon icon, const QString &text)
{
    QMessageBox message(parent);
    message.setWindowTitle("PackMate");
    message.setIcon(icon);
    message.setText(text);
    message.addButton("Гаразд", QMessageBox::AcceptRole);
    message.exec();
}

bool runTripDialog(QWidget *parent, const QString &windowTitle, Trip &trip)
{
    QDialog dialog(parent);
    dialog.setWindowTitle(windowTitle);
    dialog.resize(420, 220);

    auto *titleEdit = new QLineEdit(&dialog);
    titleEdit->setText(trip.title);

    auto *destinationEdit = new QLineEdit(&dialog);
    destinationEdit->setText(trip.destination);

    auto *startDateEdit = new QDateEdit(trip.startDate.isValid() ? trip.startDate : QDate::currentDate(), &dialog);
    auto *endDateEdit = new QDateEdit(trip.endDate.isValid() ? trip.endDate : QDate::currentDate().addDays(7), &dialog);
    startDateEdit->setCalendarPopup(true);
    endDateEdit->setCalendarPopup(true);

    auto *form = new QFormLayout;
    form->addRow("Назва", titleEdit);
    form->addRow("Місце призначення", destinationEdit);
    form->addRow("Дата початку", startDateEdit);
    form->addRow("Дата завершення", endDateEdit);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog);
    buttons->button(QDialogButtonBox::Save)->setText("Зберегти");
    buttons->button(QDialogButtonBox::Cancel)->setText("Скасувати");

    auto *layout = new QVBoxLayout(&dialog);
    layout->addLayout(form);
    layout->addWidget(buttons);

    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, [&]() {
        if (titleEdit->text().trimmed().isEmpty()) {
            showSimpleMessage(&dialog, QMessageBox::Warning, "Вкажіть назву подорожі.");
            return;
        }
        if (startDateEdit->date() > endDateEdit->date()) {
            showSimpleMessage(&dialog, QMessageBox::Warning, "Дата початку не може бути пізніше дати завершення.");
            return;
        }
        dialog.accept();
    });
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) {
        return false;
    }

    trip.title = titleEdit->text().trimmed();
    trip.destination = destinationEdit->text().trimmed();
    trip.startDate = startDateEdit->date();
    trip.endDate = endDateEdit->date();
    return true;
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    buildUi();
    loadData();
    refreshAll();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!state.hasUnsavedChanges) {
        event->accept();
        return;
    }

    QMessageBox message(this);
    message.setWindowTitle("PackMate");
    message.setIcon(QMessageBox::Question);
    message.setText("Є незбережені зміни. Зберегти перед закриттям?");
    message.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    message.button(QMessageBox::Save)->setText("Зберегти");
    message.button(QMessageBox::Discard)->setText("Не зберігати");
    message.button(QMessageBox::Cancel)->setText("Скасувати");

    const QMessageBox::StandardButton result = static_cast<QMessageBox::StandardButton>(message.exec());

    if (result == QMessageBox::Save) {
        if (saveData()) {
            event->accept();
        } else {
            event->ignore();
        }
    } else if (result == QMessageBox::Discard) {
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::buildUi()
{
    setWindowTitle("PackMate[*]");
    resize(900, 600);

    createMenus();

    stack = new QStackedWidget(this);
    stack->addWidget(createDashboardPage());
    stack->addWidget(createPackingListPage());
    stack->addWidget(createAboutPage());
    setCentralWidget(stack);
}

QWidget *MainWindow::createDashboardPage()
{
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(16);

    auto *title = new QLabel("Головна", page);
    QFont titleFont = title->font();
    titleFont.setPointSize(22);
    titleFont.setBold(true);
    title->setFont(titleFont);

    tripTitleLabel = new QLabel(page);
    tripDatesLabel = new QLabel(page);
    totalItemsLabel = new QLabel(page);
    packedItemsLabel = new QLabel(page);
    progressLabel = new QLabel(page);

    progressBar = new QProgressBar(page);
    progressBar->setRange(0, 100);

    auto *summary = new QGroupBox("Інформація про подорож", page);
    auto *summaryLayout = new QVBoxLayout(summary);
    summaryLayout->addWidget(tripTitleLabel);
    summaryLayout->addWidget(tripDatesLabel);
    summaryLayout->addWidget(totalItemsLabel);
    summaryLayout->addWidget(packedItemsLabel);
    summaryLayout->addWidget(progressLabel);
    summaryLayout->addWidget(progressBar);

    auto *createTripButton = new QPushButton("Створити подорож", page);
    editTripButton = new QPushButton("Редагувати подорож", page);
    openListButton = new QPushButton("Відкрити список речей", page);
    auto *saveButton = new QPushButton("Зберегти", page);

    connect(createTripButton, &QPushButton::clicked, this, &MainWindow::createTrip);
    connect(editTripButton, &QPushButton::clicked, this, &MainWindow::editTrip);
    connect(openListButton, &QPushButton::clicked, this, [this]() {
        stack->setCurrentIndex(PackingListPage);
    });
    connect(saveButton, &QPushButton::clicked, this, [this]() {
        saveData();
    });

    auto *buttons = new QHBoxLayout;
    buttons->addWidget(createTripButton);
    buttons->addWidget(editTripButton);
    buttons->addWidget(openListButton);
    buttons->addWidget(saveButton);
    buttons->addStretch();

    layout->addWidget(title);
    layout->addWidget(summary);
    layout->addLayout(buttons);
    layout->addStretch();

    return page;
}

QWidget *MainWindow::createPackingListPage()
{
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(18, 18, 18, 18);
    layout->setSpacing(12);

    auto *topBar = new QHBoxLayout;
    auto *backButton = new QPushButton("Головна", page);
    filterCombo = new QComboBox(page);
    filterCombo->addItems(categoryFilterLabels());
    topBar->addWidget(backButton);
    topBar->addStretch();
    topBar->addWidget(new QLabel("Категорія", page));
    topBar->addWidget(filterCombo);

    itemsTable = new QTableWidget(page);
    itemsTable->setColumnCount(4);
    itemsTable->setHorizontalHeaderLabels({"Зібрано", "Назва речі", "Категорія", "Кількість / Важливість"});
    itemsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    itemsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    itemsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    itemsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    itemsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    itemsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    itemsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    addButton = new QPushButton("Додати річ", page);
    editButton = new QPushButton("Редагувати річ", page);
    deleteButton = new QPushButton("Видалити річ", page);
    togglePackedButton = new QPushButton("Позначити зібраною", page);

    auto *buttonBar = new QHBoxLayout;
    buttonBar->addWidget(addButton);
    buttonBar->addWidget(editButton);
    buttonBar->addWidget(deleteButton);
    buttonBar->addWidget(togglePackedButton);
    buttonBar->addStretch();

    connect(backButton, &QPushButton::clicked, this, [this]() {
        stack->setCurrentIndex(DashboardPage);
    });
    connect(filterCombo, &QComboBox::currentTextChanged, this, [this](const QString &value) {
        state.activeCategoryFilter = value;
        refreshTable();
    });
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addItem);
    connect(editButton, &QPushButton::clicked, this, &MainWindow::editSelectedItem);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteSelectedItem);
    connect(togglePackedButton, &QPushButton::clicked, this, &MainWindow::toggleSelectedPacked);
    connect(itemsTable, &QTableWidget::itemSelectionChanged, this, [this]() {
        selectedItem();
        refreshTable();
    });
    connect(itemsTable, &QTableWidget::cellDoubleClicked, this, [this](int, int) {
        editSelectedItem();
    });

    layout->addLayout(topBar);
    layout->addWidget(itemsTable);
    layout->addLayout(buttonBar);

    return page;
}

QWidget *MainWindow::createAboutPage()
{
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 28, 28, 28);

    auto *title = new QLabel("PackMate", page);
    QFont titleFont = title->font();
    titleFont.setPointSize(22);
    titleFont.setBold(true);
    title->setFont(titleFont);

    auto *body = new QLabel(
        "PackMate допомагає створити локальний список речей для подорожі, розділити їх за категоріями, "
        "позначати вже зібрані речі та зберігати дані у JSON.\n\n"
        "Як користуватись:\n"
        "1. Створіть подорож на головній сторінці.\n"
        "2. Відкрийте список речей і додайте потрібні позиції.\n"
        "3. Позначайте зібрані речі та стежте за прогресом.\n"
        "4. Збережіть дані перед закриттям програми.\n\n"
        "Автор: проєкт PackMate\n"
        "Версія: 1.0.0",
        page);
    body->setWordWrap(true);

    auto *backButton = new QPushButton("Головна", page);
    connect(backButton, &QPushButton::clicked, this, [this]() {
        stack->setCurrentIndex(DashboardPage);
    });

    layout->addWidget(title);
    layout->addWidget(body);
    layout->addWidget(backButton);
    layout->addStretch();
    return page;
}

void MainWindow::createMenus()
{
    auto *fileMenu = menuBar()->addMenu("Файл");
    auto *newTripAction = fileMenu->addAction("Створити подорож");
    auto *editTripAction = fileMenu->addAction("Редагувати подорож");
    auto *saveAction = fileMenu->addAction("Зберегти");
    fileMenu->addSeparator();
    auto *exitAction = fileMenu->addAction("Вийти");

    auto *viewMenu = menuBar()->addMenu("Перегляд");
    auto *dashboardAction = viewMenu->addAction("Головна");
    auto *listAction = viewMenu->addAction("Список речей");

    auto *helpMenu = menuBar()->addMenu("Довідка");
    auto *aboutAction = helpMenu->addAction("Про програму / Допомога");

    connect(newTripAction, &QAction::triggered, this, &MainWindow::createTrip);
    connect(editTripAction, &QAction::triggered, this, &MainWindow::editTrip);
    connect(saveAction, &QAction::triggered, this, [this]() {
        saveData();
    });
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    connect(dashboardAction, &QAction::triggered, this, [this]() {
        stack->setCurrentIndex(DashboardPage);
    });
    connect(listAction, &QAction::triggered, this, [this]() {
        stack->setCurrentIndex(PackingListPage);
    });
    connect(aboutAction, &QAction::triggered, this, [this]() {
        stack->setCurrentIndex(AboutPage);
    });
}

void MainWindow::loadData()
{
    QString error;
    const PersistedData data = Storage::load(&error);
    if (!error.isEmpty()) {
        qWarning() << "App data load warning:" << error;
        showSimpleMessage(this, QMessageBox::Warning, "Не вдалося завантажити дані: " + error);
    }

    trips = data.trips;
    items = data.items;
    state = data.state;
    qInfo() << "App data load finished";

    if (trips.isEmpty()) {
        state.selectedTripId.clear();
    } else if (state.selectedTripId.isEmpty()) {
        state.selectedTripId = trips.first().id;
    }

    if (state.activeCategoryFilter.isEmpty()) {
        state.activeCategoryFilter = "Усі";
    } else if (state.activeCategoryFilter == "All") {
        state.activeCategoryFilter = "Усі";
    } else if (!categoryFilterLabels().contains(state.activeCategoryFilter)) {
        state.activeCategoryFilter = categoryDisplayString(categoryFromString(state.activeCategoryFilter));
    }
}

bool MainWindow::saveData()
{
    PersistedData data;
    data.trips = trips;
    data.items = items;
    data.state = state;

    QString error;
    if (!Storage::save(data, &error)) {
        qWarning() << "App data save failed:" << error;
        showSimpleMessage(this, QMessageBox::Warning, "Не вдалося зберегти дані: " + error);
        return false;
    }

    qInfo() << "App data save finished";
    setDirty(false);
    statusBar()->showMessage("Збережено у " + Storage::dataFilePath(), 4000);
    return true;
}

void MainWindow::createTrip()
{
    Trip trip;
    trip.id = createId();
    trip.startDate = QDate::currentDate();
    trip.endDate = QDate::currentDate().addDays(7);
    trip.createdAt = QDateTime::currentDateTime();

    if (!runTripDialog(this, "Створити подорож", trip)) {
        return;
    }

    trips.append(trip);
    state.selectedTripId = trip.id;
    state.selectedItemId.clear();
    qInfo() << "Trip created:" << trip.id << trip.title;
    setDirty(true);
    refreshAll();
}

void MainWindow::editTrip()
{
    Trip *trip = selectedTrip();
    if (!trip) {
        showSimpleMessage(this, QMessageBox::Information, "Спочатку створіть подорож.");
        return;
    }

    if (!runTripDialog(this, "Редагувати подорож", *trip)) {
        return;
    }

    qInfo() << "Trip edited:" << trip->id << trip->title;
    setDirty(true);
    refreshAll();
}

void MainWindow::addItem()
{
    Trip *trip = selectedTrip();
    if (!trip) {
        showSimpleMessage(this, QMessageBox::Information, "Спочатку створіть подорож.");
        return;
    }

    PackingItem item;
    item.id = createId();
    item.tripId = trip->id;

    AddEditItemDialog dialog(this);
    dialog.setWindowTitle("Додати річ");
    dialog.setItem(item);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    items.append(dialog.item());
    state.selectedItemId = items.last().id;
    qInfo() << "Item added:" << items.last().id << items.last().name;
    setDirty(true);
    refreshAll();
}

void MainWindow::editSelectedItem()
{
    PackingItem *item = selectedItem();
    if (!item) {
        return;
    }

    AddEditItemDialog dialog(this);
    dialog.setWindowTitle("Редагувати річ");
    dialog.setItem(*item);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    *item = dialog.item();
    qInfo() << "Item edited:" << item->id << item->name;
    setDirty(true);
    refreshAll();
}

void MainWindow::deleteSelectedItem()
{
    PackingItem *item = selectedItem();
    if (!item) {
        return;
    }

    const QString itemId = item->id;
    const QString itemName = item->name;
    QMessageBox message(this);
    message.setWindowTitle("PackMate");
    message.setIcon(QMessageBox::Question);
    message.setText("Видалити вибрану річ?");
    message.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    message.button(QMessageBox::Yes)->setText("Видалити");
    message.button(QMessageBox::Cancel)->setText("Скасувати");

    const QMessageBox::StandardButton result = static_cast<QMessageBox::StandardButton>(message.exec());

    if (result != QMessageBox::Yes) {
        return;
    }

    for (int i = 0; i < items.size(); ++i) {
        if (items[i].id == itemId) {
            items.removeAt(i);
            break;
        }
    }

    state.selectedItemId.clear();
    qInfo() << "Item deleted:" << itemId << itemName;
    setDirty(true);
    refreshAll();
}

void MainWindow::toggleSelectedPacked()
{
    PackingItem *item = selectedItem();
    if (!item) {
        return;
    }

    item->isPacked = !item->isPacked;
    qInfo() << (item->isPacked ? "Item packed:" : "Item unpacked:") << item->id << item->name;
    setDirty(true);
    refreshAll();
}

void MainWindow::refreshAll()
{
    refreshDashboard();
    refreshTable();
}

void MainWindow::refreshDashboard()
{
    const Trip *trip = selectedTrip();
    const int total = totalItemCount();
    const int packed = packedItemCount();
    const int progress = total == 0 ? 0 : qRound((packed * 100.0) / total);

    if (trip) {
        tripTitleLabel->setText("Подорож: " + trip->title + (trip->destination.isEmpty() ? "" : " до " + trip->destination));
        tripDatesLabel->setText("Дати: " + trip->startDate.toString("yyyy-MM-dd") + " - " + trip->endDate.toString("yyyy-MM-dd"));
    } else {
        tripTitleLabel->setText("Подорож: ще не створено");
        tripDatesLabel->setText("Дати: -");
    }

    totalItemsLabel->setText("Усього речей: " + QString::number(total));
    packedItemsLabel->setText("Зібрано речей: " + QString::number(packed));
    progressLabel->setText("Прогрес: " + QString::number(progress) + "%");
    progressBar->setValue(progress);

    openListButton->setEnabled(trip != nullptr);
    editTripButton->setEnabled(trip != nullptr);
}

void MainWindow::refreshTable()
{
    if (!itemsTable) {
        return;
    }

    QSignalBlocker tableBlocker(itemsTable);
    QSignalBlocker filterBlocker(filterCombo);
    filterCombo->setCurrentText(state.activeCategoryFilter);

    const QVector<PackingItem *> visibleItems = filteredItems();
    itemsTable->setRowCount(visibleItems.size());

    bool selectedVisible = false;
    for (int row = 0; row < visibleItems.size(); ++row) {
        const PackingItem *item = visibleItems[row];
        auto *packed = new QTableWidgetItem(item->isPacked ? "Так" : "Ні");
        auto *name = new QTableWidgetItem(item->name);
        auto *category = new QTableWidgetItem(categoryDisplayString(item->category));
        auto *details = new QTableWidgetItem(QString::number(item->quantity) + " / " + priorityDisplayString(item->priority));

        packed->setData(Qt::UserRole, item->id);
        name->setData(Qt::UserRole, item->id);
        category->setData(Qt::UserRole, item->id);
        details->setData(Qt::UserRole, item->id);

        itemsTable->setItem(row, 0, packed);
        itemsTable->setItem(row, 1, name);
        itemsTable->setItem(row, 2, category);
        itemsTable->setItem(row, 3, details);

        if (item->id == state.selectedItemId) {
            itemsTable->selectRow(row);
            selectedVisible = true;
        }
    }

    if (!selectedVisible) {
        state.selectedItemId.clear();
    }

    const bool hasTrip = selectedTrip() != nullptr;
    const bool hasItem = selectedItem() != nullptr;
    addButton->setEnabled(hasTrip);
    editButton->setEnabled(hasItem);
    deleteButton->setEnabled(hasItem);
    togglePackedButton->setEnabled(hasItem);

    PackingItem *item = selectedItem();
    togglePackedButton->setText(item && item->isPacked ? "Позначити незібраною" : "Позначити зібраною");
}

void MainWindow::setDirty(bool dirty)
{
    state.hasUnsavedChanges = dirty;
    setWindowModified(dirty);
}

Trip *MainWindow::selectedTrip()
{
    for (Trip &trip : trips) {
        if (trip.id == state.selectedTripId) {
            return &trip;
        }
    }
    return nullptr;
}

PackingItem *MainWindow::selectedItem()
{
    const int row = selectedTableRow();
    if (row >= 0) {
        QTableWidgetItem *tableItem = itemsTable->item(row, 0);
        if (tableItem) {
            state.selectedItemId = tableItem->data(Qt::UserRole).toString();
        }
    }

    for (PackingItem &item : items) {
        if (item.id == state.selectedItemId && item.tripId == state.selectedTripId) {
            return &item;
        }
    }

    return nullptr;
}

QVector<PackingItem *> MainWindow::filteredItems()
{
    QVector<PackingItem *> result;
    for (PackingItem &item : items) {
        if (item.tripId != state.selectedTripId) {
            continue;
        }
        if (state.activeCategoryFilter != "Усі" && categoryDisplayString(item.category) != state.activeCategoryFilter) {
            continue;
        }
        result.append(&item);
    }
    return result;
}

int MainWindow::totalItemCount() const
{
    int count = 0;
    for (const PackingItem &item : items) {
        if (item.tripId == state.selectedTripId) {
            ++count;
        }
    }
    return count;
}

int MainWindow::packedItemCount() const
{
    int count = 0;
    for (const PackingItem &item : items) {
        if (item.tripId == state.selectedTripId && item.isPacked) {
            ++count;
        }
    }
    return count;
}

int MainWindow::selectedTableRow() const
{
    if (!itemsTable || itemsTable->selectionModel()->selectedRows().isEmpty()) {
        return -1;
    }
    return itemsTable->selectionModel()->selectedRows().first().row();
}
