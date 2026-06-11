# 🩸 Smart Blood Donor Emergency Management System

## 📌 Project Overview

The Smart Blood Donor Emergency Management System is a data structure-based application designed to manage blood donor information efficiently and support hospitals during emergencies.

---

## 🎯 Key Features

### 👤 Donor Management
- Register new donors  
- Update donor details  
- Delete donor records  

### 🔍 Search System
- Search by Donor ID  
- Filter by Blood Group  
- Filter by District  
- Check availability  

### 🩸 Donation History
- Add donation records  
- View full history  
- Delete incorrect records  
- Forward & backward traversal  

### 🚨 Emergency Requests
- Add emergency blood requests  
- Priority-based processing  
- Match available donors  

---

## 🧠 Data Structures Used

### 🌳 AVL Tree
- Stores donor records  
- Fast search, insert, delete (O(log n))  

### 🔗 Doubly Linked List
- Stores donation history  
- Supports forward & backward traversal  

### 🏔 Heap (Priority Queue)
- Manages emergency requests  
- Highest priority processed first  

---

## ⚙️ System Architecture

Frontend (HTML + Tailwind + JS)  
↓  
C++ Backend Engine  
↓  
AVL Tree / DLL / Heap  

---

## 📈 Time Complexity

- AVL Search → O(log n)  
- AVL Insert → O(log n)  
- DLL Insert → O(1)  
- Heap Insert → O(log n)  

---

## 🚀 Conclusion

This system demonstrates efficient use of data structures in a real-world healthcare scenario.
