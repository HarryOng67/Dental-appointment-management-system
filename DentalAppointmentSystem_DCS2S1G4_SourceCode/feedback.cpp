// This file is #included by main.cpp (it does not compile on its own).
// Feedback: the patient leaves a rating and a comment about the service, and
// the management module reads all of them (with the average rating).
#include "common.cpp"   // structs, headers, and declarations

// ---- File handling for feedback ----
void saveFeedbackToFile(const Feedback& f) {
    ofstream file("feedback.txt", ios::app);
    file << f.feedbackID << ","
         << f.patientID << ","
         << f.rating << ","
         << f.date << ","
         << f.comment << "\n";   // comment last: it is the longest field
    file.close();
}

vector<Feedback> loadFeedbackFromFile() {
    vector<Feedback> list;
    ifstream file("feedback.txt");
    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        Feedback f;
        getline(ss, f.feedbackID, ',');
        getline(ss, f.patientID, ',');
        getline(ss, f.rating, ',');
        getline(ss, f.date, ',');
        getline(ss, f.comment);    // no delimiter: take the rest of the line
        list.push_back(f);
    }
    file.close();
    return list;
}

// Next feedback ID (largest existing + 1)
int nextFeedbackId() {
    vector<Feedback> list = loadFeedbackFromFile();
    int maxId = 0;
    for (const auto& f : list) {
        if (f.feedbackID.empty()) continue;
        int n = 0;
        try { n = stoi(f.feedbackID); } catch (...) { n = 0; }
        if (n > maxId) maxId = n;
    }
    return maxId + 1;
}

// ---- C: the patient leaves feedback ----
void giveFeedback(const string& ic) {
    Feedback f;
    f.feedbackID = to_string(nextFeedbackId());
    f.patientID  = ic;

    cout << uiIndent() << "\n";    cout << uiIndent() << "===== Rate Our Service =====" << "\n";    cout << uiIndent() << "5 = Excellent   4 = Good   3 = Average   2 = Poor   1 = Very Poor" << "\n";
    // rating must be a number inside the allowed range
   int rating;

do {
    cout << uiIndent() << "Your rating (" << MIN_RATING << "-" << MAX_RATING         << ", 0 to cancel): ";

    rating = readInt();

    // User chooses to cancel
    if (rating == 0) {
        cout << uiIndent() << "Feedback cancelled.\n";        return;
    }

    // Invalid rating
    if (rating < MIN_RATING || rating > MAX_RATING) {
        cout << uiIndent() << "  Please enter a number from "             << MIN_RATING << " to "
             << MAX_RATING << ".\n";
    }

} while (rating < MIN_RATING || rating > MAX_RATING);

f.rating = to_string(rating);

    // the comment is a whole sentence, so readLine() is used instead of cin >>
    while (true) {
        cout << uiIndent() << "Your comment (type 0 to cancel): ";        f.comment = readLine();

        if (f.comment == "0") {
            cout << uiIndent() << "Feedback cancelled.\n";            return;
        }
        if (!f.comment.empty()) break;
        cout << uiIndent() << "  The comment cannot be empty.\n";    }

    while (true) {
        cout << uiIndent() << "Today's date (DD/MM/YYYY) (0 to cancel): ";        if (cancel(f.date)) return;
        if (isValidDate(f.date)) break;      // reuses the date check
        cout << uiIndent() << "  Invalid date. Use DD/MM/YYYY\n";    }

    saveFeedbackToFile(f);
    cout << "\n" << uiIndent() << "Thank you for your feedback! (Reference no. " << f.feedbackID << ")\n";}

// ---- R: management reads every comment ----
void viewAllFeedback() {
    vector<Feedback> list = loadFeedbackFromFile();

    if (list.empty()) {
        cout << boxIndent(79) << "No feedback has been received yet.\n";        return;
    }

    vector<Patient> patients = loadPatientsFromFile();

    cout << boxIndent(79) << "\n";    cout << boxIndent(79) << "===============================================================================\n";    cout << boxIndent(79) << "                  BRIGHT SMILE DENTAL CLINIC - PATIENT FEEDBACK\n";    cout << boxIndent(79) << "===============================================================================\n";    {
        ostringstream head;
        head << left << setw(5)  << "No"
             << setw(16) << "Patient"
             << setw(13) << "Date"
             << setw(9)  << "Rating"
             << "Comment";
        cout << boxIndent(79) << head.str() << "\n";    }
    cout << boxIndent(79) << "-------------------------------------------------------------------------------\n";
    int total = 0;
    int count = 0;
    for (const auto& f : list) {
        // join to the patient record so the name is shown, not just the IC
        string name = f.patientID;
        for (const auto& p : patients) {
            if (p.patientID == f.patientID) { name = p.patientName; break; }
        }

        int stars = 0;
        try { stars = stoi(f.rating); } catch (...) { stars = 0; }
        total += stars;
        count++;

        ostringstream row;
        row << left << setw(5)  << f.feedbackID
            << setw(16) << name
            << setw(13) << f.date
            << setw(9)  << string(stars, '*')     // 4 -> "****"
            << f.comment;
        cout << boxIndent(79) << row.str() << "\n";    }

    cout << boxIndent(79) << "-------------------------------------------------------------------------------\n";    {
        ostringstream sum;
        sum << "Total feedback received: " << count;
        cout << boxIndent(79) << sum.str() << "\n";    }
    if (count > 0) {                              // guard against dividing by zero
        ostringstream avg;
        avg << "Average rating         : " << fixed << setprecision(2)
            << ((double)total / count) << " out of " << MAX_RATING;
        cout << boxIndent(79) << avg.str() << "\n";    }
    cout << boxIndent(79) << "===============================================================================\n";}