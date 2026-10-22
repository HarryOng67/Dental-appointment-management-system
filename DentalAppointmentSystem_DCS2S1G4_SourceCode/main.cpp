#include "common.cpp"

#include "register.cpp"
#include "FileManager.cpp"
#include "staff.cpp"
#include "appointment.cpp"
#include "medicine.cpp"
#include "feedback.cpp"
#include "doctor.cpp"
#include "management.cpp"

void showIntroScreen() {
    clearScreen();
    printAppHeader("DENTAL CLINIC MANAGEMENT SYSTEM");
    cout << uiIndent() << "\n";    centerText("Patients  |  Appointments  |  Medicine  |  Payments  |  Reports");
    centerText("Operating Hours: 8:00 AM - 6:00 PM");
    cout << uiIndent() << "\n";    printLine('-');
    centerText("Welcome to Bright Smile Dental Clinic");
    centerText("Please log in or register to continue.");
    printLine('-');
}

void showExitScreen() {
    clearScreen();
    printAppHeader("GOODBYE");
    cout << uiIndent() << "\n";    centerText("Thank you for using Bright Smile Dental Clinic Management System.");
    centerText("Have a great day and keep smiling!");
    cout << uiIndent() << "\n";    printLine('=');
}

void showMainMenu() {
    printAppHeader("MAIN MENU");
    printMenuHeader("Option");
    printMenuOption(1, "Login");
    printMenuOption(2, "Register New Patient");
    printMenuOption(3, "Exit");
    printMenuBottom();
    printChoicePrompt();
}

void showLoginMenu() {
    printAppHeader("LOGIN PORTAL");
    printMenuHeader("User Role");
    printMenuOption(1, "Patient");
    printMenuOption(2, "Doctor");
    printMenuOption(3, "Staff - Front Counter");
    printMenuOption(4, "Management - Payment & Reporting");
    printMenuOption("0", "Back to Main Menu");
    printMenuBottom();
    printChoicePrompt();
}

int main() {
    showIntroScreen();
    waitForEnter();

    while (true) {
        clearScreen();
        showMainMenu();
        int option1 = readInt();

        if (option1 == 1) {
            clearScreen();
            showLoginMenu();
            int role = readInt();
            if (role == 0) continue;

            string id, password;
            clearScreen();

            switch (role) {
                case 1: {
                    printAppHeader("PATIENT LOGIN");
                    cout << "\n" << uiIndent() << "Patient ID (0 to cancel): ";
                    if (cancel(id)) break;
                    cout << uiIndent() << "Password   (0 to cancel): ";
                    if (cancel(password)) break;
                    if (loginPatient(id, password)) {
                        printStatusMessage("LOGIN SUCCESSFUL");
                        waitForEnter();
                        patientMenu(id);
                    } else {
                        printStatusMessage("LOGIN FAILED - PLEASE CHECK YOUR CREDENTIALS");
                        waitForEnter();
                    }
                    break;
                }
                case 2: {
                    printAppHeader("DOCTOR LOGIN");
                    cout << "\n" << uiIndent() << "Doctor ID (0 to cancel): ";
                    if (cancel(id)) break;
                    cout << uiIndent() << "Password  (0 to cancel): ";
                    if (cancel(password)) break;
                    if (loginDoctor(id, password)) {
                        printStatusMessage("LOGIN SUCCESSFUL");
                        waitForEnter();
                        doctorMenu();
                    } else {
                        printStatusMessage("LOGIN FAILED - PLEASE CHECK YOUR CREDENTIALS");
                        waitForEnter();
                    }
                    break;
                }
                case 3: {
                    printAppHeader("STAFF LOGIN");
                    cout << "\n" << uiIndent() << "Staff ID  (0 to cancel): ";
                    if (cancel(id)) break;
                    cout << uiIndent() << "Password  (0 to cancel): ";
                    if (cancel(password)) break;
                    if (loginStaff(id, password)) {
                        printStatusMessage("LOGIN SUCCESSFUL");
                        waitForEnter();
                        staffMenu();
                    } else {
                        printStatusMessage("LOGIN FAILED - PLEASE CHECK YOUR CREDENTIALS");
                        waitForEnter();
                    }
                    break;
                }
                case 4: {
                    printAppHeader("MANAGEMENT LOGIN");
                    cout << "\n" << uiIndent() << "Management ID (0 to cancel): ";
                    if (cancel(id)) break;
                    cout << uiIndent() << "Password      (0 to cancel): ";
                    if (cancel(password)) break;
                    if (loginManagement(id, password)) {
                        printStatusMessage("LOGIN SUCCESSFUL");
                        waitForEnter();
                        managementMenu();
                    } else {
                        printStatusMessage("LOGIN FAILED - PLEASE CHECK YOUR CREDENTIALS");
                        waitForEnter();
                    }
                    break;
                }
                default:
                    printStatusMessage("INVALID CHOICE - PLEASE SELECT 0 TO 4");
                    waitForEnter();
                    break;
            }
        }
        else if (option1 == 2) {
            clearScreen();
            printAppHeader("PATIENT REGISTRATION");
            cout << uiIndent() << "\n";            Patient p;
            if (registerPatient(p)) {
                savePatientToFile(p);
                printStatusMessage("PATIENT REGISTRATION SUCCESSFUL");
                cout << "\n" << uiIndent() << left << setw(20) << "Patient ID" << ": " << p.patientID << "\n";
                cout << uiIndent() << left << setw(20) << "Patient Name" << ": " << p.patientName << "\n";
            } else {
                printStatusMessage("REGISTRATION CANCELLED");
            }
            waitForEnter();
        }
        else if (option1 == 3) {
            showExitScreen();
            break;
        }
        else {
            printStatusMessage("INVALID CHOICE - PLEASE SELECT 1 TO 3");
            waitForEnter();
        }
    }
    return 0;
}
