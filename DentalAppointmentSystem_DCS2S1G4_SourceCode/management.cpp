// This file is #included by main.cpp (it does not compile on its own).
// Payment & Reporting module (Management login):
//   C - generate a bill, generate an invoice
//   R - view receipt, view payment history
//   U - update payment details
//   D - process a refund / cancel a payment
// plus the overall business report.
#include "common.cpp"   // structs, headers, and declarations

// Returns the position of a payment in the list, or -1 if the ID is not there.
// Returning an index keeps the search separate from the input/output and lets
// the caller edit that payment record directly.
int findPaymentIndex(const vector<Payment>& pays, const string& id) {
    for (int i = 0; i < (int)pays.size(); i++) {
        if (pays[i].paymentID == id) return i;
    }
    return -1;
}

// ---- C: raise a bill for treatment a patient has received ----
// The bill is a Payment record with status "Unpaid" until the patient settles it.
void generateBill() {
    string ic;
    cout << uiIndent() << "Enter patient IC to bill (0 to cancel): ";    if (cancel(ic)) return;

    if (!patientExists(ic)) {          // reuses the check from appointment.cpp
        cout << uiIndent() << "No patient found with IC " << ic << ".\n";        return;
    }

    Payment bill;
    bill.paymentID = to_string(nextPaymentId());
    bill.patientID = ic;

    // the price comes from the fixed service list, never typed by hand
    double price = 0;
    bill.service = chooseService(price);
    if (bill.service == "0") return;

    // the same age discount the patient would get paying at the counter
    double discount = 0;
    double payable = applyAgeDiscount(ic, price, discount);

    ostringstream amt, dis;
    amt << fixed << setprecision(2) << payable;
    dis << fixed << setprecision(2) << discount;
    bill.amount   = amt.str();
    bill.discount = dis.str();

    while (true) {
        cout << uiIndent() << "Bill date (DD/MM/YYYY) (0 to cancel): ";        if (cancel(bill.date)) return;
        if (isValidDate(bill.date)) break;
        cout << uiIndent() << "  Invalid date. Use DD/MM/YYYY\n";    }

    bill.status = PAY_UNPAID;
    savePaymentToFile(bill);

    cout << "\n" << uiIndent() << "Bill generated successfully.\n";    ostringstream msg;
    msg << "Bill ID: " << bill.paymentID
        << " | " << bill.service
        << " | RM " << bill.amount
        << " | Status: " << bill.status;
    cout << uiIndent() << msg.str() << "\n";}

// ---- C: print a formal invoice for one bill or payment ----
void generateInvoice() {
    string paymentID;

    cout << "\n";
    centerText("Enter bill / payment ID (0 to cancel)");
    cout << boxIndent(64) << "> ";

    if (cancel(paymentID))
        return;

    vector<Payment> pays = loadPaymentsFromFile();
    vector<Patient> patients = loadPatientsFromFile();

    Payment selected;
    bool found = false;

    for (const auto& p : pays) {
        if (p.paymentID == paymentID) {
            selected = p;
            found = true;
            break;
        }
    }

    if (!found) {
        printStatusMessage("Payment record not found.");
        return;
    }

    string patientName = selected.patientID;

    for (const auto& p : patients) {
        if (p.patientID == selected.patientID) {
            patientName = p.patientName;
            break;
        }
    }

    double amount = 0.0;

    try {
        amount = stod(selected.amount);
    }
    catch (...) {
        amount = 0.0;
    }

    const int BOX_WIDTH = 64;

    cout << "\n";
    printBoxTop(BOX_WIDTH);

    printBoxText("Invoice No  : INV-" + selected.paymentID, BOX_WIDTH);
    printBoxText("Date        : " + selected.date, BOX_WIDTH);
    printBoxText("Patient     : " + patientName, BOX_WIDTH);
    printBoxText("IC          : " + selected.patientID, BOX_WIDTH);

    printBoxDivider(BOX_WIDTH);

    {
        ostringstream head;
        head << left << setw(44) << "Description"
             << right << setw(16) << "Amount (RM)";
        printBoxText(head.str(), BOX_WIDTH);
    }

    printBoxDivider(BOX_WIDTH);

    {
        ostringstream item;
        item << left << setw(44)
             << fitText(selected.service.empty() ? "-" : selected.service, 43)
             << right << setw(16)
             << fixed << setprecision(2) << amount;
        printBoxText(item.str(), BOX_WIDTH);
    }

    printBoxDivider(BOX_WIDTH);

    {
        ostringstream totalRow;
        totalRow << left << setw(44) << "TOTAL"
                 << right << setw(16)
                 << fixed << setprecision(2) << amount;
        printBoxText(totalRow.str(), BOX_WIDTH);
    }

    printBoxBlank(BOX_WIDTH);
    printBoxText("Payment status: " + selected.status, BOX_WIDTH);

    printBoxBottom(BOX_WIDTH);
}

// ---- R: the receipt of everything one patient has paid ----
void viewReceipt() {
    string ic;
    cout << uiIndent() << "Enter patient IC (0 to cancel): ";    if (cancel(ic)) return;

    if (!patientExists(ic)) {
        cout << uiIndent() << "No patient found with IC " << ic << ".\n";        return;
    }
    printReceipt(ic);   // reuses the patient-side receipt, no duplicated code
}

// ---- R: payment records for one patient, or for everyone ----
void viewPaymentHistory() {
    string ic;

    cout << "\n";
    centerText("Enter patient IC (ALL for everyone, 0 to cancel)");
    cout << boxIndent(72) << "> ";

    if (cancel(ic))
        return;

    vector<Payment> pays = loadPaymentsFromFile();

    if (pays.empty()) {
        printStatusMessage("No payments recorded yet.");
        return;
    }

    const int BOX_WIDTH = 78;

    cout << "\n";
    printBoxTop(BOX_WIDTH);

    {
        ostringstream head;
        head << left
             << setw(5)  << "ID"
             << setw(16) << "Patient IC"
             << setw(17) << "Service"
             << setw(13) << "Date"
             << setw(11) << "Status"
             << right << setw(12) << "Amount (RM)";
        printBoxText(head.str(), BOX_WIDTH);
    }

    printBoxDivider(BOX_WIDTH);

    int shown = 0;
    double paid = 0.0;
    double owing = 0.0;
    double refunded = 0.0;

    for (const auto& p : pays) {
        if (ic != "ALL" && p.patientID != ic)
            continue;

        shown++;

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
        else if (p.status == PAY_REFUNDED)
            refunded += amt;

        ostringstream row;
        row << left
            << setw(5)  << fitText(p.paymentID, 4)
            << setw(16) << fitText(p.patientID, 15)
            << setw(17) << fitText(p.service.empty() ? "-" : p.service, 16)
            << setw(13) << fitText(p.date, 12)
            << setw(11) << fitText(p.status, 10)
            << right << setw(12)
            << fixed << setprecision(2) << amt;

        printBoxText(row.str(), BOX_WIDTH);
    }

    printBoxDivider(BOX_WIDTH);

    if (shown == 0) {
        printBoxCentered("No payments found for that IC.", BOX_WIDTH);
    }
    else {
        ostringstream paidRow;
        paidRow << left << setw(58) << "TOTAL PAID"
                << right << setw(16)
                << fixed << setprecision(2) << paid;
        printBoxText(paidRow.str(), BOX_WIDTH);

        ostringstream owingRow;
        owingRow << left << setw(58) << "OUTSTANDING"
                 << right << setw(16)
                 << fixed << setprecision(2) << owing;
        printBoxText(owingRow.str(), BOX_WIDTH);

        ostringstream refundRow;
        refundRow << left << setw(58) << "REFUNDED"
                  << right << setw(16)
                  << fixed << setprecision(2) << refunded;
        printBoxText(refundRow.str(), BOX_WIDTH);
    }

    printBoxBottom(BOX_WIDTH);
}

// ---- U: correct the service or the date on a payment record ----
void updatePaymentDetails() {
    string id;
    cout << uiIndent() << "Enter bill / payment ID to update (0 to cancel): ";    if (cancel(id)) return;

    vector<Payment> pays = loadPaymentsFromFile();
    int index = findPaymentIndex(pays, id);
    if (index == -1) {
        cout << uiIndent() << "No bill or payment found with ID " << id << ".\n";        return;
    }

    if (pays[index].status == PAY_REFUNDED) {
        cout << uiIndent() << "This payment has been refunded and can no longer be changed.\n";        return;
    }

    ostringstream current;
    current << "Current: " << pays[index].service
            << " | RM " << pays[index].amount
            << " | " << pays[index].date
            << " | " << pays[index].status;
    cout << uiIndent() << current.str() << "\n";
    // re-pick the service so the amount always matches the price list
    double price = 0;
    string service = chooseService(price);
    if (service == "0") return;

    // the age discount is worked out again for the new service
    double discount = 0;
    double payable = applyAgeDiscount(pays[index].patientID, price, discount);

    string date;
    while (true) {
        cout << uiIndent() << "New date (DD/MM/YYYY) (0 to cancel): ";        if (cancel(date)) return;
        if (isValidDate(date)) break;
        cout << uiIndent() << "  Invalid date. Use DD/MM/YYYY\n";    }

    ostringstream amt, dis;
    amt << fixed << setprecision(2) << payable;
    dis << fixed << setprecision(2) << discount;

    pays[index].service  = service;
    pays[index].amount   = amt.str();
    pays[index].discount = dis.str();
    pays[index].date     = date;
    saveAllPayments(pays);

    cout << uiIndent() << "Payment " << id << " updated: " << service         << " | RM " << pays[index].amount << " | " << date << "\n";
}

// ---- D: refund a payment, or cancel a bill that was never paid ----
void processRefund() {
    string id;
    cout << uiIndent() << "Enter bill / payment ID to refund or cancel (0 to cancel): ";    if (cancel(id)) return;

    vector<Payment> pays = loadPaymentsFromFile();
    int index = findPaymentIndex(pays, id);
    if (index == -1) {
        cout << uiIndent() << "No bill or payment found with ID " << id << ".\n";        return;
    }

    if (pays[index].status == PAY_REFUNDED) {
        cout << uiIndent() << "Payment " << id << " has already been refunded.\n";        return;
    }

    ostringstream detail;
    detail << "Record " << id << ": " << pays[index].service
           << " | RM " << pays[index].amount
           << " | Status: " << pays[index].status;
    cout << uiIndent() << detail.str() << "\n";    cout << uiIndent() << "Type YES to confirm (anything else cancels): ";    string answer;
    cin >> answer;
    if (answer != "YES") {
        cout << uiIndent() << "No change made.\n";        return;
    }

    // A bill that was never paid is simply cancelled; money already received
    // is marked as refunded so it drops out of the revenue figures.
    if (pays[index].status == PAY_UNPAID) {
        pays[index].status = STATUS_CANCELLED;
        saveAllPayments(pays);
        cout << uiIndent() << "Unpaid bill " << id << " has been cancelled.\n";    } else {
        pays[index].status = PAY_REFUNDED;
        saveAllPayments(pays);
        cout << uiIndent() << "Payment " << id << " has been refunded (RM "             << pays[index].amount << ").\n";
    }
}

// ---- The overall business report ----
void viewReport() {
    vector<Patient> patients      = loadPatientsFromFile();
    vector<Appointment> appts     = loadAppointmentsFromFile();
    vector<Medicine> meds         = loadMedicinesFromFile();
    vector<Payment> pays          = loadPaymentsFromFile();
    vector<Prescription> scripts  = loadPrescriptionsFromFile();

    // count patients by account status
    int active = 0, inactive = 0;
    for (const auto& p : patients) {
        if (p.status == ACCOUNT_INACTIVE) inactive++;
        else                              active++;
    }

    // count appointments by status (accumulator pattern)
    int pending = 0, approved = 0, completed = 0, cancelled = 0;
    for (const auto& a : appts) {
        if (a.status == STATUS_PENDING)        pending++;
        else if (a.status == STATUS_APPROVED)  approved++;
        else if (a.status == STATUS_COMPLETED) completed++;
        else if (a.status == STATUS_CANCELLED) cancelled++;
    }

    // only money actually received counts as revenue
    double revenue = 0, outstanding = 0, refunded = 0, discounts = 0;
    int paidCount = 0, discountCount = 0;
    for (const auto& p : pays) {
        double amt = 0, dis = 0;
        try { amt = stod(p.amount); } catch (...) { amt = 0; }
        try { dis = stod(p.discount); } catch (...) { dis = 0; }
        if (p.status == PAY_PAID)          { revenue += amt; paidCount++; }
        else if (p.status == PAY_UNPAID)   outstanding += amt;
        else if (p.status == PAY_REFUNDED) refunded += amt;
        if (dis > 0) { discounts += dis; discountCount++; }
    }

    // patient feedback summary
    vector<Feedback> feedback = loadFeedbackFromFile();
    int ratingTotal = 0;
    for (const auto& f : feedback) {
        try { ratingTotal += stoi(f.rating); } catch (...) {}
    }

    // medicines that need reordering
    int lowStock = 0;
    for (const auto& m : meds) {
        if (isNumber(m.quantity) && stoi(m.quantity) < LOW_STOCK_LEVEL) lowStock++;
    }

    // =========================
    // DISPLAY REPORT
    // =========================

    cout << "\n";

    printReportBoxTop();
    printReportBlank();

    printReportTitle("PATIENT OVERVIEW");
    printReportBlank();
    printReportRow("Registered Patients", static_cast<int>(patients.size()));
    printReportRow("Active Accounts", active);
    printReportRow("Deactivated Accounts", inactive);
    printReportBlank();

    printReportTitle("APPOINTMENT OVERVIEW");
    printReportBlank();
    printReportRow("Total Appointments", static_cast<int>(appts.size()));
    printReportRow("Pending Approval", pending);
    printReportRow("Approved", approved);
    printReportRow("Completed", completed);
    printReportRow("Cancelled", cancelled);
    printReportBlank();

    printReportTitle("MEDICINE & PRESCRIPTION");
    printReportBlank();
    printReportRow("Prescription Lines", static_cast<int>(scripts.size()));
    printReportRow("Medicines in System", static_cast<int>(meds.size()));
    printReportRow("Low Stock (< 10)", lowStock);
    printReportBlank();

    printReportTitle("PAYMENT OVERVIEW");
    printReportBlank();
    printReportRow("Payment Records", static_cast<int>(pays.size()));
    printReportRow("Settled Payments", paidCount);
    printReportRow("With Age Discount", discountCount);
    printReportBlank();

    printReportTitle("FINANCIAL SUMMARY");
    printReportBlank();
    printReportMoney("Total Revenue", revenue);
    printReportMoney("Outstanding Bills", outstanding);
    printReportMoney("Refunded Amount", refunded);
    printReportMoney("Age Discounts Given", discounts);

    if (paidCount > 0) {
        printReportMoney("Average Per Payment", revenue / paidCount);
    } else {
        printReportRow("Average Per Payment", "N/A");
    }

    printReportBlank();

    printReportTitle("CUSTOMER FEEDBACK");
    printReportBlank();
    printReportRow("Feedback Received", static_cast<int>(feedback.size()));

    if (!feedback.empty()) {
        double averageRating =
            static_cast<double>(ratingTotal) / feedback.size();

        ostringstream rating;
        rating << fixed << setprecision(2)
               << averageRating << " / 5.00";

        printReportRow("Average Rating", rating.str());
    } else {
        printReportRow("Average Rating", "N/A");
    }

    printReportBlank();
    printReportBoxBottom();
}

void managementMenu() {
    int choice;
    while (true) {
        clearScreen();
        printAppHeader("MANAGEMENT - PAYMENT & REPORTING");
        printMenuHeader("Function");
        printMenuOption(1, "Generate Bill");
        printMenuOption(2, "Generate Invoice");
        printMenuOption(3, "View Receipt");
        printMenuOption(4, "View Payment History");
        printMenuOption(5, "Update Payment Details");
        printMenuOption(6, "Process Refund / Cancel Payment");
        printMenuOption(7, "View Patient Feedback");
        printMenuOption(8, "View Business Report");
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
        clearScreen();
        if (choice == 1) { printAppHeader("GENERATE BILL"); generateBill(); }
        else if (choice == 2) { printAppHeader("GENERATE INVOICE"); generateInvoice(); }
        else if (choice == 3) { printAppHeader("VIEW RECEIPT"); viewReceipt(); }
        else if (choice == 4) { printAppHeader("PAYMENT HISTORY"); viewPaymentHistory(); }
        else if (choice == 5) { printAppHeader("UPDATE PAYMENT DETAILS"); updatePaymentDetails(); }
        else if (choice == 6) { printAppHeader("REFUND / CANCEL PAYMENT"); processRefund(); }
        else if (choice == 7) { printAppHeader("PATIENT FEEDBACK"); viewAllFeedback(); }
        else if (choice == 8) { printAppHeader("BUSINESS REPORT"); viewReport(); }
        waitForEnter();
    }
}