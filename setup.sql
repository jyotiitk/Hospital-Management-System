CREATE DATABASE IF NOT EXISTS hospital;
USE hospital;

-- ================= USERS =================
CREATE TABLE IF NOT EXISTS users (
  id VARCHAR(64) PRIMARY KEY,
  name VARCHAR(120) NOT NULL,
  password VARCHAR(255) NOT NULL,
  role ENUM('patient','doctor','admin') NOT NULL
);

-- ================= DOCTORS =================
CREATE TABLE IF NOT EXISTS doctors (
  doctor_id VARCHAR(64) PRIMARY KEY,
  name VARCHAR(120),
  specialization VARCHAR(120),
  contact VARCHAR(64)
);

-- ================= PATIENTS =================
CREATE TABLE IF NOT EXISTS patients (
  patient_id VARCHAR(64) PRIMARY KEY,
  name VARCHAR(120),
  age INT,
  gender VARCHAR(16),
  contact VARCHAR(64),
  assigned_doctor_id VARCHAR(64),
  FOREIGN KEY (assigned_doctor_id) REFERENCES doctors(doctor_id)
);

-- ================= APPOINTMENTS =================
CREATE TABLE IF NOT EXISTS appointments (
  id INT AUTO_INCREMENT PRIMARY KEY,
  patient_id VARCHAR(64),
  doctor_id VARCHAR(64),
  date DATE,
  status ENUM('scheduled','completed') DEFAULT 'scheduled',
  FOREIGN KEY (patient_id) REFERENCES patients(patient_id),
  FOREIGN KEY (doctor_id) REFERENCES doctors(doctor_id)
);

-- ================= BILLS =================
CREATE TABLE IF NOT EXISTS bills (
  bill_id INT AUTO_INCREMENT PRIMARY KEY,
  patient_id VARCHAR(64),
  amount DECIMAL(10,2),
  status ENUM('unpaid','paid') DEFAULT 'unpaid',
  notes TEXT,
  FOREIGN KEY (patient_id) REFERENCES patients(patient_id)
);

-- ================= DEFAULT ADMIN =================
INSERT INTO users (id, name, password, role)
VALUES ('admin', 'Administrator', 'admin', 'admin')
AS new ON DUPLICATE KEY UPDATE
  name=new.name, password=new.password, role=new.role;

-- ================= SAMPLE DOCTORS =================
INSERT INTO doctors (doctor_id, name, specialization, contact) VALUES
('D101', 'Dr. Meera Sharma', 'Cardiologist', '9876543210'),
('D102', 'Dr. Arjun Verma', 'Neurologist', '9876500001'),
('D103', 'Dr. Amit Kumar', 'Dermatologist', '9876500002')
ON DUPLICATE KEY UPDATE name=VALUES(name);

INSERT INTO users (id, name, password, role) VALUES
('D101', 'Dr. Meera Sharma', 'D101', 'doctor'),
('D102', 'Dr. Arjun Verma', 'D102', 'doctor'),
('D103', 'Dr. Amit Kumar', 'D103', 'doctor')
ON DUPLICATE KEY UPDATE name=VALUES(name);

-- ================= SAMPLE PATIENTS =================
INSERT INTO patients (patient_id, name, age, gender, contact, assigned_doctor_id) VALUES
('P201', 'Shanvi Kiran', 34, 'Female', '9876501000', 'D101'),
('P202', 'Ananya Singh', 29, 'Female', '9876501001', 'D102'),
('P203', 'Vinay Sharma', 40, 'Male', '9876501002', 'D103')
ON DUPLICATE KEY UPDATE name=VALUES(name);

INSERT INTO users (id, name, password, role) VALUES
('P201', 'Shanvi Kiran', 'P201', 'patient'),
('P202', 'Ananya Singh', 'P202', 'patient'),
('P203', 'Vinay Sharma', 'P203', 'patient')
ON DUPLICATE KEY UPDATE name=VALUES(name);

-- ================= SAMPLE APPOINTMENTS =================
INSERT INTO appointments (patient_id, doctor_id, date, status) VALUES
('P201', 'D101', '2025-09-01', 'scheduled'),
('P202', 'D102', '2025-09-02', 'completed'),
('P203', 'D103', '2025-09-03', 'scheduled');

-- ================= SAMPLE BILLS =================
INSERT INTO bills (patient_id, amount, status, notes) VALUES
('P201', 1500.00, 'unpaid', 'Consultation and ECG'),
('P202', 2500.00, 'paid', 'MRI Scan'),
('P203', 1200.00, 'unpaid', 'Skin allergy treatment');
