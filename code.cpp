#include <bits/stdc++.h>
using namespace std;

static vector<vector<string>> readCSV(const string &fname) {
    vector<vector<string>> rows;
    ifstream file(fname);
    if (!file.is_open()) return rows; // empty if not found yet
    string line;
    while (getline(file, line)) {
        vector<string> row; string word; stringstream ss(line);
        while (getline(ss, word, ',')) row.push_back(word);
        if (!row.empty()) rows.push_back(row);
    }
    return rows;
}

static void writeCSV(const string &fname, const vector<vector<string>> &rows) {
    ofstream fout(fname, ios::out);
    for (const auto &r : rows) {
        for (size_t i = 0; i < r.size(); ++i) {
            fout << r[i];
            if (i + 1 < r.size()) fout << ",";
        }
        fout << "\n";
    }
}

static void appendRow(const string &fname, const vector<string> &row) {
    ofstream fout(fname, ios::out | ios::app);
    for (size_t i = 0; i < row.size(); ++i) {
        fout << row[i];
        if (i + 1 < row.size()) fout << ",";
    }
    fout << "\n";
}

static string nowDate() {
    time_t t = time(nullptr);
    tm *lt = localtime(&t);
    char buf[16];
    snprintf(buf, sizeof(buf), "%02d-%02d-%04d", lt->tm_mday, lt->tm_mon + 1, lt->tm_year + 1900);
    return string(buf);
}

static string uniqId(const string &prefix) {
    // naive unique id using epoch milliseconds
    using namespace std::chrono;
    auto ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    return prefix + to_string(ms);
}

static void printTable(const vector<vector<string>> &rows, const vector<string> &headers = {}) {
    vector<size_t> width;
    size_t cols = 0;
    if (!headers.empty()) cols = headers.size();
    for (const auto &r : rows) cols = max(cols, r.size());
    width.assign(cols, 3);
    auto upd = [&](const vector<string>& r){ for (size_t i=0;i<r.size();++i) width[i] = max(width[i], r[i].size()); };
    if (!headers.empty()) upd(headers);
    for (auto &r : rows) upd(r);

    auto sep = [&](){
        for (size_t i=0;i<cols;++i){ cout << "+" << string(width[i]+2,'-'); }
        cout << "+\n";
    };
    auto rowp = [&](const vector<string>& r){
        for (size_t i=0;i<cols;++i){
            string cell = (i<r.size()? r[i] : "");
            cout << "| " << left << setw((int)width[i]) << cell << " ";
        }
        cout << "|\n";
    };

    sep();
    if (!headers.empty()) { rowp(headers); sep(); }
    for (auto &r : rows) rowp(r);
    sep();
}

// ------------------------------------------------------------
// Models: stored in CSV files
// users.csv: id,name,password,role(patient|doctor|admin)
// patients.csv: patient_id,name,age,gender,assigned_doctor_id
// doctors.csv: doctor_id,name,specialization
// appointments.csv: patient_id,doctor_id,date,status
// bills.csv: bill_id,patient_id,amount,status(unpaid|paid),notes
// ------------------------------------------------------------

struct User {
    string id, name, role;

    void display_menu();
    void login();
};

struct Patient : User {
    void menu();
    void view_profile();
    void search_records();
    void view_appointments();
    void request_appointment();
    void view_bills();
    void pay_bill();
};

struct Doctor : User {
    void menu();
    void view_my_appointments();
    void search_patients();
    void add_note_and_bill();
};

struct Admin : User {
    void menu();
    void add_patient();
    void add_doctor();
    void list_patients();
    void list_doctors();
    void search_any();
    void assign_doctor_to_patient();
};

// Utility lookups
static vector<string> getUserById(const string &uid) {
    auto users = readCSV("users.csv");
    for (auto &r : users) if (!r.empty() && r[0]==uid) return r;
    return {};
}

static vector<string> getPatientById(const string &pid) {
    auto rows = readCSV("patients.csv");
    for (auto &r: rows) if (!r.empty() && r[0]==pid) return r;
    return {};
}

static vector<string> getDoctorById(const string &did) {
    auto rows = readCSV("doctors.csv");
    for (auto &r: rows) if (!r.empty() && r[0]==did) return r;
    return {};
}

// User
void User::display_menu() {
    cout << "\n\n🏥 Hospital Management System\n";
    cout << "1) Login\n2) Exit\n> ";
    char c; cin >> c;
    if (c=='1') login(); else exit(0);
}

void User::login() {
    string uid, pwd; cout << "Enter ID: "; cin >> uid; cout << "Enter Password: "; cin >> pwd;
    auto users = readCSV("users.csv");
    for (auto &r : users) {
        if (r.size()>=4 && r[0]==uid && r[2]==pwd) {
            id = r[0]; name = r[1]; role = r[3];
            if (role=="patient") { Patient p; p.id=id; p.name=name; p.role=role; p.menu(); return; }
            if (role=="doctor")  { Doctor d;  d.id=id; d.name=name; d.role=role; d.menu(); return; }
            if (role=="admin")   { Admin a;   a.id=id; a.name=name; a.role=role; a.menu(); return; }
        }
    }
    cout << "Invalid credentials.\n";
    display_menu();
}

// Patient
void Patient::menu() {
    while (true) {
        cout << "\n👤 Patient: " << name << " (" << id << ")\n";
        cout << "1) View Profile\n2) Search Records\n3) View Appointments\n4) Request Appointment\n5) View Bills\n6) Pay Bill\n7) Logout\n> ";
        char c; cin >> c;
        if (c=='1') view_profile();
        else if (c=='2') search_records();
        else if (c=='3') view_appointments();
        else if (c=='4') request_appointment();
        else if (c=='5') view_bills();
        else if (c=='6') pay_bill();
        else if (c=='7') break;
        else cout << "Invalid option.\n";
    }
}

void Patient::view_profile() {
    auto p = getPatientById(id);
    if (p.empty()) { cout << "Profile not found. Ask admin to register you.\n"; return; }
    vector<vector<string>> rows = { p };
    printTable(rows, {"Patient ID","Name","Age","Gender","Contact","Doctor ID"});
}

void Patient::search_records() {
    cout << "Search by (1=Name, 2=DoctorID): "; char c; cin >> c; cin.ignore(numeric_limits<streamsize>::max(),'\n');
    string q; cout << "Enter query: "; getline(cin, q);
    auto rows = readCSV("patients.csv");
    vector<vector<string>> out;
    for (auto &r : rows) {
        if (r.size()>=6) {
            if ((c=='1' && r[1].find(q)!=string::npos) || (c=='2' && r[5]==q)) out.push_back(r);
        }
    }
    if (out.empty()) cout << "No records.\n"; else printTable(out, {"Patient ID","Name","Age","Gender","Contact","Doctor ID"});
}

void Patient::view_appointments() {
    auto ap = readCSV("appointments.csv");
    vector<vector<string>> out;
    for (auto &r : ap) if (r.size()>=4 && r[0]==id) out.push_back(r);
    if (out.empty()) cout << "No appointments.\n"; else printTable(out, {"Patient ID","Doctor ID","Date","Status"});
}

void Patient::request_appointment() {
    string did, date; cout << "Doctor ID: "; cin >> did; cout << "Date (dd-mm-yyyy): "; cin >> date;
    if (getDoctorById(did).empty()) { cout << "Doctor not found.\n"; return; }
    appendRow("appointments.csv", {id, did, date, "scheduled"});
    cout << "Appointment scheduled.\n";
}

void Patient::view_bills() {
    auto bills = readCSV("bills.csv");
    vector<vector<string>> out;
    for (auto &r : bills) if (r.size()>=5 && r[1]==id) out.push_back(r);
    if (out.empty()) cout << "No bills.\n"; else printTable(out, {"Bill ID","Patient ID","Amount","Status","Notes"});
}

void Patient::pay_bill() {
    string billId; cout << "Enter Bill ID to pay: "; cin >> billId;
    auto bills = readCSV("bills.csv"); bool found=false;
    for (auto &r : bills) if (r.size()>=5 && r[0]==billId && r[1]==id) { r[3] = "paid"; found=true; break; }
    if (found) { writeCSV("bills.csv", bills); cout << "Bill paid.\n"; } else cout << "Bill not found.\n";
}

// Doctor
void Doctor::menu() {
    while (true) {
        cout << "\n🩺 Doctor: " << name << " (" << id << ")\n";
        cout << "1) View My Appointments\n2) Search Patients\n3) Add Note & Generate Bill\n4) Logout\n> ";
        char c; cin >> c;
        if (c=='1') view_my_appointments();
        else if (c=='2') search_patients();
        else if (c=='3') add_note_and_bill();
        else if (c=='4') break;
        else cout << "Invalid option.\n";
    }
}

void Doctor::view_my_appointments() {
    auto ap = readCSV("appointments.csv");
    vector<vector<string>> out;
    for (auto &r : ap) if (r.size()>=4 && r[1]==id) out.push_back(r);
    if (out.empty()) cout << "No appointments.\n"; else printTable(out, {"Patient ID","Doctor ID","Date","Status"});
}

void Doctor::search_patients() {
    cout << "Search patients by name contains: "; cin.ignore(numeric_limits<streamsize>::max(),'\n'); string q; getline(cin,q);
    auto rows = readCSV("patients.csv"); vector<vector<string>> out;
    for (auto &r : rows) if (r.size()>=6 && r[1].find(q)!=string::npos) out.push_back(r);
    if (out.empty()) cout << "No matches.\n"; else printTable(out, {"Patient ID","Name","Age","Gender","Contact","Doctor ID"});
}

void Doctor::add_note_and_bill() {
    string pid; cout << "Patient ID: "; cin >> pid; cin.ignore(numeric_limits<streamsize>::max(),'\n');
    if (getPatientById(pid).empty()) { cout << "Patient not found.\n"; return; }
    string notes; cout << "Visit Notes: "; getline(cin, notes);
    string amount; cout << "Bill Amount (Rs.): "; getline(cin, amount);
    string billId = uniqId("B");
    appendRow("bills.csv", {billId, pid, amount, "unpaid", notes});
    auto ap = readCSV("appointments.csv");
    bool marked=false; string today = nowDate();
    for (auto &r : ap) if (r.size()>=4 && r[0]==pid && r[1]==id && r[2]==today && r[3]=="scheduled") { r[3] = "completed"; marked=true; break; }
    if (marked) writeCSV("appointments.csv", ap);
    cout << "Note saved and bill generated. BillID=" << billId << "\n";
}

// Admin
void Admin::menu() {
    while (true) {
        cout << "\n⚙️ Admin: " << name << " (" << id << ")\n";
        cout << "1) Add Patient\n2) Add Doctor\n3) List Patients\n4) List Doctors\n5) Search (Patients/Doctors)\n6) Assign Doctor to Patient\n7) Logout\n> ";
        char c; cin >> c;
        if (c=='1') add_patient();
        else if (c=='2') add_doctor();
        else if (c=='3') list_patients();
        else if (c=='4') list_doctors();
        else if (c=='5') search_any();
        else if (c=='6') assign_doctor_to_patient();
        else if (c=='7') break;
        else cout << "Invalid option.\n";
    }
}

void Admin::add_patient() {
    string pid, pname, age, gender, contact, did;
    cout << "Patient ID: "; cin >> pid; cin.ignore(numeric_limits<streamsize>::max(),'\n');
    cout << "Name: "; getline(cin, pname);
    cout << "Age: "; getline(cin, age);
    cout << "Gender: "; getline(cin, gender);
    cout << "Contact: "; getline(cin, contact);
    cout << "Assigned Doctor ID (optional, blank=none): "; getline(cin, did);
    appendRow("patients.csv", {pid, pname, age, gender, contact, did});
    appendRow("users.csv", {pid, pname, pid, "patient"});
    cout << "Patient added & user created.\n";
}

void Admin::add_doctor() {
    string did, dname, spec, contact;
    cout << "Doctor ID: "; cin >> did; cin.ignore(numeric_limits<streamsize>::max(),'\n');
    cout << "Name: "; getline(cin, dname);
    cout << "Specialization: "; getline(cin, spec);
    cout << "Contact: "; getline(cin, contact);
    appendRow("doctors.csv", {did, dname, spec, contact});
    appendRow("users.csv", {did, dname, did, "doctor"});
    cout << "Doctor added & user created.\n";
}

void Admin::list_patients() {
    auto rows = readCSV("patients.csv");
    if (rows.empty()) cout << "No patients.\n"; else printTable(rows, {"Patient ID","Name","Age","Gender","Contact","Doctor ID"});
}

void Admin::list_doctors() {
    auto rows = readCSV("doctors.csv");
    if (rows.empty()) cout << "No doctors.\n"; else printTable(rows, {"Doctor ID","Name","Specialization","Contact"});
}

void Admin::search_any() {
    cout << "Search (1=Patients by name, 2=Doctors by name, 3=Appointments by date): "; char c; cin >> c; cin.ignore(numeric_limits<streamsize>::max(),'\n');
    string q; cout << "Enter query: "; getline(cin, q);
    if (c=='1') {
        auto rows = readCSV("patients.csv"); vector<vector<string>> out;
        for (auto &r : rows) if (r.size()>=2 && r[1].find(q)!=string::npos) out.push_back(r);
        if (out.empty()) cout << "No matches.\n"; else printTable(out, {"Patient ID","Name","Age","Gender","Contact","Doctor ID"});
    } else if (c=='2') {
        auto rows = readCSV("doctors.csv"); vector<vector<string>> out;
        for (auto &r : rows) if (r.size()>=2 && r[1].find(q)!=string::npos) out.push_back(r);
        if (out.empty()) cout << "No matches.\n"; else printTable(out, {"Doctor ID","Name","Specialization","Contact"});
    } else if (c=='3') {
        auto rows = readCSV("appointments.csv"); vector<vector<string>> out;
        for (auto &r : rows) if (r.size()>=3 && r[2]==q) out.push_back(r);
        if (out.empty()) cout << "No matches.\n"; else printTable(out, {"Patient ID","Doctor ID","Date","Status"});
    } else {
        cout << "Invalid choice.\n";
    }
}

void Admin::assign_doctor_to_patient() {
    string pid, did; cout << "Patient ID: "; cin >> pid; cout << "Doctor ID: "; cin >> did;
    auto pats = readCSV("patients.csv"); bool ok=false;
    for (auto &r : pats) if (r.size()>=6 && r[0]==pid) { r[5] = did; ok=true; break; }
    if (ok) { writeCSV("patients.csv", pats); cout << "Assigned.\n"; }
    else cout << "Patient not found.\n";
}

// main
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    vector<pair<string, vector<string>>> seeds = {
        {"users.csv", {}},
        {"patients.csv", {}},
        {"doctors.csv", {}},
        {"appointments.csv", {}},
        {"bills.csv", {}}
    };
    for (auto &p : seeds) {
        ifstream fin(p.first); if (!fin.good()) { ofstream fout(p.first); }
    }

    User u; u.display_menu();
    return 0;
}
