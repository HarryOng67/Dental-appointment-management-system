// This file is #included by main.cpp (it does not compile on its own).
// Medicine module: add, view stock, update stock, and delete medicine.
#include "common.cpp"   // structs, headers, and declarations

// ---- File handling for medicines ----
void saveMedicineToFile(const Medicine& m) {
    ofstream file("medicine.txt", ios::app);
    file << m.medicineID << ","
         << m.name << ","
         << m.quantity << ","
         << m.price << "\n";
    file.close();
}

vector<Medicine> loadMedicinesFromFile() {
    vector<Medicine> meds;
    ifstream file("medicine.txt");
    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        Medicine m;
        getline(ss, m.medicineID, ',');
        getline(ss, m.name, ',');
        getline(ss, m.quantity, ',');
        getline(ss, m.price, ',');
        meds.push_back(m);
    }
    file.close();
    return meds;
}

// Overwrites medicine.txt with the whole list (used by update/delete)
void saveAllMedicines(const vector<Medicine>& meds) {
    ofstream file("medicine.txt", ios::trunc);
    for (const auto& m : meds) {
        file << m.medicineID << ","
             << m.name << ","
             << m.quantity << ","
             << m.price << "\n";
    }
    file.close();
}

// ---- Helpers ----
// True only if the string is made of digits (used for quantity)
bool isNumber(const string& s) {
    if (s.empty()) return false;
    for (char c : s) {
        if (!isdigit(c)) return false;
    }
    return true;
}

// Next medicine ID number (max existing + 1)
int nextMedicineId() {
    vector<Medicine> meds = loadMedicinesFromFile();
    int maxId = 0;
    for (const auto& m : meds) {
        if (m.medicineID.empty()) continue;
        int n = 0;
        try { n = stoi(m.medicineID); } catch (...) { n = 0; }
        if (n > maxId) maxId = n;
    }
    return maxId + 1;
}

// ---- Operations ----
void addMedicine() {
    Medicine m;
    m.medicineID = to_string(nextMedicineId());

    cout << uiIndent() << "Medicine name (0 to cancel): ";    m.name = readLine();

    if (m.name == "0") {
        return;
    }

    while (true) {
        cout << uiIndent() << "Quantity in stock: ";        if (cancel(m.quantity)) return;

        if (isNumber(m.quantity)) {
            break;
        }

        cout << uiIndent() << "  Invalid quantity. Enter a whole number.\n";    }

    while (true) {
    cout << uiIndent() << "Price per unit (0 to cancel): ";

    if (cancel(m.price))
        return;

    try {
        size_t pos;

        // Convert once and keep the returned value
        double price = stod(m.price, &pos);

        // Make sure the whole input is a valid number
        if (pos != m.price.length()) {
            cout << uiIndent()
                 << "  Invalid price. Please enter a number only.\n";
            continue;
        }

        // Price must be positive
        if (price <= 0) {
            cout << uiIndent()
                 << "  Price must be greater than 0.\n";
            continue;
        }

        break;
    }
    catch (...) {
        cout << uiIndent()
             << "  Invalid price. Please enter a number only.\n";
    }
}
    saveMedicineToFile(m);

    cout << uiIndent() << "Medicine added! Its ID is "         << m.medicineID << ".\n";
}

void viewStock() {
    vector<Medicine> meds = loadMedicinesFromFile();

    if (meds.empty()) {
        printStatusMessage("No medicine in stock yet.");
        return;
    }

    const int BOX_WIDTH = 76;

    cout << "\n";

    // Top of medicine stock box
    printBoxTop(BOX_WIDTH);

    // Table header
    {
        ostringstream head;

        head << left
             << setw(8)  << "ID"
             << setw(24) << "Medicine"
             << setw(12) << "Quantity"
             << setw(14) << "Price (RM)"
             << setw(14) << "Status";

        printBoxText(
            head.str(),
            BOX_WIDTH
        );
    }

    printBoxDivider(BOX_WIDTH);


    // Display medicine records
    for (const auto& m : meds) {

        string stockStatus = "";

        // Low-stock warning when fewer than 10 left
        if (isNumber(m.quantity) &&
            stoi(m.quantity) < 10) {

            stockStatus = "LOW STOCK";
        }


        ostringstream row;

        row << left
            << setw(8)
            << fitText(
                m.medicineID,
                7
            )

            << setw(24)
            << fitText(
                m.name,
                23
            )

            << setw(12)
            << fitText(
                m.quantity,
                11
            )

            << setw(14)
            << fitText(
                m.price,
                13
            )

            << setw(14)
            << fitText(
                stockStatus,
                13
            );


        printBoxText(
            row.str(),
            BOX_WIDTH
        );
    }


    printBoxBottom(BOX_WIDTH);
}

void updateStock() {
    string id;
    cout << uiIndent() << "Enter medicine ID to update (0 to cancel): ";    if (cancel(id)) return;

    vector<Medicine> meds = loadMedicinesFromFile();
    bool found = false;

    for (auto& m : meds) {
        if (m.medicineID == id) {
            found = true;
            cout << uiIndent() << "Found: " << m.name << " (current qty: " << m.quantity << ")\n";
            while (true) {
                cout << uiIndent() << "New quantity: ";                if (cancel(m.quantity)) return;
                if (isNumber(m.quantity)) break;
                cout << uiIndent() << "  Invalid quantity. Enter a whole number.\n";            }
            break;
        }
    }

    if (found) {
        saveAllMedicines(meds);
        cout << uiIndent() << "Stock updated successfully.\n";    } else {
        cout << uiIndent() << "No medicine found with ID " << id << ".\n";    }
}

void deleteMedicine() {
    string id;
    cout << uiIndent() << "Enter medicine ID to delete (0 to cancel): ";    if (cancel(id)) return;

    vector<Medicine> meds = loadMedicinesFromFile();
    vector<Medicine> remaining;
    bool found = false;

    for (const auto& m : meds) {
        if (m.medicineID == id) {
            found = true;           // skip = delete
        } else {
            remaining.push_back(m);
        }
    }

    if (found) {
        saveAllMedicines(remaining);
        cout << uiIndent() << "Medicine deleted successfully.\n";    } else {
        cout << uiIndent() << "No medicine found with ID " << id << ".\n";    }
}

void medicineMenu() {
    int choice;
    while (true) {
        clearScreen();
        printAppHeader("MEDICINE STOCK MANAGEMENT");
        printMenuHeader("Function");
        printMenuOption(1, "Add Medicine");
        printMenuOption(2, "View Stock");
        printMenuOption(3, "Update Stock");
        printMenuOption(4, "Delete Medicine");
        printMenuOption(5, "Back to Counter Menu");
        printMenuBottom();
        printChoicePrompt();
        choice = readInt();

        if (choice == 5) break;
        if (choice < 1 || choice > 5) {
            printStatusMessage("INVALID CHOICE");
            waitForEnter();
            continue;
        }

        clearScreen();
        if (choice == 1) { printAppHeader("ADD MEDICINE"); addMedicine(); }
        else if (choice == 2) { printAppHeader("MEDICINE STOCK"); viewStock(); }
        else if (choice == 3) { printAppHeader("UPDATE MEDICINE STOCK"); updateStock(); }
        else if (choice == 4) { printAppHeader("DELETE MEDICINE"); deleteMedicine(); }
        waitForEnter();
    }
}