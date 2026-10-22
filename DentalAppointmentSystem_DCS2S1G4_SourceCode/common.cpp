#pragma once
// Shared definitions for the whole program.
// Every .cpp file includes this, so the editor knows about the structs,
// the system headers, the cancel() helper, and all the functions.
// #pragma once makes sure it is only pasted in once when main.cpp is built.

#include <iostream>
#include <string>
#include <cctype>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <limits>   // needed by readInt() to skip bad input
#include <ctime>    // needed to work out a patient's age from today's date
#ifdef _WIN32
#include <conio.h>
#endif
using namespace std;

// ---- Shared structs ----
struct Patient {
    string patientName;
    string patientID;
    string patientContact;
    string patientEmail;
    string patientGender;
    string patientPassword;
    string status;          // "Active" or "Inactive" (deactivated accounts)
};

struct Appointment {
    string appointmentID;   // auto-generated number
    string patientID;       // IC of the patient who booked
    string date;            // DD/MM/YYYY
    string time;            // HH:MM
    string reason;          // reason for visit
    string status;          // Pending / Approved / Completed / Cancelled

    string rescheduleDate;  
    string rescheduleTime;
    string rescheduleStatus;
};

struct Medicine {
    string medicineID;      // auto-generated number
    string name;            // medicine name
    string quantity;        // how many in stock
    string price;           // price per unit
};

struct Payment {
    string paymentID;       // auto-generated number
    string patientID;       // IC of the patient who paid
    string amount;          // amount billed / paid (AFTER any age discount)
    string date;            // DD/MM/YYYY
    string service;         // which dental service was paid for
    string status;          // Unpaid (a bill) / Paid / Refunded
    string discount;        // how much was taken off for age (0.00 if none)
};

// One comment left by a patient about the service they received.
struct Feedback {
    string feedbackID;      // auto-generated number
    string patientID;       // IC of the patient who wrote it
    string rating;          // 1 to 5 stars
    string comment;         // what the patient wrote
    string date;            // DD/MM/YYYY
};

// ---- Named constants used across the modules ----
const string STATUS_PENDING   = "Pending";
const string STATUS_APPROVED  = "Approved";
const string STATUS_COMPLETED = "Completed";
const string STATUS_CANCELLED = "Cancelled";

const string RESCHEDULE_NONE = "None";
const string RESCHEDULE_PENDING = "Pending";
const string RESCHEDULE_APPROVED = "Approved";
const string RESCHEDULE_REJECTED = "Rejected";

const string ACCOUNT_ACTIVE   = "Active";
const string ACCOUNT_INACTIVE = "Inactive";

const string PAY_UNPAID       = "Unpaid";
const string PAY_PAID         = "Paid";
const string PAY_REFUNDED     = "Refunded";

const int LOW_STOCK_LEVEL     = 10;   // stock warning threshold

// ---- Age-based discount (young patients and senior citizens pay less) ----
const int    YOUTH_MAX_AGE       = 18;     // below this age -> youth discount
const int    SENIOR_MIN_AGE      = 60;     // this age and above -> senior discount
const double YOUTH_DISCOUNT_RATE = 0.10;   // 10% off
const double SENIOR_DISCOUNT_RATE= 0.20;   // 20% off

const int MIN_RATING = 1;    // feedback rating range
const int MAX_RATING = 5;

// One appointment can be given MANY medicines, and one medicine can be given in
// MANY appointments. A record cannot hold a list, so this struct is the "bridge"
// record that resolves that many-to-many relationship: each Prescription is ONE
// line linking ONE appointment to ONE medicine. Several lines with the same
// appointmentID make up the full prescription for that visit.
struct Prescription {
    string prescriptionID;  // auto-generated number
    string appointmentID;   // links to Appointment.appointmentID
    string medicineID;      // links to Medicine.medicineID
    string quantity;        // how many units given to the patient
    string dosage;          // how to take it, e.g. "3 times a day"
};

// ---- Shared helpers ----
// Reads one word into 'out'. Returns true if the user typed "0" (cancel/back).
inline bool cancel(string& out) {
    cin >> out;
    return out == "0";
}

// Reads a whole number from the user. If letters or symbols are typed, it
// clears the error state and asks again, so the menus can never crash or spin
// in an infinite loop on bad input. (Covers the "invalid menu choice" case.)
inline int readInt() {
    string input;
    int value;

    while (true) {

        getline(cin >> ws, input);

        stringstream ss(input);

        // Check whether input starts with an integer
        if (ss >> value) {

            string extra;

            // Make sure there is nothing extra after the number
            if (!(ss >> extra)) {
                return value;
            }
        }

        cout << "  Invalid input. Please enter ONE number only: ";
    }
}

// Reads a WHOLE line, so a sentence with spaces (e.g. a feedback comment) is
// captured. 'cin >> ws' first skips the newline left behind by readInt().
// Commas are swapped for semicolons so the line cannot break the CSV files.
inline string readLine() {
    string text;
    cin >> ws;
    getline(cin, text);
    for (char& c : text) {
        if (c == ',') c = ';';
    }
    return text;
}


// ---- Professional console UI helpers ----
// 80-column panel with a small margin. All modules reuse these helpers so the
// interface stays consistent throughout the program.
const int UI_WIDTH = 80;
const int UI_MARGIN = 8;
const int INNER_BOX_WIDTH = 60;

inline string uiIndent() { return string(UI_MARGIN, ' '); }
inline string innerIndent() { return uiIndent() + string((UI_WIDTH - INNER_BOX_WIDTH) / 2, ' '); }

inline void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    cout << "\033[2J\033[H";
#endif
}

inline string fitText(string text, int width) {
    if ((int)text.length() > width) text = text.substr(0, width);
    return text;
}

inline void printLine(char ch = '-') {
    cout << uiIndent() << string(UI_WIDTH, ch) << "\n";
}

inline void centerText(const string& text) {
    int spaces = (UI_WIDTH - static_cast<int>(text.length())) / 2;
    if (spaces < 0) spaces = 0;
    cout << uiIndent() << string(spaces, ' ') << text << "\n";
}

inline void printWideTop() { cout << uiIndent() << " " << string(UI_WIDTH - 2, '_') << "\n"; }
inline void printWideBottom() { cout << uiIndent() << "|" << string(UI_WIDTH - 2, '_') << "|\n"; }
inline void printWideBlank() { cout << uiIndent() << "|" << string(UI_WIDTH - 2, ' ') << "|\n"; }

inline void printWideCentered(const string& text) {
    string value = fitText(text, UI_WIDTH - 4);
    int inside = UI_WIDTH - 2;
    int l = (inside - (int)value.length()) / 2;
    int r = inside - (int)value.length() - l;
    cout << uiIndent() << "|" << string(l, ' ') << value << string(r, ' ') << "|\n";
}

inline void printAppHeader(const string& pageTitle) {
    printWideTop();
    printWideBlank();
    printWideCentered("BRIGHT SMILE DENTAL CLINIC");
    printWideCentered(pageTitle);
    printWideBlank();
    printWideBottom();
}

inline void printInnerTop() { cout << innerIndent() << " " << string(INNER_BOX_WIDTH - 2, '_') << "\n"; }
inline void printInnerBottom() { cout << innerIndent() << "|" << string(INNER_BOX_WIDTH - 2, '_') << "|\n"; }
inline void printInnerDivider() { cout << innerIndent() << "|" << string(INNER_BOX_WIDTH - 2, '_') << "|\n"; }

inline void printInnerText(const string& text) {
    string value = fitText(text, INNER_BOX_WIDTH - 4);
    cout << innerIndent() << "| " << left << setw(INNER_BOX_WIDTH - 4) << value << " |\n";
}

inline void printInnerCentered(const string& text) {
    string value = fitText(text, INNER_BOX_WIDTH - 4);
    int inside = INNER_BOX_WIDTH - 2;
    int l = (inside - (int)value.length()) / 2;
    int r = inside - (int)value.length() - l;
    cout << innerIndent() << "|" << string(l, ' ') << value << string(r, ' ') << "|\n";
}

inline void printMenuHeader(const string& title = "Option") {
    cout << "\n";
    printInnerTop();
    printInnerCentered(title);
    printInnerDivider();
}

inline void printMenuOption(int number, const string& text) {
    ostringstream row; row << right << setw(2) << number << "   " << text;
    printInnerText(row.str());
}

inline void printMenuOption(const string& number, const string& text) {
    ostringstream row; row << right << setw(2) << number << "   " << text;
    printInnerText(row.str());
}

inline void printMenuBottom() { printInnerBottom(); }

inline void printChoicePrompt() {
    cout << "\n";
    centerText("Enter your choice");
    cout << innerIndent() << string((INNER_BOX_WIDTH - 4) / 2, ' ') << "> ";
}

inline void printSectionTitle(const string& title) {
    cout << "\n"; printInnerTop(); printInnerCentered(title); printInnerBottom();
}

inline void printStatusMessage(const string& message) {
    cout << "\n"; printInnerTop(); printInnerCentered(message); printInnerBottom();
}

inline void printCenteredField(const string& label, const string& value) {
    ostringstream row; row << left << setw(14) << label << ": " << value;
    printInnerText(row.str());
}

inline void printCenteredBlockTop() { printInnerTop(); }
inline void printCenteredBlockBottom() { printInnerBottom(); }


inline void printInfoRow(const string& label, const string& value) {
    const int labelWidth = 16;
    const int valueWidth = 34;

    cout << innerIndent()
         << "|   "
         << left << setw(labelWidth) << fitText(label, labelWidth)
         << ": "
         << left << setw(valueWidth) << fitText(value, valueWidth)
         << "   |\n";
}

inline void printReportBoxTop() {
    cout << innerIndent()
         << " " << string(INNER_BOX_WIDTH - 2, '_') << "\n";
}

inline void printReportBoxBottom() {
    cout << innerIndent()
         << "|" << string(INNER_BOX_WIDTH - 2, '_') << "|\n";
}

inline void printReportBlank() {
    cout << innerIndent()
         << "|" << string(INNER_BOX_WIDTH - 2, ' ') << "|\n";
}

inline void printReportTitle(const string& title) {
    const int insideWidth = INNER_BOX_WIDTH - 2;
    string text = fitText(title, insideWidth - 2);

    int leftSpace = (insideWidth - static_cast<int>(text.length())) / 2;
    int rightSpace = insideWidth - static_cast<int>(text.length()) - leftSpace;

    cout << innerIndent()
         << "|"
         << string(leftSpace, ' ')
         << text
         << string(rightSpace, ' ')
         << "|\n";
}

inline void printReportRow(const string& label, const string& value) {
    const int contentWidth = INNER_BOX_WIDTH - 2;
    const int leftPadding = 3;
    const int rightPadding = 3;
    const int labelWidth = 31;
    const int valueWidth =
        contentWidth - leftPadding - rightPadding - labelWidth;

    cout << innerIndent()
         << "|"
         << string(leftPadding, ' ')
         << left << setw(labelWidth) << fitText(label, labelWidth)
         << right << setw(valueWidth) << fitText(value, valueWidth)
         << string(rightPadding, ' ')
         << "|\n";
}

inline void printReportRow(const string& label, int value) {
    printReportRow(label, to_string(value));
}

inline void printReportMoney(const string& label, double value) {
    ostringstream amount;
    amount << "RM " << fixed << setprecision(2) << value;
    printReportRow(label, amount.str());
}


inline string boxIndent(int width) {
    int extra = (UI_WIDTH - width) / 2;
    if (extra < 0) extra = 0;
    return uiIndent() + string(extra, ' ');
}

inline void printBoxTop(int width) {
    cout << boxIndent(width) << " " << string(width - 2, '_') << "\n";
}

inline void printBoxBottom(int width) {
    cout << boxIndent(width) << "|" << string(width - 2, '_') << "|\n";
}

inline void printBoxBlank(int width) {
    cout << boxIndent(width) << "|" << string(width - 2, ' ') << "|\n";
}

inline void printBoxCentered(const string& text, int width) {
    string value = fitText(text, width - 4);
    int inside = width - 2;
    int l = (inside - static_cast<int>(value.length())) / 2;
    int r = inside - static_cast<int>(value.length()) - l;

    cout << boxIndent(width)
         << "|" << string(l, ' ')
         << value
         << string(r, ' ') << "|\n";
}

inline void printBoxText(const string& text, int width) {
    string value = fitText(text, width - 4);
    cout << boxIndent(width)
         << "| "
         << left << setw(width - 4) << value
         << " |\n";
}

inline void printBoxDivider(int width) {
    cout << boxIndent(width) << "|"
         << string(width - 2, '-') << "|\n";
}

inline void waitForEnter() {
    cout << "\n"; centerText("Press ENTER to continue...");
#ifdef _WIN32
    int ch;
    do { ch = _getch(); } while (ch != 13 && ch != 10);
    cout << "\n";
#else
    string dummy;
    getline(cin, dummy);
#endif
}

// ---- Function declarations (the real code is in the other .cpp files) ----
// register.cpp
bool isValidIC(const string& ic);
bool isValidName(const string& name);
bool isValidContact(const string& contact);
bool isValidEmail(const string& email);
bool isValidPassword(const string& pw);
bool icAlreadyUsed(const string& ic);
bool registerPatient(Patient& p);

// FileManager.cpp
void savePatientToFile(const Patient& p);
void saveAllPatients(const vector<Patient>& patients);
vector<Patient> loadPatientsFromFile();
bool loginPatient(const string& id, const string& password);
bool loginDoctor(const string& id, const string& password);
bool loginStaff(const string& id, const string& password);
bool loginManagement(const string& id, const string& password);

// staff.cpp  (the admin assistant at the front counter)
void viewAllPatients();
void updatePatient();
void deletePatient();
void searchPatient();   // extra function: find a patient by IC or name
void reactivatePatient();
void patientRecordMenu();
bool askExistingPatientIC(string& ic);
void counterBookAppointment();       // C: create an appointment for a patient
void approveAppointment();           // U: Pending -> Approved
void counterUpdateStatus();          // U: set any appointment status
void counterCancelAppointment();     // D: cancel an appointment
void staffMenu();
void manageRescheduleRequests();

// appointment.cpp
void saveAppointmentToFile(const Appointment& a);
vector<Appointment> loadAppointmentsFromFile();
void saveAllAppointments(const vector<Appointment>& appts);
bool isValidDate(const string& d);
bool isValidTime(const string& t);
bool isWithinOperatingHours(const string& t);
bool patientExists(const string& ic);
int nextAppointmentId();
string chooseReason();
string chooseStatus();                                    // status picker menu
bool setAppointmentStatus(const string& id, const string& newStatus);
void listAppointmentsByStatus(const string& wanted);      // "" or "ALL" = all
void makeAppointment(const string& ic, const string& status);
void viewAppointments(const string& ic);
void updateAppointment();
void deleteAppointment();
void savePaymentToFile(const Payment& pay);
vector<Payment> loadPaymentsFromFile();
void saveAllPayments(const vector<Payment>& pays);
int nextPaymentId();
bool isValidAmount(const string& s);
string chooseService(double& priceOut);   // service price list (fixed prices)
void requestReschedule(const string& patientID);


// ---- Age-based discount ----
int getAgeFromIC(const string& ic);       // birth date is the first 6 digits of the IC
double discountRateForAge(int age);       // 0.10 below 18, 0.20 at 60 and above
double applyAgeDiscount(const string& ic, double price, double& discountOut);
bool settleOutstandingBill(const string& ic);
void makePayment(const string& ic);
void printReceipt(const string& ic);   // extra function: formatted payment receipt
// ---- Patient module (profile + own records) ----
void viewMyProfile(const string& ic);
void updateMyProfile(const string& ic);
void deactivateMyAccount(const string& ic, bool& loggedOut);
void viewMyPrescriptions(const string& ic);
void viewMyPaymentHistory(const string& ic);
void patientMenu(const string& ic);

// feedback.cpp
void saveFeedbackToFile(const Feedback& f);
vector<Feedback> loadFeedbackFromFile();
int nextFeedbackId();
void giveFeedback(const string& ic);   // C: the patient writes a comment
void viewAllFeedback();                // R: management reads the comments

// medicine.cpp
void saveMedicineToFile(const Medicine& m);
vector<Medicine> loadMedicinesFromFile();
void saveAllMedicines(const vector<Medicine>& meds);
bool isNumber(const string& s);
int nextMedicineId();
void addMedicine();
void viewStock();
void updateStock();
void deleteMedicine();
void medicineMenu();

// doctor.cpp
void savePrescriptionToFile(const Prescription& pr);
vector<Prescription> loadPrescriptionsFromFile();
int nextPrescriptionId();
bool appointmentExists(const string& id);
int findMedicineIndex(const vector<Medicine>& meds, const string& id);
string chooseDosage();
int findPrescriptionIndex(const vector<Prescription>& list, const string& id);
void saveAllPrescriptions(const vector<Prescription>& list);
void viewAllAppointments();          // R: the doctor's appointment schedule
void createPrescription();           // C
void listPrescriptions(const string& key);   // prints the table (no prompt)
void viewPrescriptions();            // R
void updatePrescription();           // U: change quantity / dosage
void doctorUpdateStatus();           // U: mark an appointment Completed etc.
void cancelConsultation();           // D
void viewPatientHistory();   // one patient's visits + the medicines given
void doctorMenu();

// management.cpp  (Payment & Reporting)
int findPaymentIndex(const vector<Payment>& pays, const string& id);
void generateBill();                 // C: raise an unpaid bill for a patient
void generateInvoice();              // C: print a formal invoice for one bill
void viewReceipt();                  // R: receipt of what a patient has paid
void viewPaymentHistory();           // R: payments by IC or ALL
void updatePaymentDetails();         // U: correct the service / date
void processRefund();                // D: refund a paid bill, cancel an unpaid one
void viewReport();
void managementMenu();