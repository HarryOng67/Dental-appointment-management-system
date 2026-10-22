// This file is #included by main.cpp (it does not compile on its own).
#include "common.cpp"   // structs, headers, and declarations

// IC must be exactly 12 digits, e.g. 000000000000
bool isValidIC(const string& ic) {
    if (ic.length() != 12) return false;
    for (char c : ic) {
        if (!isdigit(c)) return false;   // every character must be a number
    }
    return true;
}

// Name must be letters only (no digits/symbols); e.g. "Alice" is valid, "A1ice" is not.
bool isValidName(const string& name) {
    if (name.empty()) return false;
    for (char c : name) {
        if (!isalpha(c)) return false;   // reject digits and symbols
    }
    return true;
}

bool isValidContact(const string& contact) {
    if (contact.empty()) {
        return false;
    }

    for (char c : contact) {
        if (!isdigit(c)) {
            return false;
        }
    }

    return true;
}

// True if this IC is already registered in patients.txt (used to block duplicates)
bool icAlreadyUsed(const string& ic) {
    vector<Patient> patients = loadPatientsFromFile();   // from FileManager.cpp
    for (const auto& p : patients) {
        if (p.patientID == ic) return true;
    }
    return false;
}

// Email must look like something@gmail.com (at least 1 char before @gmail.com)
bool isValidEmail(const string& email) {
    const string suffix = "@gmail.com";
    if (email.length() <= suffix.length()) return false;   // need a name before @gmail.com
    // check the ending matches "@gmail.com"
    return email.compare(email.length() - suffix.length(), suffix.length(), suffix) == 0;
}

// Password must have: 1 uppercase, 1 lowercase, 1 digit, 1 special char, min 8 chars
bool isValidPassword(const string& pw) {
    if (pw.length() < 8) return false;

    bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
    for (char c : pw) {
        if (isupper(c))      hasUpper = true;
        else if (islower(c)) hasLower = true;
        else if (isdigit(c)) hasDigit = true;
        else                 hasSpecial = true;   // anything else counts as special
    }
    return hasUpper && hasLower && hasDigit && hasSpecial;
}

// Fills in 'p'. Returns true on success, or false if the user typed 0 to cancel.
bool registerPatient(Patient& p) {
    cout << uiIndent() << "\n";    cout << uiIndent() << "Register Page (type 0 at any prompt to cancel)" << "\n";
    // Keep asking until the name is letters only (no digits)
    while (true) {
        cout << uiIndent() << "Name: ";        if (cancel(p.patientName)) return false;
        if (isValidName(p.patientName)) break;
        cout << uiIndent() << "  Invalid name. Use letters only (no digits or symbols)." << "\n";    }

    // Keep asking until the IC is 12 digits AND not already registered
    while (true) {
        cout << uiIndent() << "IC (12 digits, e.g. 000000000000): ";        if (cancel(p.patientID)) return false;
        if (!isValidIC(p.patientID)) {
            cout << uiIndent() << "  Invalid IC. It must be exactly 12 digits." << "\n";            continue;
        }
        if (icAlreadyUsed(p.patientID)) {
            cout << uiIndent() << "  This IC is already registered. Please log in instead." << "\n";            return false;   // duplicate ID -> cancel registration
        }
        break;
    }

    while (true) {
    cout << uiIndent() << "Contact Number (digits only, 0 to cancel): ";
    if (cancel(p.patientContact)) {
        return false;
    }if (isValidContact(p.patientContact)) {
        break;
    }   
        cout << uiIndent() << "  Invalid contact number. Digits only are allowed.\n";    }

    // Keep asking until the email ends with @gmail.com
    while (true) {
        cout << uiIndent() << "Email (must be name@gmail.com): ";        if (cancel(p.patientEmail)) return false;
        if (isValidEmail(p.patientEmail)) break;
        cout << uiIndent() << "  Invalid email. It must be like name@gmail.com" << "\n";    }

    while (true) {
        cout << uiIndent() << "Gender (Male/Female): ";

    if (cancel(p.patientGender))
        return false;

    if (p.patientGender == "Male" ||
        p.patientGender == "Female") {
        break;
    }

    cout << uiIndent()
         << "  Invalid gender. Please enter Male or Female only.\n";
    }

    // Keep asking until the password is strong enough
    while (true) {
        cout << uiIndent() << "Password (min 8 chars, 1 upper, 1 lower, 1 number, 1 special): ";        if (cancel(p.patientPassword)) return false;
        if (isValidPassword(p.patientPassword)) break;
        cout << uiIndent() << "  Weak password. Needs an uppercase, a lowercase, a number, and a special character." << "\n";    }

    p.status = ACCOUNT_ACTIVE;   // every new account starts active
    return true;
}