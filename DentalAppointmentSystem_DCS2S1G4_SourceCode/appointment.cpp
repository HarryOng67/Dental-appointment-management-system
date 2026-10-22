// This file is #included by main.cpp (it does not compile on its own).
// Patient module: make, view, reschedule, and delete appointments.
#include "common.cpp"   // structs, headers, and declarations

// ---- File handling for appointments ----
void saveAppointmentToFile(const Appointment& a) {
    ofstream file("appointments.txt", ios::app);
    // status is written LAST so older 5-field rows still load correctly
    file << a.appointmentID << ","
         << a.patientID << ","
         << a.date << ","
         << a.time << ","
         << a.reason << ","
         << a.status << ","
         << a.rescheduleDate << ","
         << a.rescheduleTime << ","
         << a.rescheduleStatus << "\n";
    file.close();
}

vector<Appointment> loadAppointmentsFromFile() {
    vector<Appointment> appts;
    ifstream file("appointments.txt");
    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        Appointment a;
        getline(ss, a.appointmentID, ',');
        getline(ss, a.patientID, ',');
        getline(ss, a.date, ',');
        getline(ss, a.time, ',');
        getline(ss, a.reason, ',');
        getline(ss, a.status, ',');

        if (!getline(ss, a.rescheduleDate, ',')) {
            a.rescheduleDate = "";
        }

        if (!getline(ss, a.rescheduleTime, ',')) {
            a.rescheduleTime = "";
        }

        if (!getline(ss, a.rescheduleStatus)) {
            a.rescheduleStatus = RESCHEDULE_NONE;
        }

        if (a.rescheduleStatus.empty()) {
            a.rescheduleStatus = RESCHEDULE_NONE;
        }
        // rows saved before statuses existed are treated as still pending
        if (a.status.empty()) a.status = STATUS_PENDING;
        appts.push_back(a);
    }
    file.close();
    return appts;
}

// Overwrites appointments.txt with the whole list (used by update/delete/status)
void saveAllAppointments(const vector<Appointment>& appts) {
    ofstream file("appointments.txt", ios::trunc);

    for (const auto& a : appts) {
        file << a.appointmentID << ","
             << a.patientID << ","
             << a.date << ","
             << a.time << ","
             << a.reason << ","
             << a.status << ","
             << a.rescheduleDate << ","
             << a.rescheduleTime << ","
             << a.rescheduleStatus << "\n";
    }

    file.close();
}
// ---- Helpers ----
// Date must look like DD/MM/YYYY, e.g. 25/12/2026, AND be a real calendar date
// (day 1-31, month 1-12, year 2000-2100). This rejects things like 99/99/9999.
bool isValidDate(const string& d) {
    if (d.length() != 10) return false;
    if (d[2] != '/' || d[5] != '/') return false;
    for (int i = 0; i < 10; i++) {
        if (i == 2 || i == 5) continue;      // skip the '/' positions
        if (!isdigit(d[i])) return false;
    }

    // Pull out the numbers and check they make sense
    int day   = stoi(d.substr(0, 2));
    int month = stoi(d.substr(3, 2));
    int year  = stoi(d.substr(6, 4));

    if (month < 1 || month > 12) return false;
    if (year < 2000 || year > 2100) return false;

    // days in each month (index 1 = Jan ... 12 = Dec)
    int daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    // leap-year check for February
    bool leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    if (month == 2 && leap) daysInMonth[2] = 29;

    if (day < 1 || day > daysInMonth[month]) return false;
    return true;
}

bool isPastDate(const string& d) {
    int day   = stoi(d.substr(0, 2));
    int month = stoi(d.substr(3, 2));
    int year  = stoi(d.substr(6, 4));

    time_t now = time(nullptr);
    tm* current = localtime(&now);

    int todayDay   = current->tm_mday;
    int todayMonth = current->tm_mon + 1;
    int todayYear  = current->tm_year + 1900;

    if (year < todayYear)
        return true;

    if (year > todayYear)
        return false;

    if (month < todayMonth)
        return true;

    if (month > todayMonth)
        return false;

    if (day < todayDay)
        return true;

    return false;
}
// Time must look like HH:MM, e.g. 09:30
bool isValidTime(const string& t) {

    // Time must be exactly HH:MM
    if (t.length() != 5) {
        return false;
    }

    // Check colon position
    if (t[2] != ':') {
        return false;
    }

    // Check that HH and MM are numbers
    if (!isdigit(t[0]) || !isdigit(t[1]) ||
        !isdigit(t[3]) || !isdigit(t[4])) {
        return false;
    }

    // Convert hour and minute to integer
    int hour = stoi(t.substr(0, 2));
    int minute = stoi(t.substr(3, 2));

    // Hour: 00-23
    // Minute: 00-59
    if (hour < 0 || hour > 23) {
        return false;
    }

    if (minute < 0 || minute > 59) {
        return false;
    }

    return true;
}

bool isWithinOperatingHours(const string& t) {

    int hour = stoi(t.substr(0, 2));
    int minute = stoi(t.substr(3, 2));

    // Clinic opens at 08:00
    if (hour < 8) {
        return false;
    }

    // Clinic closes at 18:00
    if (hour > 18) {
        return false;
    }

    // After 18:00 is not allowed
    if (hour == 18 && minute > 0) {
        return false;
    }

    return true;
}

// Check a patient with this IC exists (reuses loadPatientsFromFile from FileManager.cpp)
bool patientExists(const string& ic) {
    vector<Patient> patients = loadPatientsFromFile();
    for (const auto& p : patients) {
        if (p.patientID == ic) return true;
    }
    return false;
}

// Work out the next appointment ID number (max existing + 1)
int nextAppointmentId() {
    vector<Appointment> appts = loadAppointmentsFromFile();
    int maxId = 0;
    for (const auto& a : appts) {
        if (a.appointmentID.empty()) continue;
        int n = 0;
        try { n = stoi(a.appointmentID); } catch (...) { n = 0; }
        if (n > maxId) maxId = n;
    }
    return maxId + 1;
}

// ---- Operations ----
// Lets the patient pick an appointment type from a list.
// The last option lets them type their own reason.
string chooseReason() {
    int c;
    while (true) {
        cout << uiIndent() << "Appointment type:\n";        cout << uiIndent() << "1. Check-up\n";        cout << uiIndent() << "2. Cleaning\n";        cout << uiIndent() << "3. Tooth Extraction\n";        cout << uiIndent() << "4. Filling\n";        cout << uiIndent() << "5. Whitening\n";        cout << uiIndent() << "6. Other (type your own)\n";        cout << uiIndent() << "0. Cancel\n";        cout << uiIndent() << "Choice: ";        c = readInt();

        if (c == 0) return "0";       // "0" is the cancel signal
        else if (c == 1) return "Check-up";
        else if (c == 2) return "Cleaning";
        else if (c == 3) return "Tooth Extraction";
        else if (c == 4) return "Filling";
        else if (c == 5) return "Whitening";
        else if (c == 6) {
            string custom;
            cout << uiIndent() << "Enter your reason (0 to cancel): ";            cin >> custom;
            return custom;            // typing 0 here also returns "0" = cancel
        }
        else {
            cout << uiIndent() << "  Invalid choice. Please try again.\n";        }
    }
}

// Lets the user pick an appointment status. Returns "0" if they cancel.
string chooseStatus() {
    while (true) {
        cout << uiIndent() << "\n";        cout << uiIndent() << "===== Appointment Status =====" << "\n";        cout << uiIndent() << "1. " << STATUS_PENDING   << "\n";        cout << uiIndent() << "2. " << STATUS_APPROVED  << "\n";        cout << uiIndent() << "3. " << STATUS_COMPLETED << "\n";        cout << uiIndent() << "0. Cancel" << "\n";        cout << uiIndent() << "Select status: ";        int c = readInt();

        if (c == 0) return "0";
        else if (c == 1) return STATUS_PENDING;
        else if (c == 2) return STATUS_APPROVED;
        else if (c == 3) return STATUS_COMPLETED;
        else cout << "Invalid choice. Please try again." << "\n";
    }
}

// Finds the appointment and writes the new status back to the file.
// Returns false if no appointment has that ID, so the caller can report it.
// One shared routine, used by both the doctor and the counter staff.
bool setAppointmentStatus(const string& id, const string& newStatus) {
    vector<Appointment> appts = loadAppointmentsFromFile();
    bool found = false;

    for (auto& a : appts) {              // reference, so the record is edited in place
        if (a.appointmentID == id) {
            a.status = newStatus;
            found = true;
            break;
        }
    }

    if (found) saveAllAppointments(appts);
    return found;
}

// Lists appointments filtered by status. Pass "ALL" for every appointment.
void listAppointmentsByStatus(const string& wanted) {
    vector<Appointment> appts = loadAppointmentsFromFile();
    vector<Patient> patients = loadPatientsFromFile();

    const int BOX_WIDTH = 80;

    cout << "\n";

    printBoxTop(BOX_WIDTH);

    {
        ostringstream head;

        head << left
             << setw(4)  << "ID"
             << setw(14) << "Patient IC"
             << setw(11) << "Name"
             << setw(11) << "Date"
             << setw(7)  << "Time"
             << setw(17) << "Reason"
             << setw(10) << "Status";

        printBoxText(
            head.str(),
            BOX_WIDTH
        );
    }

    printBoxDivider(BOX_WIDTH);

    int shown = 0;

    for (const auto& a : appts) {

        if (wanted != "ALL" &&
            a.status != wanted) {
            continue;
        }

        shown++;

        string name = "-";

        for (const auto& p : patients) {
            if (p.patientID == a.patientID) {
                name = p.patientName;
                break;
            }
        }

        ostringstream row;

        row << left
            << setw(4)
            << fitText(a.appointmentID, 3)

            << setw(14)
            << fitText(a.patientID, 13)

            << setw(11)
            << fitText(name, 10)

            << setw(11)
            << fitText(a.date, 10)

            << setw(7)
            << fitText(a.time, 6)

            << setw(17)
            << fitText(a.reason, 16)

            << setw(10)
            << fitText(a.status, 9);

        printBoxText(
            row.str(),
            BOX_WIDTH
        );
    }

    printBoxDivider(BOX_WIDTH);

    if (shown == 0) {

        string message = "No ";

        if (wanted != "ALL") {
            message += wanted + " ";
        }

        message += "appointments found.";

        printBoxCentered(
            message,
            BOX_WIDTH
        );
    }
    else {

        printBoxText(
            "Showing " +
            to_string(shown) +
            " appointment(s).",
            BOX_WIDTH
        );
    }

    printBoxBottom(BOX_WIDTH);
}
// The patient's IC is passed in from login, so they don't type it again.
// The status is passed in too: a patient REQUESTS (Pending), while the counter
// staff booking on the spot can create it already Approved.
void makeAppointment(const string& ic, const string& status) {
    Appointment a;
    a.appointmentID = to_string(nextAppointmentId());
    a.patientID = ic;
    a.status = status;
    a.rescheduleDate = "";
    a.rescheduleTime = "";
    a.rescheduleStatus = RESCHEDULE_NONE;

    while (true) {

        cout << uiIndent()
             << "Date (DD/MM/YYYY) (0 to cancel): ";

        if (cancel(a.date))
            return;

        // Check whether the date format and actual date are valid
        if (!isValidDate(a.date)) {
            cout << uiIndent()
                 << "  Invalid date. Use DD/MM/YYYY, e.g. 25/12/2026\n";
            continue;
        }

        // Do not allow a date before today
        if (isPastDate(a.date)) {
            cout << uiIndent()
                 << "  Date cannot be earlier than today.\n";
            continue;
        }

        break;
    }
    while (true) {

        cout << uiIndent() << "Time (HH:MM) (Operating hours: 08:00-18:00, 0 to cancel): ";
        if (cancel(a.time)) {
            return;
        }

        // Check time format first
        if (!isValidTime(a.time)) {
            cout << uiIndent() << "  Invalid time. Use HH:MM format, e.g. 09:30.\n";            continue;
        }

        // Check clinic operating hours
        if (!isWithinOperatingHours(a.time)) {
            cout << uiIndent() << "  Appointment must be between 08:00 and 18:00.\n";            continue;
        }

        break;
    }
    
    a.reason = chooseReason();
    if (a.reason == "0") return;      // user cancelled at the type menu

    saveAppointmentToFile(a);
    cout << uiIndent() << "Appointment " << a.appointmentID << " saved with status: " << a.status << ".\n";    if (a.status == STATUS_PENDING) {
        cout << uiIndent() << "Please wait for the counter staff to approve it.\n";    }
}

void requestReschedule(const string& patientID) {

    vector<Appointment> appointments = loadAppointmentsFromFile();

    cout << "\n" << uiIndent() << "===== REQUEST RESCHEDULE =====\n";
    bool found = false;

    for (const auto& a : appointments) {
        if (a.patientID == patientID &&
            a.status != "Cancelled" &&
            a.status != "Completed") {

            cout << uiIndent() << "Appointment ID: " << a.appointmentID << "\n";            cout << uiIndent() << "Date: " << a.date << "\n";            cout << uiIndent() << "Time: " << a.time << "\n";            cout << uiIndent() << "Reason: " << a.reason << "\n";            cout << uiIndent() << "Reschedule Status: " << a.rescheduleStatus << "\n\n";
            found = true;
        }
    }

    if (!found) {
        cout << uiIndent() << "No appointment available for rescheduling.\n";        return;
    }

    string appointmentID;

    cout << uiIndent() << "Enter Appointment ID (0 to cancel): ";    if (cancel(appointmentID)) {
        return;
    }

    for (auto& a : appointments) {

        if (a.appointmentID == appointmentID &&
            a.patientID == patientID) {

            if (a.status == "Cancelled" ||
                a.status == "Completed") {

                cout << uiIndent() << "This appointment cannot be rescheduled.\n";                return;
            }

            if (a.rescheduleStatus == RESCHEDULE_PENDING) {
                cout << uiIndent() << "A reschedule request is already pending.\n";                return;
            }

            string newDate;
            string newTime;

            while (true) {

                cout << uiIndent()
                << "New Date (DD/MM/YYYY) (0 to cancel): ";

                if (cancel(newDate)) {
                    return;
                }

                // Check valid date format and actual date
                if (!isValidDate(newDate)) {
                    cout << uiIndent()
                        << "Invalid date. Use DD/MM/YYYY format.\n";
                    continue;
                }

                // Do not allow past date
                if (isPastDate(newDate)) {
                    cout << uiIndent()
                        << "Date cannot be earlier than today.\n";
                continue;
                }

                break;
            }
            while (true) {

                cout << uiIndent() << "New Time (HH:MM) "                     << "(Operating hours: 08:00-18:00, 0 to cancel): ";

                if (cancel(newTime)) {
                    return;
                }

                if (!isValidTime(newTime)) {
                    cout << uiIndent() << "Invalid time. Use HH:MM format.\n";                    continue;
                }

                if (!isWithinOperatingHours(newTime)) {
                    cout << uiIndent() << "Appointment must be between 08:00 and 18:00.\n";                    continue;
                }

                break;
            }

            a.rescheduleDate = newDate;
            a.rescheduleTime = newTime;
            a.rescheduleStatus = RESCHEDULE_PENDING;

            saveAllAppointments(appointments);

            cout << "\n" << uiIndent() << "Reschedule request submitted successfully.\n";            cout << uiIndent() << "Requested Date: " << newDate << "\n";            cout << uiIndent() << "Requested Time: " << newTime << "\n";            cout << uiIndent() << "Status: Pending Staff Approval\n";
            return;
        }
    }

    cout << uiIndent() << "Appointment ID not found.\n";}

void viewAppointments(const string& ic) {
    vector<Appointment> appts = loadAppointmentsFromFile();

    bool any = false;
    int count = 0;

    const int BOX_WIDTH = 76;

    cout << "\n";

    printBoxTop(BOX_WIDTH);

    // Table header
    {
        ostringstream head;

        head << left
             << setw(8)  << "ID"
             << setw(14) << "Date"
             << setw(10) << "Time"
             << setw(22) << "Reason"
             << setw(16) << "Status";

        printBoxText(head.str(), BOX_WIDTH);
    }

    printBoxDivider(BOX_WIDTH);

    // Display patient's appointments
    for (const auto& a : appts) {

        if (a.patientID == ic) {

            any = true;
            count++;

            ostringstream row;

            row << left
                << setw(8)
                << fitText(a.appointmentID, 7)

                << setw(14)
                << fitText(a.date, 13)

                << setw(10)
                << fitText(a.time, 9)

                << setw(22)
                << fitText(a.reason, 21)

                << setw(16)
                << fitText(a.status, 15);

            printBoxText(
                row.str(),
                BOX_WIDTH
            );
        }
    }

    if (!any) {
        printBoxCentered(
            "No appointments found for IC " + ic + ".",
            BOX_WIDTH
        );
    }
    else {
        printBoxDivider(BOX_WIDTH);

        printBoxText(
            "Total appointments: " + to_string(count),
            BOX_WIDTH
        );
    }

    printBoxBottom(BOX_WIDTH);
}

void updateAppointment() {
    string id;
    cout << uiIndent() << "Enter appointment ID to reschedule (0 to cancel): ";    if (cancel(id)) return;

    vector<Appointment> appts = loadAppointmentsFromFile();
    bool found = false;

    for (auto& a : appts) {
        if (a.appointmentID == id) {
            found = true;
            cout << uiIndent() << "Rescheduling appointment " << id << ".\n";
            while (true) {
                cout << uiIndent() << "New Date (DD/MM/YYYY) (0 to cancel): ";                if (cancel(a.date)) return;
                if (isValidDate(a.date)) break;
                cout << uiIndent() << "  Invalid date. Use DD/MM/YYYY\n";            }

            while (true) {
                cout << uiIndent() << "New Time (HH:MM): ";                if (cancel(a.time)) return;
                if (isValidTime(a.time)) break;
                cout << uiIndent() << "  Invalid time. Use HH:MM\n";            }

            string r = chooseReason();
            if (r == "0") return;      // cancelled at type menu
            a.reason = r;
            break;
        }
    }

    if (found) {
        saveAllAppointments(appts);
        cout << uiIndent() << "Appointment updated successfully.\n";    } else {
        cout << uiIndent() << "No appointment found with ID " << id << ".\n";    }
}

void deleteAppointment() {
    string id;
    cout << uiIndent() << "Enter appointment ID to delete (0 to cancel): ";    if (cancel(id)) return;

    vector<Appointment> appts = loadAppointmentsFromFile();
    vector<Appointment> remaining;
    bool found = false;

    for (const auto& a : appts) {
        if (a.appointmentID == id) {
            found = true;           // skip = delete
        } else {
            remaining.push_back(a);
        }
    }

    if (found) {
        saveAllAppointments(remaining);
        cout << uiIndent() << "Appointment cancelled successfully.\n";    } else {
        cout << uiIndent() << "No appointment found with ID " << id << ".\n";    }
}

// ---- Payments (patient pays for their visit) ----
void savePaymentToFile(const Payment& pay) {
    ofstream file("payments.txt", ios::app);
    // service, status and discount are written last so older rows still load
    file << pay.paymentID << ","
         << pay.patientID << ","
         << pay.amount << ","
         << pay.date << ","
         << pay.service << ","
         << pay.status << ","
         << pay.discount << "\n";
    file.close();
}

vector<Payment> loadPaymentsFromFile() {
    vector<Payment> pays;
    ifstream file("payments.txt");
    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        Payment pay;
        getline(ss, pay.paymentID, ',');
        getline(ss, pay.patientID, ',');
        getline(ss, pay.amount, ',');
        getline(ss, pay.date, ',');
        getline(ss, pay.service, ',');   // empty for old rows without a service
        getline(ss, pay.status, ',');
        getline(ss, pay.discount, ',');
        // money already recorded before bills existed was money received
        if (pay.status.empty())   pay.status = PAY_PAID;
        if (pay.discount.empty()) pay.discount = "0.00";
        pays.push_back(pay);
    }
    file.close();
    return pays;
}

// Overwrites payments.txt with the whole list (used by pay / update / refund)
void saveAllPayments(const vector<Payment>& pays) {
    ofstream file("payments.txt", ios::trunc);
    for (const auto& pay : pays) {
        file << pay.paymentID << ","
             << pay.patientID << ","
             << pay.amount << ","
             << pay.date << ","
             << pay.service << ","
             << pay.status << ","
             << pay.discount << "\n";
    }
    file.close();
}

// ================= Age-based discount =================
// A Malaysian IC starts with the birth date: YYMMDD. The first six digits are
// read back out to work out how old the patient is today.
// Returns -1 if the IC cannot be read as a date.
int getAgeFromIC(const string& ic) {
    if (ic.length() != 12) return -1;
    for (int i = 0; i < 6; i++) {
        if (!isdigit(ic[i])) return -1;
    }

    int yy = stoi(ic.substr(0, 2));
    int mm = stoi(ic.substr(2, 2));
    int dd = stoi(ic.substr(4, 2));
    if (mm < 1 || mm > 12 || dd < 1 || dd > 31) return -1;

    // today's date, so the age is always current
    time_t now = time(0);
    tm* today = localtime(&now);
    int curYear  = today->tm_year + 1900;
    int curMonth = today->tm_mon + 1;
    int curDay   = today->tm_mday;

    // a two-digit year that has not happened yet must belong to the 1900s
    int birthYear = (yy <= (curYear % 100)) ? (2000 + yy) : (1900 + yy);

    int age = curYear - birthYear;
    // if this year's birthday has not arrived yet, they are still a year younger
    if (curMonth < mm || (curMonth == mm && curDay < dd)) age--;

    if (age < 0 || age > 120) return -1;   // guard against a nonsense IC
    return age;
}

// The discount rate for an age: children pay less, senior citizens pay less.
// Returns 0.0 for everyone in between, and for an unreadable IC.
double discountRateForAge(int age) {
    if (age < 0)                 return 0.0;
    if (age < YOUTH_MAX_AGE)     return YOUTH_DISCOUNT_RATE;
    if (age >= SENIOR_MIN_AGE)   return SENIOR_DISCOUNT_RATE;
    return 0.0;
}

// Works out what the patient actually pays, prints the breakdown, and passes
// the discount amount back through 'discountOut'. Returns the final price.
// Used by BOTH the patient paying at the counter and management raising a bill,
// so the discount rule exists in only one place.
double applyAgeDiscount(const string& ic, double price, double& discountOut) {
    int age = getAgeFromIC(ic);
    double rate = discountRateForAge(age);

    discountOut = price * rate;
    double payable = price - discountOut;

    cout << uiIndent() << "\n";    cout << uiIndent() << "------------------------------------------------------------" << "\n";    {
        ostringstream row;
        row << left << setw(40) << "Service charge"
            << right << setw(18) << fixed << setprecision(2) << price;
        cout << uiIndent() << row.str() << "\n";    }

    if (rate > 0) {
        // build the label, e.g. "Senior discount (age 67, 20%)"
        ostringstream label;
        label << (age < YOUTH_MAX_AGE ? "Youth discount" : "Senior discount")
              << " (age " << age << ", " << (int)(rate * 100) << "%)";
        ostringstream row;
        row << left << setw(40) << label.str()
            << right << setw(18) << fixed << setprecision(2) << (-discountOut);
        cout << uiIndent() << row.str() << "\n";    } else if (age >= 0) {
        ostringstream row;
        row << left << setw(40) << ("No discount (age " + to_string(age) + ")")
            << right << setw(18) << "0.00";
        cout << uiIndent() << row.str() << "\n";    }

    cout << uiIndent() << "------------------------------------------------------------" << "\n";    {
        ostringstream row;
        row << left << setw(40) << "AMOUNT PAYABLE (RM)"
            << right << setw(18) << fixed << setprecision(2) << payable;
        cout << uiIndent() << row.str() << "\n";    }
    cout << uiIndent() << "------------------------------------------------------------" << "\n";
    return payable;
}

int nextPaymentId() {
    vector<Payment> pays = loadPaymentsFromFile();
    int maxId = 0;
    for (const auto& p : pays) {
        if (p.paymentID.empty()) continue;
        int n = 0;
        try { n = stoi(p.paymentID); } catch (...) { n = 0; }
        if (n > maxId) maxId = n;
    }
    return maxId + 1;
}

// Amount can be digits with at most one decimal point, e.g. 150 or 150.00
bool isValidAmount(const string& s) {
    if (s.empty()) return false;
    int dots = 0;
    for (char c : s) {
        if (c == '.') {
            dots++;
            if (dots > 1) return false;
        } else if (!isdigit(c)) {
            return false;
        }
    }
    return true;
}

// ---- Fixed dental service price list ----
// The two arrays line up by index (names[i] costs prices[i]).
// Returns the chosen service name and sets priceOut to its price.
// Returns "0" if the patient cancels.
string chooseService(double& priceOut) {
    const int N = 5;
    string names[N]  = {"Check-up", "Cleaning", "Filling", "Tooth Extraction", "Whitening"};
    double prices[N] = {50.00,      80.00,      120.00,    150.00,             300.00};

    while (true) {
        cout << uiIndent() << "\n";        cout << uiIndent() << "===== Dental Services =====" << "\n";        for (int i = 0; i < N; i++) {
            ostringstream row;
            row << (i + 1) << ". " << left << setw(18) << names[i]
                << "RM " << fixed << setprecision(2) << prices[i];
            cout << uiIndent() << row.str() << "\n";        }
        cout << uiIndent() << "0. Cancel" << "\n";        cout << uiIndent() << "Select a service: ";        int c = readInt();

        if (c == 0) return "0";
        if (c >= 1 && c <= N) {
            priceOut = prices[c - 1];
            return names[c - 1];
        }
        cout << uiIndent() << "Invalid choice. Please try again." << "\n";    }
}

// Settling an outstanding bill: the patient picks one of their Unpaid bills and
// it becomes Paid. Returns true if a bill was settled.
bool settleOutstandingBill(const string& ic) {
    vector<Payment> pays = loadPaymentsFromFile();

    // show only this patient's unpaid bills
    int outstanding = 0;
    double owed = 0;
    cout << "\n" << uiIndent() << "--- Your Outstanding Bills ---\n";    for (const auto& p : pays) {
        if (p.patientID == ic && p.status == PAY_UNPAID) {
            outstanding++;
            try { owed += stod(p.amount); } catch (...) {}
            ostringstream row;
            row << "Bill " << left << setw(5) << p.paymentID
                << setw(20) << p.service
                << "RM " << p.amount;
            cout << uiIndent() << row.str() << "\n";        }
    }

    if (outstanding == 0) {
        cout << uiIndent() << "You have no outstanding bills.\n";        return false;
    }
    cout << uiIndent() << "Total owed: RM " << fixed << setprecision(2) << owed << "\n";
    string id;
    cout << uiIndent() << "Enter the bill ID to pay (0 to cancel): ";    if (cancel(id)) return false;

    bool found = false;
    for (auto& p : pays) {
        if (p.paymentID == id && p.patientID == ic && p.status == PAY_UNPAID) {
            found = true;
            while (true) {

                cout << uiIndent()
                << "Payment date (DD/MM/YYYY) (0 to cancel): ";

                if (cancel(p.date))
                     return false;

                // Check valid date
                if (!isValidDate(p.date)) {
                    cout << uiIndent()
                         << "  Invalid date. Use DD/MM/YYYY\n";
                    continue;
                }

                // Do not allow past date
                if (isPastDate(p.date)) {
                    cout << uiIndent()
                        << "  Payment date cannot be earlier than today.\n";
                    continue;
                }

                break;
        }

            p.status = PAY_PAID;

            break;
        }
    }

    if (!found) {
        cout << uiIndent() << "No outstanding bill of yours has ID " << id << ".\n";        return false;
    }

    saveAllPayments(pays);
    cout << uiIndent() << "Bill " << id << " has been paid. Thank you!\n";    return true;
}

void makePayment(const string& ic) {
    // If the clinic has already raised a bill, settle that instead of
    // creating a second record for the same treatment.
    vector<Payment> pays = loadPaymentsFromFile();
    bool hasUnpaid = false;
    for (const auto& p : pays) {
        if (p.patientID == ic && p.status == PAY_UNPAID) { hasUnpaid = true; break; }
    }

    if (hasUnpaid) {
        cout << "\n" << uiIndent() << "You have outstanding bills.\n";        cout << uiIndent() << "1. Pay an outstanding bill\n";        cout << uiIndent() << "2. Pay for a service now\n";        cout << uiIndent() << "0. Cancel\n";        cout << uiIndent() << "Choice: ";        int c = readInt();

        if (c == 0) return;
        if (c == 1) { settleOutstandingBill(ic); return; }
        if (c != 2) { cout << "Invalid choice.\n"; return; }
    }

    Payment pay;
    pay.paymentID = to_string(nextPaymentId());
    pay.patientID = ic;

    // Patient picks a service; the price is fixed (not typed by the patient).
    double price = 0;
    pay.service = chooseService(price);
    if (pay.service == "0") return;   // cancelled

    // young and senior patients automatically pay less
    double discount = 0;
    double payable = applyAgeDiscount(pay.patientID, price, discount);

    ostringstream amt, dis;
    amt << fixed << setprecision(2) << payable;
    dis << fixed << setprecision(2) << discount;
    pay.amount   = amt.str();
    pay.discount = dis.str();

    while (true) {
        cout << uiIndent()
             << "Date (DD/MM/YYYY) (0 to cancel): ";

        if (cancel(pay.date))
            return;

        if (!isValidDate(pay.date)) {
            cout << uiIndent()
                 << "  Invalid date. Use DD/MM/YYYY\n";
            continue;
        }

        if (isPastDate(pay.date)) {
            cout << uiIndent()
                 << "  Payment date cannot be earlier than today.\n";
            continue;
        }

        break;
    }

    pay.status = PAY_PAID;
    savePaymentToFile(pay);
    cout << uiIndent() << "Payment successful!" << "\n";    ostringstream msg;
    msg << pay.service << " - RM " << pay.amount << " | Payment ID: " << pay.paymentID;
    cout << uiIndent() << msg.str() << "\n";}

// ---- Extra function: Print Receipt ----
// Produces a neatly formatted receipt of all payments the logged-in patient
// has made, with a running total. Uses <iomanip> manipulators for alignment.
void printReceipt(const string& ic) {
    vector<Payment> pays = loadPaymentsFromFile();
    vector<Patient> patients = loadPatientsFromFile();

    string name = ic;
    for (const auto& p : patients) {
        if (p.patientID == ic) {
            name = p.patientName;
            break;
        }
    }

    vector<Payment> mine;
    for (const auto& p : pays) {
        if (p.patientID == ic)
            mine.push_back(p);
    }

    if (mine.empty()) {
        printStatusMessage("No payments found to print a receipt for.");
        return;
    }

    const int BOX_WIDTH = 64;
    double total = 0.0;

    cout << "\n";
    printBoxTop(BOX_WIDTH);
    printBoxText("Patient : " + name, BOX_WIDTH);
    printBoxText("IC      : " + ic, BOX_WIDTH);
    printBoxDivider(BOX_WIDTH);

    {
        ostringstream head;
        head << left
             << setw(5)  << "ID"
             << setw(17) << "Service"
             << setw(13) << "Date"
             << setw(11) << "Status"
             << right << setw(14) << "Amount (RM)";
        printBoxText(head.str(), BOX_WIDTH);
    }

    printBoxDivider(BOX_WIDTH);

    for (const auto& p : mine) {
        double amt = 0.0;
        try {
            amt = stod(p.amount);
        }
        catch (...) {
            amt = 0.0;
        }

        if (p.status == PAY_PAID)
            total += amt;

        ostringstream row;
        row << left
            << setw(5)  << fitText(p.paymentID, 4)
            << setw(17) << fitText(p.service.empty() ? "-" : p.service, 16)
            << setw(13) << fitText(p.date, 12)
            << setw(11) << fitText(p.status, 10)
            << right << setw(14)
            << fixed << setprecision(2) << amt;

        printBoxText(row.str(), BOX_WIDTH);
    }

    printBoxDivider(BOX_WIDTH);

    {
        ostringstream totalRow;
        totalRow << left << setw(42) << "TOTAL PAID"
                 << right << setw(16)
                 << fixed << setprecision(2) << total;
        printBoxText(totalRow.str(), BOX_WIDTH);
    }

    printBoxBlank(BOX_WIDTH);
    printBoxCentered("Thank you for your payment!", BOX_WIDTH);
    printBoxBottom(BOX_WIDTH);
}

// ---- Patient profile (R and U of the patient's own record) ----
void viewMyProfile(const string& ic) {
    vector<Patient> patients = loadPatientsFromFile();

    for (const auto& p : patients) {
        if (p.patientID == ic) {
            cout << "\n";
            printInnerTop();
            printInfoRow("Name", p.patientName);
            printInfoRow("IC", p.patientID);
            printInfoRow("Contact", p.patientContact);
            printInfoRow("Email", p.patientEmail);
            printInfoRow("Gender", p.patientGender);
            printInfoRow("Account", p.status);
            printInnerBottom();
            return;
        }
    }

    printStatusMessage("Profile not found.");
}

// The patient edits their OWN record, so the IC is never asked for or changed.
void updateMyProfile(const string& ic) {
    vector<Patient> patients = loadPatientsFromFile();
    bool found = false;

    for (auto& p : patients) {        // reference, so the record is edited in place
        if (p.patientID == ic) {
            found = true;
            cout << "\n" << uiIndent() << "Updating profile for " << p.patientName << " (0 to cancel).\n";
            while (true) {
                cout << uiIndent() << "New Name: ";                if (cancel(p.patientName)) return;
                if (isValidName(p.patientName)) break;
                cout << uiIndent() << "  Invalid name. Use letters only.\n";            }

            while (true) {
               cout << uiIndent() << "New Contact Number (digits only, 0 to cancel): ";
                if (cancel(p.patientContact)) {
                    return;
                }
                if (isValidContact(p.patientContact)) {
                    break;
                }
                cout << uiIndent() << "  Invalid contact number. Digits only are allowed.\n";            }

            while (true) {
                cout << uiIndent() << "New Email (must be name@gmail.com): ";                if (cancel(p.patientEmail)) return;
                if (isValidEmail(p.patientEmail)) break;
                cout << uiIndent() << "  Invalid email. It must be like name@gmail.com\n";            }

            while (true) {
                cout << uiIndent()
                     << "New Gender (Male/Female, 0 to cancel): ";

                if (cancel(p.patientGender))
                    return;

                if (p.patientGender == "Male" ||p.patientGender == "Female") {
                    break;
                 }

                cout << uiIndent() << "  Invalid gender. Please enter Male or Female only.\n";
            }

            while (true) {
                cout << uiIndent() << "New Password (min 8, 1 upper, 1 lower, 1 number, 1 special): ";                if (cancel(p.patientPassword)) return;
                if (isValidPassword(p.patientPassword)) break;
                cout << uiIndent() << "  Weak password. Please try again.\n";            }
            break;
        }
    }

    if (found) {
        saveAllPatients(patients);
        cout << uiIndent() << "Profile updated successfully.\n";    } else {
        cout << uiIndent() << "Profile not found.\n";    }
}

// Soft delete: the record (and its history) is kept, but the account can no
// longer log in. 'loggedOut' tells the menu to close after a deactivation.
void deactivateMyAccount(const string& ic, bool& loggedOut) {
    cout << "\n" << uiIndent() << "Deactivating your account will log you out and block future logins.\n";    cout << uiIndent() << "Your appointment and payment history will be kept.\n";    cout << uiIndent() << "Type YES to confirm (anything else cancels): ";    string answer;
    cin >> answer;

    if (answer != "YES") {
        cout << uiIndent() << "Account deactivation cancelled.\n";        return;
    }

    vector<Patient> patients = loadPatientsFromFile();
    bool found = false;
    for (auto& p : patients) {
        if (p.patientID == ic) {
            p.status = ACCOUNT_INACTIVE;
            found = true;
            break;
        }
    }

    if (found) {
        saveAllPatients(patients);
        cout << uiIndent() << "Your account has been deactivated. Logging out...\n";        loggedOut = true;
    } else {
        cout << uiIndent() << "Account not found.\n";    }
}

// ---- The patient's own prescriptions (read-only) ----
void viewMyPrescriptions(const string& ic) {
    vector<Appointment> appts = loadAppointmentsFromFile();
    vector<Prescription> list = loadPrescriptionsFromFile();
    vector<Medicine> meds = loadMedicinesFromFile();

    const int BOX_WIDTH = 74;

    cout << "\n";
    printBoxTop(BOX_WIDTH);

    {
        ostringstream head;
        head << left
             << setw(5)  << "No"
             << setw(7)  << "Appt"
             << setw(13) << "Date"
             << setw(18) << "Medicine"
             << setw(6)  << "Qty"
             << setw(19) << "Dosage";
        printBoxText(head.str(), BOX_WIDTH);
    }

    printBoxDivider(BOX_WIDTH);

    int count = 0;

    for (const auto& pr : list) {
        string apptDate = "";
        bool mine = false;

        for (const auto& a : appts) {
            if (a.appointmentID == pr.appointmentID &&
                a.patientID == ic) {
                mine = true;
                apptDate = a.date;
                break;
            }
        }

        if (!mine)
            continue;

        count++;

        string medName = "-";
        int mi = findMedicineIndex(meds, pr.medicineID);

        if (mi != -1)
            medName = meds[mi].name;

        ostringstream row;
        row << left
            << setw(5)  << fitText(pr.prescriptionID, 4)
            << setw(7)  << fitText(pr.appointmentID, 6)
            << setw(13) << fitText(apptDate, 12)
            << setw(18) << fitText(medName, 17)
            << setw(6)  << fitText(pr.quantity, 5)
            << setw(19) << fitText(pr.dosage, 18);

        printBoxText(row.str(), BOX_WIDTH);
    }

    printBoxDivider(BOX_WIDTH);

    if (count == 0) {
        printBoxCentered("You have no prescriptions yet.", BOX_WIDTH);
    }
    else {
        printBoxText(
            "Total prescription lines: " + to_string(count),
            BOX_WIDTH
        );
    }

    printBoxBottom(BOX_WIDTH);
}

// ---- The patient's own payment history (read-only) ----
void viewMyPaymentHistory(const string& ic) {
    vector<Payment> pays = loadPaymentsFromFile();

    const int BOX_WIDTH = 64;

    cout << "\n";
    printBoxTop(BOX_WIDTH);

    {
        ostringstream head;
        head << left
             << setw(5)  << "ID"
             << setw(17) << "Service"
             << setw(13) << "Date"
             << setw(11) << "Status"
             << right << setw(14) << "Amount (RM)";
        printBoxText(head.str(), BOX_WIDTH);
    }

    printBoxDivider(BOX_WIDTH);

    int count = 0;
    double paid = 0.0;
    double owing = 0.0;

    for (const auto& p : pays) {
        if (p.patientID != ic)
            continue;

        count++;

        double amt = 0.0;
        try {
            amt = stod(p.amount);
        }
        catch (...) {
            amt = 0.0;
        }

        if (p.status == PAY_PAID)
            paid += amt;
        else if (p.status == PAY_UNPAID)
            owing += amt;

        ostringstream row;
        row << left
            << setw(5)  << fitText(p.paymentID, 4)
            << setw(17) << fitText(p.service.empty() ? "-" : p.service, 16)
            << setw(13) << fitText(p.date, 12)
            << setw(11) << fitText(p.status, 10)
            << right << setw(14) << fixed << setprecision(2) << amt;

        printBoxText(row.str(), BOX_WIDTH);
    }

    printBoxDivider(BOX_WIDTH);

    if (count == 0) {
        printBoxCentered("No payment records found.", BOX_WIDTH);
    }
    else {
        ostringstream paidRow;
        paidRow << left << setw(42) << "TOTAL PAID"
                << right << setw(16)
                << fixed << setprecision(2) << paid;
        printBoxText(paidRow.str(), BOX_WIDTH);

        ostringstream owingRow;
        owingRow << left << setw(42) << "STILL OUTSTANDING"
                 << right << setw(16)
                 << fixed << setprecision(2) << owing;
        printBoxText(owingRow.str(), BOX_WIDTH);
    }

    printBoxBottom(BOX_WIDTH);
}

// The logged-in patient's IC is passed in, so it is reused everywhere below.
void patientMenu(const string& ic) {
    int choice;
    bool loggedOut = false;

    while (!loggedOut) {
        clearScreen();
        printAppHeader("PATIENT PORTAL");
        cout << uiIndent() << "Logged in as: " << ic << "\n\n";
        printMenuHeader("Function");
        printMenuOption(1, "Request Appointment");
        printMenuOption(2, "View My Appointments");
        printMenuOption(3, "View My Profile");
        printMenuOption(4, "Update My Profile");
        printMenuOption(5, "Request Appointment Reschedule");
        printMenuOption(6, "View My Prescriptions");
        printMenuOption(7, "Make Payment");
        printMenuOption(8, "View Payment History");
        printMenuOption(9, "Print Receipt");
        printMenuOption(10, "Give Feedback");
        printMenuOption(11, "Deactivate My Account");
        printMenuOption(12, "Logout");
        printMenuBottom();
        printChoicePrompt();
        choice = readInt();

        if (choice == 12) break;
        if (choice < 1 || choice > 12) {
            printStatusMessage("INVALID CHOICE");
            waitForEnter();
            continue;
        }

        clearScreen();
        if (choice == 1) { printAppHeader("REQUEST APPOINTMENT"); makeAppointment(ic, STATUS_PENDING); }
        else if (choice == 2) { printAppHeader("MY APPOINTMENTS"); viewAppointments(ic); }
        else if (choice == 3) { printAppHeader("MY PROFILE"); viewMyProfile(ic); }
        else if (choice == 4) { printAppHeader("UPDATE PROFILE"); updateMyProfile(ic); }
        else if (choice == 5) { printAppHeader("RESCHEDULE REQUEST"); requestReschedule(ic); }
        else if (choice == 6) { printAppHeader("MY PRESCRIPTIONS"); viewMyPrescriptions(ic); }
        else if (choice == 7) { printAppHeader("MAKE PAYMENT"); makePayment(ic); }
        else if (choice == 8) { printAppHeader("PAYMENT HISTORY"); viewMyPaymentHistory(ic); }
        else if (choice == 9) { printAppHeader("PAYMENT RECEIPT"); printReceipt(ic); }
        else if (choice == 10) { printAppHeader("PATIENT FEEDBACK"); giveFeedback(ic); }
        else if (choice == 11) { printAppHeader("DEACTIVATE ACCOUNT"); deactivateMyAccount(ic, loggedOut); }

        if (!loggedOut) waitForEnter();
    }
}