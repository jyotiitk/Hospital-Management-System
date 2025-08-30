#include <bits/stdc++.h>
#include <mysql/jdbc.h>
using namespace std;
using namespace sql;

// Global DB connection
unique_ptr<Connection> con;

// Utility: run a query with prepared statement
static vector<vector<string>> runQuery(const string &q, const vector<string> &params = {}) {
    vector<vector<string>> result;
    try {
        unique_ptr<PreparedStatement> ps(con->prepareStatement(q));
        for (size_t i = 0; i < params.size(); i++) {
            ps->setString((int)(i + 1), params[i]);
        }
        if (q.find("SELECT") == 0) {
            unique_ptr<ResultSet> rs(ps->executeQuery());
            int cols = rs->getMetaData()->getColumnCount();
            while (rs->next()) {
                vector<string> row;
                for (int i = 1; i <= cols; i++) row.push_back(rs->getString(i));
                result.push_back(row);
            }
        } else {
            ps->execute();
        }
    } catch (SQLException &e) {
        cerr << "SQL error: " << e.what() << endl;
    }
    return result;
}

// USER 
struct User {
    string id, name, role;
    void display_menu();
    void login();
};

void User::display_menu() {
    cout << "\n\n🏥 Hospital Management System\n";
    cout << "1) Login\n2) Exit\n> ";
    char c; cin >> c;
    if (c=='1') login();
    else exit(0);
}

void User::login() {
    string uid, pwd; 
    cout << "Enter ID: "; cin >> uid; 
    cout << "Enter Password: "; cin >> pwd;

    auto rows = runQuery("SELECT id, name, role FROM users WHERE id=? AND password=?",
                         {uid, pwd});
    if (rows.empty()) {
        cout << "Invalid credentials.\n";
        display_menu();
        return;
    }
    id = rows[0][0]; name = rows[0][1]; role = rows[0][2];

    if (role=="patient") { struct Patient p; p.id=id; p.name=name; p.role=role; p.menu(); }
    else if (role=="doctor") { struct Doctor d; d.id=id; d.name=name; d.role=role; d.menu(); }
    else if (role=="admin") { struct Admin a; a.id=id; a.name=name; a.role=role; a.menu(); }
}

// PATIENT
struct Patient : User {
    void menu();
    void view_profile();
    void view_appointments();
    void request_appointment();
    void view_bills();
    void pay_bill();
};

void Patient::menu() {
    while (true) {
        cout << "\n👤 Patient: " << name << " (" << id << ")\n";
        cout << "1) View Profile\n2) View Appointments\n3) Request Appointment\n4) View Bills\n5) Pay Bill\n6) Logout\n> ";
        char c; cin >> c;
        if (c=='1') view_profile();
        else if (c=='2') view_appointments();
        else if (c=='3') request_appointment();
        else if (c=='4') view_bills();
        else if (c=='5') pay_bill();
        else if (c=='6') break;
    }
}

void Patient::view_profile() {
    auto rows = runQuery("SELECT * FROM patients WHERE patient_id=?", {id});
    if (rows.empty()) cout << "Profile not found.\n";
    else {
        auto &p = rows[0];
        cout << "ID: " << p[0] << "\nName: " << p[1] << "\nAge: " << p[2]
             << "\nGender: " << p[3] << "\nContact: " << p[4]
             << "\nDoctorID: " << p[5] << endl;
    }
}

void Patient::view_appointments() {
    auto rows = runQuery("SELECT patient_id, doctor_id, date, status FROM appointments WHERE patient_id=?", {id});
    if (rows.empty()) cout << "No appointments.\n";
    else for (auto &r: rows) cout << r[0]<<" | "<<r[1]<<" | "<<r[2]<<" | "<<r[3]<<endl;
}

void Patient::request_appointment() {
    string did, date; 
    cout << "Doctor ID: "; cin >> did; 
    cout << "Date (yyyy-mm-dd): "; cin >> date;
    runQuery("INSERT INTO appointments (patient_id, doctor_id, date, status) VALUES (?,?,?,?)", {id,did,date,"scheduled"});
    cout << "Appointment requested.\n";
}

void Patient::view_bills() {
    auto rows = runQuery("SELECT bill_id, amount, status, notes FROM bills WHERE patient_id=?", {id});
    if (rows.empty()) cout << "No bills.\n";
    else for (auto &r: rows) cout << "Bill "<<r[0]<<": Rs."<<r[1]<<" | "<<r[2]<<" | "<<r[3]<<endl;
}

void Patient::pay_bill() {
    string bid; cout << "Enter Bill ID: "; cin >> bid;
    runQuery("UPDATE bills SET status='paid' WHERE bill_id=? AND patient_id=?", {bid,id});
    cout << "Payment done.\n";
}

// DOCTOR 
struct Doctor : User {
    void menu();
    void view_my_appointments();
    void add_note_and_bill();
};

void Doctor::menu() {
    while (true) {
        cout << "\n🩺 Doctor: " << name << " (" << id << ")\n";
        cout << "1) View My Appointments\n2) Add Note & Bill\n3) Logout\n> ";
        char c; cin >> c;
        if (c=='1') view_my_appointments();
        else if (c=='2') add_note_and_bill();
        else if (c=='3') break;
    }
}

void Doctor::view_my_appointments() {
    auto rows = runQuery("SELECT patient_id, date, status FROM appointments WHERE doctor_id=?", {id});
    if (rows.empty()) cout << "No appointments.\n";
    else for (auto &r: rows) cout << "Patient "<<r[0]<<" | Date "<<r[1]<<" | "<<r[2]<<endl;
}

void Doctor::add_note_and_bill() {
    string pid, notes, amount;
    cout << "Patient ID: "; cin >> pid; cin.ignore();
    cout << "Visit Notes: "; getline(cin, notes);
    cout << "Amount: "; cin >> amount;
    runQuery("INSERT INTO bills (patient_id, amount, status, notes) VALUES (?,?, 'unpaid', ?)", {pid,amount,notes});
    runQuery("UPDATE appointments SET status='completed' WHERE patient_id=? AND doctor_id=? AND status='scheduled'", {pid,id});
    cout << "Note added & bill generated.\n";
}

// ADMIN 
struct Admin : User {
    void menu();
    void add_patient();
    void add_doctor();
};

void Admin::menu() {
    while (true) {
        cout << "\n⚙️ Admin: " << name << " (" << id << ")\n";
        cout << "1) Add Patient\n2) Add Doctor\n3) Logout\n> ";
        char c; cin >> c;
        if (c=='1') add_patient();
        else if (c=='2') add_doctor();
        else if (c=='3') break;
    }
}

void Admin::add_patient() {
    string pid, pname, age, gender, contact, did;
    cout << "Patient ID: "; cin >> pid; cin.ignore();
    cout << "Name: "; getline(cin, pname);
    cout << "Age: "; getline(cin, age);
    cout << "Gender: "; getline(cin, gender);
    cout << "Contact: "; getline(cin, contact);
    cout << "Doctor ID: "; getline(cin, did);

    runQuery("INSERT INTO patients (patient_id,name,age,gender,contact,assigned_doctor_id) VALUES (?,?,?,?,?,?)",
             {pid,pname,age,gender,contact,did});
    runQuery("INSERT INTO users (id,name,password,role) VALUES (?,?,?,?)",
             {pid,pname,pid,"patient"});
    cout << "Patient added.\n";
}

void Admin::add_doctor() {
    string did, dname, spec, contact;
    cout << "Doctor ID: "; cin >> did; cin.ignore();
    cout << "Name: "; getline(cin, dname);
    cout << "Specialization: "; getline(cin, spec);
    cout << "Contact: "; getline(cin, contact);

    runQuery("INSERT INTO doctors (doctor_id,name,specialization,contact) VALUES (?,?,?,?)",
             {did,dname,spec,contact});
    runQuery("INSERT INTO users (id,name,password,role) VALUES (?,?,?,?)",
             {did,dname,did,"doctor"});
    cout << "Doctor added.\n";
}

//MAIN
int main() {
    try {
        Driver *driver = get_driver_instance();
        con.reset(driver->connect("tcp://127.0.0.1:3306", "root", "yourpassword"));
        con->setSchema("hospital");
    } catch (SQLException &e) {
        cerr << "MySQL connection failed: " << e.what() << endl;
        return 1;
    }
    User u; u.display_menu();
    return 0;
}

    




   
