// This file is #included by main.cpp (it does not compile on its own).
// Staff module - the admin assistant at the front counter.
// Handles patient records, books appointments and takes payments on behalf of
// the patient who is standing at the counter, and keeps the medicine stock.
#include "common.cpp"   // structs, headers, and declarations

void viewAllPatients() {
    vector<Patient> patients = loadPatientsFromFile();

    if (patients.empty()) {
        printStatusMessage("No patients registered yet.");
        return;
    }

    const int BOX_WIDTH = 90;

    cout << "\n";

    printBoxTop(BOX_WIDTH);

    {
        ostringstream head;

        head << left
             << setw(4)  << "No"
             << setw(18) << "Name"
             << setw(16) << "IC"
             << setw(14) << "Contact"
             << setw(25) << "Email"
             << setw(10) << "Gender"
             << setw(10) << "Account";

        printBoxText(head.str(), BOX_WIDTH);
    }

    printBoxDivider(BOX_WIDTH);

    int i = 1;

    for (const auto& p : patients) {
        ostringstream row;

        row << left
            << setw(4)  << i++
            << setw(18) << fitText(p.patientName, 17)
            << setw(16) << fitText(p.patientID, 15)
            << setw(14) << fitText(p.patientContact, 13)
            << setw(25) << fitText(p.patientEmail, 24)
            << setw(10) << fitText(p.patientGender, 9)
            << setw(10) << fitText(p.status, 9);

        printBoxText(row.str(), BOX_WIDTH);
    }

    printBoxBottom(BOX_WIDTH);
}

void updatePatient() {
    string ic;
    cout << uiIndent() << "Enter IC of patient to update (0 to cancel): ";    if (cancel(ic)) return;

    vector<Patient> patients = loadPatientsFromFile();
    bool found = false;

    for (auto& p : patients) {          // note: reference so we can edit in place
        if (p.patientID == ic) {
            found = true;
            cout << uiIndent() << "Found patient: " + p.patientName << "\n";            cout << uiIndent() << "Enter new details below (0 to cancel)." << "\n";
            cout << uiIndent() << "New Name: ";            if (cancel(p.patientName)) return;

            cout << uiIndent() << "New Contact: ";            if (cancel(p.patientContact)) return;

            while (true) {
                cout << uiIndent() << "New Email (must be name@gmail.com): ";                if (cancel(p.patientEmail)) return;
                if (isValidEmail(p.patientEmail)) break;
                cout << uiIndent() << "  Invalid email. It must be like name@gmail.com" << "\n";            }

            cout << uiIndent() << "New Gender: ";            if (cancel(p.patientGender)) return;
            break;
        }
    }

    if (found) {
        saveAllPatients(patients);
        cout << uiIndent() << "Patient updated successfully." << "\n";    } else {
        cout << uiIndent() << "No patient found with IC " + ic + "." << "\n";    }
}

void deletePatient() {
    string ic;
    cout << uiIndent() << "Enter IC of patient to delete (0 to cancel): ";    if (cancel(ic)) return;

    vector<Patient> patients = loadPatientsFromFile();
    vector<Patient> remaining;      // everyone we keep
    bool found = false;

    for (const auto& p : patients) {
        if (p.patientID == ic) {
            found = true;           // skip this one = delete it
        } else {
            remaining.push_back(p);
        }
    }

    if (found) {
        saveAllPatients(remaining);
        cout << uiIndent() << "Patient deleted successfully." << "\n";    } else {
        cout << uiIndent() << "No patient found with IC " + ic + "." << "\n";    }
}

// ---- Extra function: Search Patient ----
// The doctor can look up a patient by IC or by (part of) their name. Useful for
// quickly finding a record without scrolling the whole list.
void searchPatient() {
    string keyword;

    cout << boxIndent(80)
         << "Enter IC or name to search (0 to cancel): ";

    if (cancel(keyword))
        return;

    vector<Patient> patients = loadPatientsFromFile();

    int matches = 0;

    const int BOX_WIDTH = 80;

    cout << "\n";

    // Top of result box
    printBoxTop(BOX_WIDTH);

    // Table header
    {
        ostringstream head;

        head << left
             << setw(4)  << "No"
             << setw(16) << "Name"
             << setw(16) << "IC"
             << setw(14) << "Contact"
             << setw(20) << "Email"
             << setw(8)  << "Gender";

        printBoxText(
            head.str(),
            BOX_WIDTH
        );
    }

    printBoxDivider(BOX_WIDTH);


    // Search patients
    for (const auto& p : patients) {

        // Match if IC is exactly the keyword
        // OR name contains the keyword
        if (p.patientID == keyword ||
            p.patientName.find(keyword) != string::npos) {

            matches++;

            ostringstream row;

            row << left
                << setw(4)
                << matches

                << setw(16)
                << fitText(
                    p.patientName,
                    15
                )

                << setw(16)
                << fitText(
                    p.patientID,
                    15
                )

                << setw(14)
                << fitText(
                    p.patientContact,
                    13
                )

                << setw(20)
                << fitText(
                    p.patientEmail,
                    19
                )

                << setw(8)
                << fitText(
                    p.patientGender,
                    7
                );

            printBoxText(
                row.str(),
                BOX_WIDTH
            );
        }
    }


    // Bottom section
    printBoxDivider(BOX_WIDTH);


    if (matches == 0) {

        printBoxCentered(
            "No patient matched \"" +
            keyword + "\".",
            BOX_WIDTH
        );
    }

    else {

        printBoxText(
            "Found " +
            to_string(matches) +
            " matching patient(s).",
            BOX_WIDTH
        );
    }


    printBoxBottom(BOX_WIDTH);
}
void manageRescheduleRequests() {

    vector<Appointment> appointments = loadAppointmentsFromFile();

    cout << "\n" << boxIndent(33) << "===== RESCHEDULE REQUESTS =====\n";
    bool found = false;

    for (const auto& a : appointments) {

        if (a.rescheduleStatus == RESCHEDULE_PENDING) {

            cout << boxIndent(33) << "Appointment ID: " << a.appointmentID << "\n";            cout << boxIndent(33) << "Patient ID: " << a.patientID << "\n";
            cout << boxIndent(33) << "Current Date: " << a.date << "\n";            cout << boxIndent(33) << "Current Time: " << a.time << "\n";
            cout << boxIndent(33) << "Requested Date: " << a.rescheduleDate << "\n";            cout << boxIndent(33) << "Requested Time: " << a.rescheduleTime << "\n";
            cout << boxIndent(33) << "---------------------------------\n";
            found = true;
        }
    }

    if (!found) {
        cout << boxIndent(33) << "No pending reschedule requests.\n";        return;
    }

    string appointmentID;

    cout << boxIndent(33) << "Enter Appointment ID (0 to cancel): ";
    if (cancel(appointmentID)) {
        return;
    }

    for (auto& a : appointments) {

        if (a.appointmentID == appointmentID &&
            a.rescheduleStatus == RESCHEDULE_PENDING) {

            cout << "\n" << boxIndent(33) << "1. Approve\n";            cout << boxIndent(33) << "2. Reject\n";            cout << boxIndent(33) << "0. Cancel\n";            cout << boxIndent(33) << "Choice: ";
            int choice = readInt();

            if (choice == 1) {

                a.date = a.rescheduleDate;
                a.time = a.rescheduleTime;

                a.rescheduleStatus = RESCHEDULE_APPROVED;

                cout << "\n" << boxIndent(33) << "Reschedule request approved.\n";                cout << boxIndent(33) << "New Date: " << a.date << "\n";                cout << boxIndent(33) << "New Time: " << a.time << "\n";            }

            else if (choice == 2) {

                a.rescheduleStatus = RESCHEDULE_REJECTED;

                cout << "\n" << boxIndent(33) << "Reschedule request rejected.\n";            }

            else if (choice == 0) {
                return;
            }

            else {
                cout << boxIndent(33) << "Invalid choice.\n";                return;
            }

            saveAllAppointments(appointments);

            return;
        }
    }

    cout << boxIndent(33) << "Pending reschedule request not found.\n";}

// ---- Counter helpers ----
// At the counter the patient is not logged in, so the assistant types their IC.
// Returns false if the assistant cancels (0) or the IC is not registered.
bool askExistingPatientIC(string& ic) {
    cout << uiIndent() << "Enter patient IC (0 to cancel): ";    if (cancel(ic)) return false;

    if (!patientExists(ic)) {          // reuses the check from appointment.cpp
        cout << uiIndent() << "No patient found with IC " << ic << ". Please register them first.\n";        return false;
    }
    return true;
}

// C: books an appointment for the patient standing at the counter.
// Once the IC is known, the SAME makeAppointment() the patient uses is called,
// so there is only one booking routine in the whole system. A booking taken at
// the counter is Approved straight away - the assistant is confirming it.
void counterBookAppointment() {
    string ic;
    if (!askExistingPatientIC(ic)) return;
    makeAppointment(ic, STATUS_APPROVED);
}

// U: approving the requests patients made themselves.
// Only Pending appointments are listed, so the assistant sees the work queue.
void approveAppointment() {
    cout << "\n";
    centerText("--- Appointment Requests Waiting for Approval ---");

    listAppointmentsByStatus(STATUS_PENDING);

    string id;

    cout << "\n"
         << uiIndent()
         << "Enter appointment ID to approve (0 to cancel): ";

    if (cancel(id))
        return;

    vector<Appointment> appts = loadAppointmentsFromFile();

    bool pending = false;

    for (const auto& a : appts) {
        if (a.appointmentID == id) {

            if (a.status == STATUS_PENDING)
                pending = true;

            break;
        }
    }

    if (!pending) {
        printStatusMessage(
            "Appointment " + id +
            " is not waiting for approval."
        );
        return;
    }

    setAppointmentStatus(
        id,
        STATUS_APPROVED
    );

    printStatusMessage(
        "Appointment " + id +
        " has been approved."
    );
}void counterUpdateStatus() {
    listAppointmentsByStatus("ALL");

    string id;
    cout << "\n" << uiIndent() << "Enter appointment ID (0 to cancel): ";    if (cancel(id)) return;

    if (!appointmentExists(id)) {
        cout << uiIndent() << "No appointment found with ID " << id << ".\n";        return;
    }

    string newStatus = chooseStatus();
    if (newStatus == "0") return;

    setAppointmentStatus(id, newStatus);
    cout << uiIndent() << "Appointment " << id << " is now " << newStatus << ".\n";}

// D: cancelling an appointment on the patient's behalf.
// The record is marked Cancelled rather than erased, so the clinic keeps a
// history of what was booked.
void counterCancelAppointment() {
    string ic;
    if (!askExistingPatientIC(ic)) return;
    viewAppointments(ic);

    string id;
    cout << uiIndent() << "Enter appointment ID to cancel (0 to cancel): ";    if (cancel(id)) return;

    if (!appointmentExists(id)) {
        cout << uiIndent() << "No appointment found with ID " << id << ".\n";        return;
    }

    setAppointmentStatus(id, STATUS_CANCELLED);
    cout << uiIndent() << "Appointment " << id << " has been cancelled.\n";}

void reactivatePatient() {

    string ic;

    cout << "\n" << uiIndent() << "===== REACTIVATE PATIENT ACCOUNT =====\n";    cout << uiIndent() << "Enter Patient IC (0 to cancel): ";
    if (cancel(ic)) {
        return;
    }

    vector<Patient> patients = loadPatientsFromFile();

    bool found = false;

    for (Patient& p : patients) {

        if (p.patientID == ic) {

            found = true;

            // Already active
            if (p.status == ACCOUNT_ACTIVE) {
                cout << uiIndent() << "This patient account is already active.\n";                return;
            }

            // Reactivate account
            p.status = ACCOUNT_ACTIVE;

            saveAllPatients(patients);

            cout << uiIndent() << "Patient account reactivated successfully.\n";            cout << uiIndent() << "Patient ID: " << p.patientID << "\n";            cout << uiIndent() << "Patient Name: " << p.patientName << "\n";            cout << uiIndent() << "Status: " << p.status << "\n";
            return;
        }
    }

    if (!found) {
        cout << uiIndent() << "Patient ID not found.\n";    }
}

void patientRecordMenu() {
    int choice;
    while (true) {
        clearScreen();
        printAppHeader("PATIENT RECORD MANAGEMENT");
        printMenuHeader("Function");
        printMenuOption(1, "Register Patient");
        printMenuOption(2, "Update Patient");
        printMenuOption(3, "Delete Patient");
        printMenuOption(4, "View All Patients");
        printMenuOption(5, "Reactivate Patient Account");
        printMenuOption(6, "Back to Counter Menu");
        printMenuBottom();
        printChoicePrompt();
        choice = readInt();

        if (choice == 6) break;
        if (choice < 1 || choice > 6) {
            printStatusMessage("INVALID CHOICE");
            waitForEnter();
            continue;
        }
        clearScreen();
        if (choice == 1) {
            printAppHeader("REGISTER PATIENT");
            Patient p;
            if (registerPatient(p)) savePatientToFile(p);
            else cout << "Registration cancelled.\n";
        } else if (choice == 2) { printAppHeader("UPDATE PATIENT"); updatePatient(); }
        else if (choice == 3) { printAppHeader("DELETE PATIENT"); deletePatient(); }
        else if (choice == 4) { printAppHeader("PATIENT DIRECTORY"); viewAllPatients(); }
        else if (choice == 5) { printAppHeader("ACCOUNT REACTIVATION"); reactivatePatient(); }
        waitForEnter();
    }
}

// The front counter menu the admin assistant works from.
void staffMenu() {
    int choice;
    while (true) {
        clearScreen();
        printAppHeader("STAFF - FRONT COUNTER PORTAL");
        printMenuHeader("Function");
        printMenuOption(1, "Approve Appointment");
        printMenuOption(2, "Create Appointment for Patient");
        printMenuOption(3, "Update Appointment Status");
        printMenuOption(4, "Cancel Appointment");
        printMenuOption(5, "Manage Reschedule Requests");
        printMenuOption(6, "Search Patient");
        printMenuOption(7, "Patient Records");
        printMenuOption(8, "Medicine Stock Management");
        printMenuOption(9, "Logout");
        printMenuBottom();
        printChoicePrompt();
        choice = readInt();

        if (choice == 9) break;
        if (choice < 1 || choice > 9) {
            printStatusMessage("INVALID CHOICE");
            waitForEnter();
            continue;
        }
        if (choice == 7) { patientRecordMenu(); continue; }
        if (choice == 8) { medicineMenu(); continue; }

        clearScreen();
        if (choice == 1) { printAppHeader("APPROVE APPOINTMENT"); approveAppointment(); }
        else if (choice == 2) { printAppHeader("CREATE APPOINTMENT"); counterBookAppointment(); }
        else if (choice == 3) { printAppHeader("UPDATE APPOINTMENT STATUS"); counterUpdateStatus(); }
        else if (choice == 4) { printAppHeader("CANCEL APPOINTMENT"); counterCancelAppointment(); }
        else if (choice == 5) { printAppHeader("RESCHEDULE REQUESTS"); manageRescheduleRequests(); }
        else if (choice == 6) { printAppHeader("PATIENT SEARCH"); searchPatient(); }
        waitForEnter();
    }
}