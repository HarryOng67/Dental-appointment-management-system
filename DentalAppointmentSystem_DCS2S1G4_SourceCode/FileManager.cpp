// This file is #included by main.cpp (it does not compile on its own).
#include "common.cpp"   // structs, headers, and declarations

void savePatientToFile(const Patient& p) {
    ofstream file("patients.txt", ios::app);
    // status is written LAST so older 6-field rows still load correctly
    file << p.patientName << ","
         << p.patientID << ","
         << p.patientContact << ","
         << p.patientEmail << ","
         << p.patientGender << ","
         << p.patientPassword << ","
         << p.status << "\n";
    file.close();
    cout << uiIndent() << "Saved permanently to patients.txt" << "\n";}

// Overwrites patients.txt with the whole list (used by update/delete/deactivate)
void saveAllPatients(const vector<Patient>& patients) {
    ofstream file("patients.txt", ios::trunc);   // trunc = wipe and rewrite
    for (const auto& p : patients) {
        file << p.patientName << ","
             << p.patientID << ","
             << p.patientContact << ","
             << p.patientEmail << ","
             << p.patientGender << ","
             << p.patientPassword << ","
             << p.status << "\n";
    }
    file.close();
}

vector<Patient> loadPatientsFromFile() {
    vector<Patient> patients;
    ifstream file("patients.txt");
    string line;

    while (getline(file, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        Patient p;
        getline(ss, p.patientName, ',');
        getline(ss, p.patientID, ',');
        getline(ss, p.patientContact, ',');
        getline(ss, p.patientEmail, ',');
        getline(ss, p.patientGender, ',');
        getline(ss, p.patientPassword, ',');
        getline(ss, p.status, ',');
        // rows saved before accounts could be deactivated have no status yet
        if (p.status.empty()) p.status = ACCOUNT_ACTIVE;
        patients.push_back(p);
    }

    file.close();
    return patients;
}

// Checks BOTH IC and password match
bool loginPatient(const string& id, const string& password) {
    vector<Patient> patients = loadPatientsFromFile();

    for (const auto& p : patients) {
        if (p.patientID == id) {

            // Deactivated account cannot login
            if (p.status == ACCOUNT_INACTIVE) {
                cout << uiIndent() << "This account has been deactivated. "                     << "Please contact staff to reactivate your account.\n";
                return false;
            }

            // Check password
            if (p.patientPassword == password) {
                cout << uiIndent() << "Login successful! Welcome, "                     << p.patientName << "!\n";
                return true;
            }
            else {
                cout << uiIndent() << "Incorrect password.\n";                return false;
            }
        }
    }

    cout << uiIndent() << "Patient ID not found.\n";    return false;
}

// ---- Fixed (hardcoded) doctor accounts ----
// To add a doctor, add another line to the check below.
bool loginDoctor(const string& id, const string& password) {
    if ((id == "doc01" && password == "Doc@123") ||
        (id == "doc02" && password == "Doc@456")) {
        cout << uiIndent() << "Doctor login successful! Welcome, " + id + "!" << "\n";        return true;
    }
    cout << uiIndent() << "Invalid doctor ID or password." << "\n";    return false;
}

// ---- Fixed (hardcoded) counter staff accounts ----
// To add an assistant, add another line to the check below.
bool loginStaff(const string& id, const string& password) {
    if ((id == "staff01" && password == "Staff@123") ||
        (id == "staff02" && password == "Staff@456")) {
        cout << uiIndent() << "Staff login successful! Welcome, " + id + "!" << "\n";        return true;
    }
    cout << uiIndent() << "Invalid staff ID or password." << "\n";    return false;
}

// ---- Fixed (hardcoded) management account ----
bool loginManagement(const string& id, const string& password) {
    if (id == "admin" && password == "Admin@123") {
        cout << uiIndent() << "Management login successful!" << "\n";        return true;
    }
    cout << uiIndent() << "Invalid management ID or password." << "\n";    return false;
}