// This file is #included by main.cpp (it does not compile on its own).
// Doctor module: view appointments, create a prescription, and view prescriptions.
//
// A prescription is the bridge record between an appointment and a medicine.
// Each line links ONE appointment to ONE medicine, so an appointment can have
// many medicines and a medicine can be used in many appointments (many-to-many).
#include "common.cpp"   // structs, headers, and declarations

// ---- File handling for prescriptions ----
void savePrescriptionToFile(const Prescription& pr) {
    ofstream file("prescriptions.txt", ios::app);
    file << pr.prescriptionID << ","  
         << pr.appointmentID << ","
         << pr.medicineID << ","
         << pr.quantity << ","
         << pr.dosage << "\n";
    file.close();
}

vector<Prescription> loadPrescriptionsFromFile() {
    vector<Prescription> list;
    ifstream file("prescriptions.txt");
    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        Prescription pr;
        getline(ss, pr.prescriptionID, ',');
        getline(ss, pr.appointmentID, ',');
        getline(ss, pr.medicineID, ',');
        getline(ss, pr.quantity, ',');
        getline(ss, pr.dosage, ',');
        list.push_back(pr);
    }
    file.close();
    return list;
}

// ---- Helpers ----
// Next prescription ID number (largest existing + 1)
int nextPrescriptionId() {
    vector<Prescription> list = loadPrescriptionsFromFile();
    int maxId = 0;
    for (const auto& pr : list) {
        if (pr.prescriptionID.empty()) continue;
        int n = 0;
        try { n = stoi(pr.prescriptionID); } catch (...) { n = 0; }
        if (n > maxId) maxId = n;
    }
    return maxId + 1;
}

// True if an appointment with this ID exists (reuses appointment.cpp's loader)
bool appointmentExists(const string& id) {
    vector<Appointment> appts = loadAppointmentsFromFile();
    for (const auto& a : appts) {
        if (a.appointmentID == id) return true;
    }
    return false;
}

// True only if the appointment is approved and can receive a prescription
bool canCreatePrescription(const string& id) {
    vector<Appointment> appts = loadAppointmentsFromFile();

    for (const auto& a : appts) {
        if (a.appointmentID == id) {
            return a.status == STATUS_APPROVED;
        }
    }

    return false;
}

// Returns the position of the medicine in the list, or -1 if it is not there.
// Returning an index (instead of printing) keeps the search logic separate
// from the input/output, and lets the caller edit that medicine's stock.
int findMedicineIndex(const vector<Medicine>& meds, const string& id) {
    for (int i = 0; i < (int)meds.size(); i++) {
        if (meds[i].medicineID == id) return i;
    }
    return -1;
}

// Fixed list of dosage instructions, so the doctor picks instead of typing.
// Returns "0" if the doctor cancels.
string chooseDosage() {
    const int N = 5;
    string dosages[N] = {"Once a day", "Twice a day", "3 times a day",
                         "After meal", "Before sleep"};
    while (true) {
        cout << uiIndent() << "\n";        cout << uiIndent() << "===== Dosage Instruction =====" << "\n";        for (int i = 0; i < N; i++) {
            cout << uiIndent() << (i + 1) << ". " << dosages[i] << "\n";        }
        cout << uiIndent() << "0. Cancel" << "\n";        cout << uiIndent() << "Select dosage: ";        int c = readInt();

        if (c == 0) return "0";
        if (c >= 1 && c <= N) return dosages[c - 1];
        cout << uiIndent() << "Invalid choice. Please try again." << "\n";    }
}

// Returns the position of a prescription line, or -1 if the ID is not there.
int findPrescriptionIndex(const vector<Prescription>& list, const string& id) {
    for (int i = 0; i < (int)list.size(); i++) {
        if (list[i].prescriptionID == id) return i;
    }
    return -1;
}

// Overwrites prescriptions.txt with the whole list (used by update)
void saveAllPrescriptions(const vector<Prescription>& list) {
    ofstream file("prescriptions.txt", ios::trunc);
    for (const auto& pr : list) {
        file << pr.prescriptionID << ","
             << pr.appointmentID << ","
             << pr.medicineID << ","
             << pr.quantity << ","
             << pr.dosage << "\n";
    }
    file.close();
}

// ---- Operations ----
// The doctor's schedule: every appointment, or just one status at a time.
void viewAllAppointments() {
    cout << "\n" << uiIndent() << "=== Appointment Schedule ===\n";    cout << uiIndent() << "1. All appointments\n";    cout << uiIndent() << "2. " << STATUS_PENDING   << " only\n";    cout << uiIndent() << "3. " << STATUS_APPROVED  << " only\n";    cout << uiIndent() << "4. " << STATUS_COMPLETED << " only\n";    cout << uiIndent() << "5. " << STATUS_CANCELLED << " only\n";    cout << uiIndent() << "0. Cancel\n";    cout << uiIndent() << "Choice: ";    int c = readInt();

    if (c == 0) return;
    else if (c == 1) listAppointmentsByStatus("ALL");
    else if (c == 2) listAppointmentsByStatus(STATUS_PENDING);
    else if (c == 3) listAppointmentsByStatus(STATUS_APPROVED);
    else if (c == 4) listAppointmentsByStatus(STATUS_COMPLETED);
    else if (c == 5) listAppointmentsByStatus(STATUS_CANCELLED);
    else cout << "Invalid choice.\n";
}

// Creates ONE prescription line: appointment + medicine + quantity + dosage.
// Call it again with the same appointment ID to add another medicine to that
// same visit - that is how the many-to-many relationship is built up.
void createPrescription() {
    Prescription pr;

    // 1) Which appointment is this prescription for?
    listAppointmentsByStatus(STATUS_APPROVED);

    cout << "\n" << uiIndent() << "Enter appointment ID to prescribe for (0 to cancel): ";    if (cancel(pr.appointmentID)) return;

    if (!appointmentExists(pr.appointmentID)) {
        cout << uiIndent() << "No appointment found with ID "             << pr.appointmentID << ".\n";
        return;
    }

    if (!canCreatePrescription(pr.appointmentID)) {
        cout << uiIndent() << "Cannot create prescription.\n";        cout << uiIndent() << "Only approved appointments can receive prescriptions.\n";        return;
    }

    // 2) Which medicine? The stock list is shown first so the doctor can choose.
    viewStock();

    vector<Medicine> meds = loadMedicinesFromFile();

    if (meds.empty()) {
        cout << uiIndent() << "Cannot prescribe: there is no medicine in stock.\n";        return;
    }

    cout << "\n" << uiIndent() << "Enter medicine ID to prescribe (0 to cancel): ";    if (cancel(pr.medicineID)) return;

    int index = findMedicineIndex(meds, pr.medicineID);

    if (index == -1) {
        cout << uiIndent() << "No medicine found with ID "             << pr.medicineID << ".\n";
        return;
    }

    // 3) How many units? Must be a whole number, at least 1, and in stock.
    int available = 0;

    if (isNumber(meds[index].quantity)) {
        available = stoi(meds[index].quantity);
    }

    int qty = 0;

    while (true) {
        cout << uiIndent() << "Quantity to give (available: "             << available
             << ", 0 to cancel): ";

        if (cancel(pr.quantity)) return;

        if (!isNumber(pr.quantity)) {
            cout << uiIndent() << "  Invalid quantity. Enter a whole number.\n";            continue;
        }

        qty = stoi(pr.quantity);

        if (qty < 1) {
            cout << uiIndent() << "  Quantity must be at least 1.\n";            continue;
        }

        if (qty > available) {
            cout << uiIndent() << "  Not enough stock. Only "                 << available << " left.\n";
            continue;
        }

        break;
    }

    // 4) How should the patient take it?
    pr.dosage = chooseDosage();

    if (pr.dosage == "0") {
        return;
    }

    // 5) Save the prescription and take the medicine out of stock.
    pr.prescriptionID = to_string(nextPrescriptionId());

    savePrescriptionToFile(pr);

    meds[index].quantity = to_string(available - qty);

    saveAllMedicines(meds);

    cout << "\n" << uiIndent() << "Prescription created successfully!\n";
    ostringstream msg;

    msg << "Prescription ID: " << pr.prescriptionID
        << " | Appointment: " << pr.appointmentID
        << " | Medicine: " << meds[index].name
        << " x" << qty
        << " | Dosage: " << pr.dosage;

    cout << uiIndent() << msg.str() << "\n";
    cout << uiIndent() << "Stock of "         << meds[index].name
         << " is now "
         << meds[index].quantity
         << ".\n";
}

// Prints the prescription table. Pass "ALL" for every line, or an appointment
// ID to see just that visit. Split out from viewPrescriptions() so other
// functions can show the table without asking the doctor a question first.
void listPrescriptions(const string& key) {
    vector<Prescription> list = loadPrescriptionsFromFile();
    if (list.empty()) {
        cout << boxIndent(73) << "No prescriptions have been created yet.\n";        return;
    }

    vector<Medicine> meds     = loadMedicinesFromFile();
    vector<Appointment> appts = loadAppointmentsFromFile();

    cout << boxIndent(73) << "\n";    cout << boxIndent(73) << "=========================================================================\n";    cout << boxIndent(73) << "                    BRIGHT SMILE DENTAL CLINIC - PRESCRIPTIONS\n";    cout << boxIndent(73) << "=========================================================================\n";    {
        ostringstream head;
        head << left << setw(6)  << "No"
             << setw(8)  << "Appt"
             << setw(16) << "Patient IC"
             << setw(18) << "Medicine"
             << setw(6)  << "Qty"
             << "Dosage";
        cout << boxIndent(73) << head.str() << "\n";    }
    cout << boxIndent(73) << "-------------------------------------------------------------------------\n";
    int count = 0;
    for (const auto& pr : list) {
        if (key != "ALL" && pr.appointmentID != key) continue;   // filter
        count++;

        // join to the medicine record for its name
        string medName = "-";
        int mi = findMedicineIndex(meds, pr.medicineID);
        if (mi != -1) medName = meds[mi].name;

        // join to the appointment record for the patient's IC
        string ic = "-";
        for (const auto& a : appts) {
            if (a.appointmentID == pr.appointmentID) { ic = a.patientID; break; }
        }

        ostringstream row;
        row << left << setw(6)  << pr.prescriptionID
            << setw(8)  << pr.appointmentID
            << setw(16) << ic
            << setw(18) << medName
            << setw(6)  << pr.quantity
            << pr.dosage;
        cout << boxIndent(73) << row.str() << "\n";    }

    cout << boxIndent(73) << "=========================================================================\n";    if (count == 0) {
        cout << boxIndent(73) << "No prescriptions found for appointment " << key << ".\n";    } else {
        cout << boxIndent(73) << "Total prescription lines shown: " << count << "\n";    }
}

// R: asks which appointment to look at, then shows the table.
void viewPrescriptions() {
    string key;
    cout << uiIndent() << "Enter appointment ID (ALL for everyone, 0 to cancel): ";    if (cancel(key)) return;
    listPrescriptions(key);
}

// ---- Update a prescription line (quantity and/or dosage) ----
// Changing the quantity also corrects the medicine stock: giving 2 more units
// takes 2 more out of stock, giving 3 fewer puts 3 back.
void updatePrescription() {
    listPrescriptions("ALL");   // show everything, no extra question

    string id;
    cout << "\n" << uiIndent() << "Enter prescription ID to update (0 to cancel): ";    if (cancel(id)) return;

    vector<Prescription> list = loadPrescriptionsFromFile();
    int index = findPrescriptionIndex(list, id);
    if (index == -1) {
        cout << uiIndent() << "No prescription found with ID " << id << ".\n";        return;
    }

    vector<Medicine> meds = loadMedicinesFromFile();
    int mi = findMedicineIndex(meds, list[index].medicineID);

    int oldQty = 0;
    if (isNumber(list[index].quantity)) oldQty = stoi(list[index].quantity);

    int inStock = 0;
    if (mi != -1 && isNumber(meds[mi].quantity)) inStock = stoi(meds[mi].quantity);

    cout << uiIndent() << "Current: " << (mi == -1 ? "(medicine removed)" : meds[mi].name)         << " x" << oldQty << ", " << list[index].dosage << "\n";

    // The most that can be given now is what is left in stock plus what this
    // line already took out of it.
    int maxQty = inStock + oldQty;

    string newQty;
    int qty = 0;
    while (true) {
        cout << uiIndent() << "New quantity (max " << maxQty << ", 0 to cancel): ";        if (cancel(newQty)) return;

        if (!isNumber(newQty)) {
            cout << uiIndent() << "  Invalid quantity. Enter a whole number.\n";            continue;
        }
        qty = stoi(newQty);
        if (qty < 1) {
            cout << uiIndent() << "  Quantity must be at least 1.\n";            continue;
        }
        if (qty > maxQty) {
            cout << uiIndent() << "  Not enough stock. The most you can give is " << maxQty << ".\n";            continue;
        }
        break;
    }

    string newDosage = chooseDosage();
    if (newDosage == "0") return;

    list[index].quantity = newQty;
    list[index].dosage   = newDosage;
    saveAllPrescriptions(list);

    // put the difference back into (or take it out of) the stock
    if (mi != -1) {
        meds[mi].quantity = to_string(maxQty - qty);
        saveAllMedicines(meds);
        cout << uiIndent() << "Stock of " << meds[mi].name << " is now " << meds[mi].quantity << ".\n";    }

    cout << uiIndent() << "Prescription " << id << " updated successfully.\n";}

// ---- Update the status of an appointment (e.g. mark it Completed) ----
void doctorUpdateStatus() {
    listAppointmentsByStatus("ALL");

    string id;
    cout << "\n" << uiIndent() << "Enter appointment ID (0 to cancel): ";    if (cancel(id)) return;

    if (!appointmentExists(id)) {
        cout << uiIndent() << "No appointment found with ID " << id << ".\n";        return;
    }

    string newStatus = chooseStatus();
    if (newStatus == "0") return;

    if (setAppointmentStatus(id, newStatus)) {
        cout << uiIndent() << "Appointment " << id << " is now " << newStatus << ".\n";    } else {
        cout << uiIndent() << "No appointment found with ID " << id << ".\n";    }
}

// ---- Cancel a consultation the doctor cannot attend ----
void cancelConsultation() {
    listAppointmentsByStatus("ALL");

    string id;
    cout << "\n" << uiIndent() << "Enter appointment ID to cancel (0 to cancel): ";    if (cancel(id)) return;

    if (!appointmentExists(id)) {
        cout << uiIndent() << "No appointment found with ID " << id << ".\n";        return;
    }

    cout << uiIndent() << "Type YES to confirm cancelling consultation " << id << ": ";    string answer;
    cin >> answer;
    if (answer != "YES") {
        cout << uiIndent() << "Cancellation aborted.\n";        return;
    }

    // The record is kept and simply marked Cancelled, so the history stays.
    setAppointmentStatus(id, STATUS_CANCELLED);
    cout << uiIndent() << "Consultation " << id << " has been cancelled.\n";}

// Shows ONE patient's full clinical history: every appointment they have had,
// and under each one the medicines that were prescribed at that visit.
// The outer loop walks the appointments and the inner loop walks the
// prescription lines belonging to that appointment - so the Appointment,
// Prescription and Medicine records are all joined together here.
void viewPatientHistory() {
    string ic;

    cout << boxIndent(73)
         << "Enter patient IC (0 to cancel): ";

    if (cancel(ic))
        return;


    vector<Patient> patients = loadPatientsFromFile();


    // Find patient
    string name = "";
    string contact = "";

    for (const auto& p : patients) {

        if (p.patientID == ic) {

            name = p.patientName;
            contact = p.patientContact;

            break;
        }
    }


    if (name.empty()) {

        printStatusMessage(
            "No patient found with IC " + ic + "."
        );

        return;
    }


    vector<Appointment> appts =
        loadAppointmentsFromFile();

    vector<Prescription> list =
        loadPrescriptionsFromFile();

    vector<Medicine> meds =
        loadMedicinesFromFile();


    const int BOX_WIDTH = 73;


    cout << "\n";

    // =========================
    // PATIENT INFORMATION
    // =========================

    printBoxTop(BOX_WIDTH);

    printBoxText(
        "Patient : " + name,
        BOX_WIDTH
    );

    printBoxText(
        "IC      : " + ic,
        BOX_WIDTH
    );

    printBoxText(
        "Contact : " + contact,
        BOX_WIDTH
    );

    printBoxDivider(BOX_WIDTH);


    int visitCount = 0;
    int medCount = 0;


    // =========================
    // APPOINTMENT HISTORY
    // =========================

    for (const auto& a : appts) {

        if (a.patientID != ic)
            continue;


        visitCount++;


        // Appointment row
        ostringstream head;

        head << left
             << setw(18)
             << ("Appointment " + a.appointmentID)

             << setw(14)
             << a.date

             << setw(10)
             << a.time

             << a.reason;


        printBoxText(
            head.str(),
            BOX_WIDTH
        );


        // =========================
        // MEDICINE FOR THIS VISIT
        // =========================

        bool given = false;


        for (const auto& pr : list) {

            if (
                pr.appointmentID
                != a.appointmentID
            ) {
                continue;
            }


            given = true;
            medCount++;


            string medName = "-";


            int mi = findMedicineIndex(
                meds,
                pr.medicineID
            );


            if (mi != -1) {

                medName =
                    meds[mi].name;
            }


            ostringstream row;


            row << "     - "

                << left
                << setw(20)
                << fitText(
                    medName,
                    19
                )

                << setw(8)
                << (
                    "x"
                    + pr.quantity
                )

                << pr.dosage;


            printBoxText(
                row.str(),
                BOX_WIDTH
            );
        }


        // No medicine
        if (!given) {

            printBoxText(
                "     - (no medicine prescribed)",
                BOX_WIDTH
            );
        }


        // Divider between visits
        printBoxDivider(
            BOX_WIDTH
        );
    }


    // =========================
    // SUMMARY
    // =========================

    if (visitCount == 0) {

        printBoxCentered(
            "This patient has no appointments yet.",
            BOX_WIDTH
        );
    }

    else {

        ostringstream sum;


        sum << "Total visits: "
            << visitCount

            << "   |   "

            << "Total medicines prescribed: "
            << medCount;


        printBoxText(
            sum.str(),
            BOX_WIDTH
        );
    }


    printBoxBottom(
        BOX_WIDTH
    );
}

void doctorMenu() {
    int choice;
    while (true) {
        clearScreen();
        printAppHeader("DOCTOR PORTAL");
        printMenuHeader("Function");
        printMenuOption(1, "View Appointment Schedule");
        printMenuOption(2, "Create Prescription");
        printMenuOption(3, "View Prescription");
        printMenuOption(4, "Update Prescription");
        printMenuOption(5, "Update Appointment Status");
        printMenuOption(6, "Cancel Consultation");
        printMenuOption(7, "View Patient History");
        printMenuOption(8, "Logout");
        printMenuBottom();
        printChoicePrompt();
        choice = readInt();

        if (choice == 8) break;
        if (choice < 1 || choice > 8) {
            printStatusMessage("INVALID CHOICE");
            waitForEnter();
            continue;
        }

        clearScreen();
        if (choice == 1) { printAppHeader("APPOINTMENT SCHEDULE"); viewAllAppointments(); }
        else if (choice == 2) { printAppHeader("CREATE PRESCRIPTION"); createPrescription(); }
        else if (choice == 3) { printAppHeader("VIEW PRESCRIPTIONS"); viewPrescriptions(); }
        else if (choice == 4) { printAppHeader("UPDATE PRESCRIPTION"); updatePrescription(); }
        else if (choice == 5) { printAppHeader("UPDATE APPOINTMENT STATUS"); doctorUpdateStatus(); }
        else if (choice == 6) { printAppHeader("CANCEL CONSULTATION"); cancelConsultation(); }
        else if (choice == 7) { printAppHeader("PATIENT CLINICAL HISTORY"); viewPatientHistory(); }
        waitForEnter();
    }
}