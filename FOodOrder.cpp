#include <bits/stdc++.h>
using namespace std;

// ================= FILE NAMES =================
const string RESTAURANT_DATA_FILE = "restaurants_data.txt";
const string ORDER_DATA_FILE      = "orders_data.txt";
const string FEEDBACK_DATA_FILE   = "feedback_data.txt";
const string RIDER_DATA_FILE      = "riders_data.txt";
const string RIDER_HISTORY_FILE   = "rider_history_data.txt";

// ================= HASH & OTP =================

string hashPassword(string pass) {
    hash<string> h;
    return to_string(h(pass));
}

class OTP {
public:
    int generate() {
        return rand() % 9000 + 1000;
    }
    bool verify(int real) {
        int input;
        cout << "Enter OTP: ";
        cin >> input;
        return input == real;
    }
};

// ================= BASIC STRUCTS =================

struct MenuItem {
    int id;
    string name;
    double price;
};

struct Restaurant {
    string name;
    string email;
    string location;
    vector<MenuItem> menu;
};

struct Order {
    int id;
    string customerPhone;
    string customerLocation;
    string restaurantEmail;
    int itemId;
    int quantity;
    string status;      
    string riderPhone;

    double itemAmount;      
    double deliveryCharge;  
    double totalAmount;     

    bool paid;
    string paymentMethod;   
};

struct Payment {
    int orderId;
    double amount;
    string method;
    bool success;
};

struct Feedback {
    int orderId;
    string customerPhone;
    string restaurantEmail;
    int rating;     // 1-5
    string comment;
};

struct Rider {
    string phone;
    string location;
    int activeOrders;
    Rider() {}
    Rider(string p, string loc, int act) : phone(p), location(loc), activeOrders(act) {}
};

struct RiderComparator {
    bool operator()(const Rider &a, const Rider &b) const {
        return a.activeOrders > b.activeOrders;  
    }
};

// ================= GLOBAL DATA =================

// Orders
vector<Order> allOrders;
unordered_map<int,int> orderIndex;   

// Restaurants
unordered_map<string, Restaurant> restaurants;  

// Riders
unordered_map<string, Rider> riderInfo;  
priority_queue<Rider, vector<Rider>, RiderComparator> riderHeap;

// Rider delivery history
unordered_map<string, vector<int>> riderDeliveredOrders;  

// Feedback
vector<Feedback> allFeedbacks;

// Order processing queue
queue<int> orderQueue;

// ================= SIMPLE DISTANCE / TIME =================

class LocationDistance {
public:
    int getDistance(string a, string b) {
        if (a == b) return 2;
        if ((a == "Uttara" && b == "Dhanmondi") || (a == "Dhanmondi" && b == "Uttara"))
            return 12;
        if ((a == "Gulshan" && b == "Mirpur") || (a == "Mirpur" && b == "Gulshan"))
            return 10;
        return 8;
    }
    int calculateTime(int km) {
        return km * 2.5 + 5;
    }
};

// ================= RESTAURANT FILE SAVE / LOAD =================

void loadRestaurantsFromFile() {
    ifstream fin(RESTAURANT_DATA_FILE);
    if (!fin.is_open()) return; // first run, file নাই

    restaurants.clear();

    int n;
    fin >> n;
    fin.ignore(numeric_limits<streamsize>::max(), '\n');

    for (int i = 0; i < n; i++) {
        Restaurant r;
        getline(fin, r.name);
        getline(fin, r.email);
        getline(fin, r.location);

        int m;
        fin >> m;
        fin.ignore(numeric_limits<streamsize>::max(), '\n');

        r.menu.clear();
        for (int j = 0; j < m; j++) {
            MenuItem item;
            fin >> item.id;
            fin.ignore();                 
            getline(fin, item.name);      
            fin >> item.price;
            fin.ignore(numeric_limits<streamsize>::max(), '\n');
            r.menu.push_back(item);
        }
        restaurants[r.email] = r;
    }
    fin.close();
}

void saveRestaurantsToFile() {
    ofstream fout(RESTAURANT_DATA_FILE);
    if (!fout.is_open()) return;

    fout << restaurants.size() << "\n";
    for (auto &p : restaurants) {
        Restaurant r = p.second;
        fout << r.name << "\n";
        fout << r.email << "\n";
        fout << r.location << "\n";
        fout << r.menu.size() << "\n";
        for (auto &item : r.menu) {
            fout << item.id << "\n";
            fout << item.name << "\n";
            fout << item.price << "\n";
        }
    }
    fout.close();
}

// ================= ORDERS FILE SAVE / LOAD =================

void saveOrdersToFile() {
    ofstream fout(ORDER_DATA_FILE);
    if (!fout.is_open()) return;

    fout << allOrders.size() << "\n";
    for (auto &o : allOrders) {
        fout << o.id << "\n";
        fout << o.customerPhone << "\n";
        fout << o.customerLocation << "\n";
        fout << o.restaurantEmail << "\n";
        fout << o.itemId << "\n";
        fout << o.quantity << "\n";
        fout << o.status << "\n";
        fout << o.riderPhone << "\n";
        fout << fixed << setprecision(2)
             << o.itemAmount << "\n"
             << o.deliveryCharge << "\n"
             << o.totalAmount << "\n";
        fout << o.paid << "\n";
        fout << o.paymentMethod << "\n";
    }
    fout.close();
}

void loadOrdersFromFile() {
    ifstream fin(ORDER_DATA_FILE);
    if (!fin.is_open()) return;

    allOrders.clear();
    orderIndex.clear();
    while (!orderQueue.empty()) orderQueue.pop();

    int n;
    fin >> n;
    fin.ignore(numeric_limits<streamsize>::max(), '\n');

    for (int i = 0; i < n; i++) {
        Order o;
        fin >> o.id;
        fin.ignore(numeric_limits<streamsize>::max(), '\n');
        getline(fin, o.customerPhone);
        getline(fin, o.customerLocation);
        getline(fin, o.restaurantEmail);
        fin >> o.itemId;
        fin >> o.quantity;
        fin.ignore(numeric_limits<streamsize>::max(), '\n');
        getline(fin, o.status);
        getline(fin, o.riderPhone);
        fin >> o.itemAmount;
        fin >> o.deliveryCharge;
        fin >> o.totalAmount;
        fin >> o.paid;
        fin.ignore(numeric_limits<streamsize>::max(), '\n');
        getline(fin, o.paymentMethod);

        allOrders.push_back(o);
        orderIndex[o.id] = (int)allOrders.size() - 1;

        if (o.status == "Pending") {
            orderQueue.push(o.id);
        }
    }
    fin.close();
}

// ================= FEEDBACK FILE SAVE / LOAD =================

void saveFeedbacksToFile() {
    ofstream fout(FEEDBACK_DATA_FILE);
    if (!fout.is_open()) return;

    fout << allFeedbacks.size() << "\n";
    for (auto &f : allFeedbacks) {
        fout << f.orderId << "\n";
        fout << f.customerPhone << "\n";
        fout << f.restaurantEmail << "\n";
        fout << f.rating << "\n";
        fout << f.comment << "\n";
    }
    fout.close();
}

void loadFeedbacksFromFile() {
    ifstream fin(FEEDBACK_DATA_FILE);
    if (!fin.is_open()) return;

    allFeedbacks.clear();

    int n;
    fin >> n;
    fin.ignore(numeric_limits<streamsize>::max(), '\n');

    for (int i = 0; i < n; i++) {
        Feedback f;
        fin >> f.orderId;
        fin.ignore(numeric_limits<streamsize>::max(), '\n');
        getline(fin, f.customerPhone);
        getline(fin, f.restaurantEmail);
        fin >> f.rating;
        fin.ignore(numeric_limits<streamsize>::max(), '\n');
        getline(fin, f.comment);

        allFeedbacks.push_back(f);
    }
    fin.close();
}

// ================= RIDER FILE SAVE / LOAD =================

void saveRidersToFile() {
    ofstream fout(RIDER_DATA_FILE);
    if (!fout.is_open()) return;

    fout << riderInfo.size() << "\n";
    for (auto &p : riderInfo) {
        const Rider &r = p.second;
        fout << r.phone << "\n";
        fout << r.location << "\n";
        fout << r.activeOrders << "\n";
    }
    fout.close();
}

void loadRidersFromFile() {
    ifstream fin(RIDER_DATA_FILE);
    if (!fin.is_open()) return;

    riderInfo.clear();
    while (!riderHeap.empty()) riderHeap.pop();

    int n;
    fin >> n;
    fin.ignore(numeric_limits<streamsize>::max(), '\n');

    for (int i = 0; i < n; i++) {
        Rider r;
        getline(fin, r.phone);
        getline(fin, r.location);
        fin >> r.activeOrders;
        fin.ignore(numeric_limits<streamsize>::max(), '\n');

        riderInfo[r.phone] = r;
    }

    for (auto &x : riderInfo) {
        riderHeap.push(x.second);
    }

    fin.close();
}

// ================= RIDER HISTORY FILE SAVE / LOAD =================

void saveRiderHistoryToFile() {
    ofstream fout(RIDER_HISTORY_FILE);
    if (!fout.is_open()) return;

    fout << riderDeliveredOrders.size() << "\n";
    for (auto &p : riderDeliveredOrders) {
        const string &phone = p.first;
        const vector<int> &v = p.second;
        fout << phone << "\n";
        fout << v.size() << "\n";
        for (int oid : v) {
            fout << oid << "\n";
        }
    }
    fout.close();
}

void loadRiderHistoryFromFile() {
    ifstream fin(RIDER_HISTORY_FILE);
    if (!fin.is_open()) return;

    riderDeliveredOrders.clear();

    int n;
    fin >> n;
    fin.ignore(numeric_limits<streamsize>::max(), '\n');

    for (int i = 0; i < n; i++) {
        string phone;
        getline(fin, phone);
        int m;
        fin >> m;
        fin.ignore(numeric_limits<streamsize>::max(), '\n');
        vector<int> v;
        for (int j = 0; j < m; j++) {
            int oid;
            fin >> oid;
            fin.ignore(numeric_limits<streamsize>::max(), '\n');
            v.push_back(oid);
        }
        riderDeliveredOrders[phone] = v;
    }
    fin.close();
}

// ================= PAYMENT FUNCTIONS =================

Payment processPayment(int orderId, double amount) {
    Payment p;
    p.orderId = orderId;
    p.amount = amount;

    int methodChoice;
    cout << "\n=== Payment ===\n";
    cout << "1. Cash on Delivery\n";
    cout << "2. Card\n";
    cout << "3. Mobile Banking (Bkash)\n";
    cout << "Choose method: ";
    cin >> methodChoice;

    if (methodChoice == 1) p.method = "Cash";
    else if (methodChoice == 2) p.method = "Card";
    else p.method = "Bkash";

    p.success = true;

    if (p.success) {
        cout << "✅ Payment successful by " << p.method << " for amount " << amount << "\n";
    } else {
        cout << "❌ Payment failed.\n";
    }
    return p;
}

// ================= FEEDBACK FUNCTIONS =================

void giveFeedback(const Order &o) {
    int rating;
    string comment;
    cout << "\n=== Give Feedback for Order " << o.id << " ===\n";
    cout << "Rating (1-5): ";
    cin >> rating;
    cin.ignore();
    cout << "Comment: ";
    getline(cin, comment);

    Feedback f;
    f.orderId = o.id;
    f.customerPhone = o.customerPhone;
    f.restaurantEmail = o.restaurantEmail;
    f.rating = rating;
    f.comment = comment;

    allFeedbacks.push_back(f);
    saveFeedbacksToFile();

    cout << "✅ Thanks for your feedback!\n";
}

void viewRestaurantFeedback(const string &email) {
    cout << "\n=== Feedback for Restaurant: " << email << " ===\n";
    int count = 0;
    int sum = 0;
    for (auto &f : allFeedbacks) {
        if (f.restaurantEmail == email) {
            cout << "Order " << f.orderId
                 << " | Customer: " << f.customerPhone
                 << " | Rating: " << f.rating
                 << " | Comment: " << f.comment << "\n";
            sum += f.rating;
            count++;
        }
    }
    if (count == 0) {
        cout << "No feedback yet.\n";
    } else {
        double avg = (double)sum / count;
        cout << "Average Rating: " << fixed << setprecision(2) << avg << "/5\n";
    }
}

// ================= RECEIPT GENERATION =================

void printReceipt(const Order &o) {
    cout << "\n========== RECEIPT ==========\n";
    cout << "Order ID: " << o.id << "\n";
    cout << "Customer Phone: " << o.customerPhone << "\n";
    cout << "Restaurant: " << o.restaurantEmail << "\n";
    cout << "Customer Location: " << o.customerLocation << "\n";
    cout << "Rider: " << o.riderPhone << "\n";
    cout << "Status: " << o.status << "\n";
    cout << "Quantity: " << o.quantity << "\n";
    cout << "Food Amount: " << o.itemAmount << " Taka\n";
    cout << "Delivery Charge: " << o.deliveryCharge << " Taka\n";
    cout << "------------------------------\n";
    cout << "Total Amount: " << o.totalAmount << " Taka\n";
    cout << "Payment: " << (o.paid ? "Paid" : "Unpaid")
         << " via " << o.paymentMethod << "\n";
    cout << "==============================\n";
}

// ================= LOGIN / SIGNUP FUNCTIONS =================

// Admin
void adminSignup() {
    string email, pass;
    cout << "Admin Email: "; cin >> email;
    cout << "Password: "; cin >> pass;

    ofstream f("admin.txt", ios::app);
    f << email << " " << hashPassword(pass) << endl;
    f.close();

    cout << "✅ Admin Registered\n";
}

bool adminLogin(string &loggedEmail) {
    string email, pass;
    cout << "Admin Email: "; cin >> email;
    cout << "Password: "; cin >> pass;

    ifstream f("admin.txt");
    string e,p;
    while (f >> e >> p) {
        if (e == email && p == hashPassword(pass)) {
            cout << "✅ Admin Login Success\n";
            loggedEmail = email;
            return true;
        }
    }
    cout << "❌ Login Failed\n";
    return false;
}

// Restaurant
void restaurantSignup() {
    string email, pass;
    cout << "Restaurant Email: "; cin >> email;
    cout << "Password: "; cin >> pass;

    ofstream f("restaurant.txt", ios::app);
    f << email << " " << hashPassword(pass) << endl;
    f.close();

    cout << "✅ Restaurant Registered\n";
}

bool restaurantLogin(string &loggedEmail) {
    string email, pass;
    cout << "Restaurant Email: "; cin >> email;
    cout << "Password: "; cin >> pass;

    ifstream f("restaurant.txt");
    string e,p;
    while (f >> e >> p) {
        if (e == email && p == hashPassword(pass)) {
            cout << "✅ Restaurant Login Success\n";
            loggedEmail = email;
            return true;
        }
    }
    cout << "❌ Login Failed\n";
    return false;
}

// Rider
bool riderLogin(string &phone) {
    cout << "Enter Rider Phone: ";
    cin >> phone;

    OTP o;
    int code = o.generate();
    cout << "📩 OTP: " << code << endl;

    if (o.verify(code)) {
        cout << "✅ Rider Login Success\n";
        return true;
    }
    cout << "❌ OTP Failed\n";
    return false;
}

// Customer
bool customerLogin(string &phone) {
    cout << "Enter Customer Phone: ";
    cin >> phone;

    OTP o;
    int code = o.generate();
    cout << "📩 OTP: " << code << endl;

    if (o.verify(code)) {
        cout << "✅ Customer Login Success\n";
        return true;
    }
    cout << "❌ OTP Failed\n";
    return false;
}

// ================= CORE SYSTEM CLASS =================

class SystemCore {
public:
    LocationDistance dist;

    void adminPanel(const string &email);
    void restaurantPanel(const string &email);
    void riderPanel(const string &phone);
    void customerPanel(const string &phone);

    bool assignRiderToOrder(int orderId);       
    bool assignNextAcceptedOrderFIFO();         
};

// ----------- Rider Assign Function (Min-Heap) -------------

bool SystemCore::assignRiderToOrder(int orderId) {
    if (orderIndex.find(orderId) == orderIndex.end()) {
        cout << "Invalid order ID.\n";
        return false;
    }
    Order &o = allOrders[orderIndex[orderId]];

    if (o.status != "Accepted" || o.riderPhone != "N/A") {
        cout << "Order is not in 'Accepted' state without rider.\n";
        return false;
    }

    if (riderInfo.empty()) {
        cout << "No riders available.\n";
        return false;
    }

    while (!riderHeap.empty()) riderHeap.pop();
    for (auto &p : riderInfo) {
        riderHeap.push(p.second);
    }

    Rider topR = riderHeap.top();
    riderHeap.pop();

    cout << "Assigning min-load rider " << topR.phone
         << " (activeOrders = " << topR.activeOrders << ") to Order " << orderId << "\n";

    o.riderPhone = topR.phone;
    o.status = "On the way";

    riderInfo[topR.phone].activeOrders++;

    while (!riderHeap.empty()) riderHeap.pop();
    for (auto &p : riderInfo) {
        riderHeap.push(p.second);
    }

    saveOrdersToFile();
    saveRidersToFile();

    cout << "✅ Rider " << topR.phone << " assigned to Order " << orderId << " (Status: On the way)\n";
    return true;
}

// ----------- FIFO helper: oldest accepted order -------------

bool SystemCore::assignNextAcceptedOrderFIFO() {
    int bestId = -1;

    for (auto &o : allOrders) {
        if (o.status == "Accepted" && o.riderPhone == "N/A") {
            bestId = o.id;  
            break;
        }
    }

    if (bestId == -1) {
        cout << "No accepted order waiting for rider.\n";
        return false;
    }

    cout << "FIFO picked oldest accepted order ID: " << bestId << "\n";
    return assignRiderToOrder(bestId);
}

// ----------- ADMIN PANEL -------------

void SystemCore::adminPanel(const string &email) {
    while (true) {
        int ch;
        cout << "\n===== ADMIN PANEL =====\n";
        cout << "1. Add Restaurant (basic info)\n";
        cout << "2. View All Restaurants\n";
        cout << "3. View All Orders\n";
        cout << "4. View Order Summary\n";
        cout << "5. View Restaurant Feedback\n";
        cout << "6. Delete Restaurant\n";
        cout << "7. Rider Management\n";
        cout << "8. Assign Rider to Accepted Order (FIFO)\n";
        cout << "9. Logout\nChoice: ";
        cin >> ch;

        if (ch == 1) {
            Restaurant r;
            cout << "Restaurant Name: ";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            getline(cin, r.name);

            cout << "Restaurant Email: ";
            cin >> r.email;

            cout << "Location (Uttara/Dhanmondi/Gulshan/Mirpur/Banani or Others): ";
            cin >> r.location;

            r.menu.clear();
            restaurants[r.email] = r;
            saveRestaurantsToFile();
            cout << "✅ Restaurant basic info added. Menu will be managed by restaurant.\n";
        }
        else if (ch == 2) {
            cout << "\n--- All Restaurants ---\n";
            for (auto &p : restaurants) {
                cout << "Name: " << p.second.name
                     << " | Email: " << p.second.email
                     << " | Location: " << p.second.location
                     << " | Menu items: " << p.second.menu.size() << endl;
            }
        }
        else if (ch == 3) {
            cout << "\n--- All Orders ---\n";
            for (auto &o : allOrders) {
                cout << "Order " << o.id
                     << " | Rest: " << o.restaurantEmail
                     << " | Cust: " << o.customerPhone
                     << " | Status: " << o.status
                     << " | Rider: " << o.riderPhone << endl;
            }
        }
        else if (ch == 4) {
            int pending=0, accepted=0, otw=0, del=0, rej=0;
            for (auto &o : allOrders) {
                if (o.status == "Pending") pending++;
                else if (o.status == "Accepted") accepted++;
                else if (o.status == "On the way") otw++;
                else if (o.status == "Delivered") del++;
                else if (o.status == "Rejected") rej++;
            }
            cout << "Pending: " << pending
                 << "\nAccepted: " << accepted
                 << "\nOn the way: " << otw
                 << "\nDelivered: " << del
                 << "\nRejected: " << rej << endl;
        }
        else if (ch == 5) {
            string remail;
            cout << "Enter restaurant email: ";
            cin >> remail;
            if (restaurants.find(remail) == restaurants.end()) {
                cout << "Restaurant not found.\n";
            } else {
                viewRestaurantFeedback(remail);
            }
        }
        else if (ch == 6) {
            string remail;
            cout << "Enter restaurant email to delete: ";
            cin >> remail;

            auto it = restaurants.find(remail);
            if (it == restaurants.end()) {
                cout << "Restaurant not found.\n";
            } else {
                for (auto &o : allOrders) {
                    if (o.restaurantEmail == remail &&
                        (o.status == "Pending" || o.status == "Accepted" || o.status == "On the way")) {
                        o.status = "Rejected";
                    }
                }
                saveOrdersToFile();

                restaurants.erase(it);
                saveRestaurantsToFile();
                cout << "✅ Restaurant deleted.\n";
            }
        }
        else if (ch == 7) {
            while (true) {
                int rc;
                cout << "\n--- RIDER MANAGEMENT ---\n";
                cout << "1. View All Riders\n";
                cout << "2. Add Rider\n";
                cout << "3. Which Riders are Busy\n";
                cout << "4. Which Riders are Free\n";
                cout << "5. View Rider Delivery History\n";
                cout << "6. Delete Rider\n";
                cout << "7. Back to Admin Panel\nChoice: ";
                cin >> rc;

                if (rc == 1) {
                    cout << "\nAll Riders:\n";
                    if (riderInfo.empty()) {
                        cout << "No riders registered yet.\n";
                    } else {
                        for (auto &p : riderInfo) {
                            cout << "Phone: " << p.second.phone
                                 << " | Location: " << p.second.location
                                 << " | Active Orders: " << p.second.activeOrders << "\n";
                        }
                    }
                }
                else if (rc == 2) {
                    string ph, loc;
                    cout << "Enter Rider Phone: ";
                    cin >> ph;
                    cout << "Enter Rider Base Location (Uttara/Dhanmondi/Gulshan/Mirpur/Banani or Others): ";
                    cin >> loc;

                    if (riderInfo.find(ph) != riderInfo.end()) {
                        cout << "Rider already exists.\n";
                    } else {
                        Rider r(ph, loc, 0);
                        riderInfo[ph] = r;
                        cout << "✅ Rider added manually.\n";

                        while (!riderHeap.empty()) riderHeap.pop();
                        for (auto &x : riderInfo) {
                            riderHeap.push(x.second);
                        }
                        saveRidersToFile();
                    }
                }
                else if (rc == 3) {
                    cout << "\nBusy Riders (activeOrders > 0):\n";
                    bool any = false;
                    for (auto &p : riderInfo) {
                        if (p.second.activeOrders > 0) {
                            any = true;
                            cout << "Phone: " << p.second.phone
                                 << " | Location: " << p.second.location
                                 << " | Active Orders: " << p.second.activeOrders << "\n";
                        }
                    }
                    if (!any) cout << "No busy riders right now.\n";
                }
                else if (rc == 4) {
                    cout << "\nFree Riders (activeOrders == 0):\n";
                    bool any = false;
                    for (auto &p : riderInfo) {
                        if (p.second.activeOrders == 0) {
                            any = true;
                            cout << "Phone: " << p.second.phone
                                 << " | Location: " << p.second.location << "\n";
                        }
                    }
                    if (!any) cout << "No free riders (or no riders registered).\n";
                }
                else if (rc == 5) {
                    string ph;
                    cout << "Enter Rider Phone to see history: ";
                    cin >> ph;

                    if (riderDeliveredOrders.find(ph) == riderDeliveredOrders.end() ||
                        riderDeliveredOrders[ph].empty()) {
                        cout << "No delivered orders for this rider.\n";
                    } else {
                        cout << "\nDelivery History for Rider " << ph << ":\n";
                        for (int oid : riderDeliveredOrders[ph]) {
                            Order &o = allOrders[orderIndex[oid]];
                            cout << "Order " << o.id
                                 << " | Restaurant: " << o.restaurantEmail
                                 << " | Customer: " << o.customerPhone
                                 << " | Total: " << o.totalAmount << "\n";
                        }
                    }
                }
                else if (rc == 6) {
                    string ph;
                    cout << "Enter Rider Phone to delete: ";
                    cin >> ph;

                    auto itR = riderInfo.find(ph);
                    if (itR == riderInfo.end()) {
                        cout << "Rider not found.\n";
                    } else {
                        bool assigned = false;
                        for (auto &o : allOrders) {
                            if (o.riderPhone == ph && o.status != "Delivered" && o.status != "Rejected") {
                                assigned = true;
                                break;
                            }
                        }
                        if (assigned) {
                            cout << "Warning: This rider has active/ongoing orders.\n";
                        }

                        riderInfo.erase(itR);
                        cout << "✅ Rider deleted.\n";

                        while (!riderHeap.empty()) riderHeap.pop();
                        for (auto &x : riderInfo) {
                            riderHeap.push(x.second);
                        }
                        saveRidersToFile();
                    }
                }
                else if (rc == 7) {
                    break;
                }
            }
        }
        else if (ch == 8) {
            cout << "\nAssigning rider to next accepted order (FIFO)...\n";
            assignNextAcceptedOrderFIFO();
        }
        else if (ch == 9) {
            break;
        }
    }
}

// ----------- CUSTOMER PANEL -------------

void SystemCore::customerPanel(const string &phone) {
    string myLocation;
    cout << "Enter your location (Uttara/Dhanmondi/Gulshan/Mirpur/Banani or Others): ";
    cin >> myLocation;

    while (true) {
        int ch;
        cout << "\n===== CUSTOMER PANEL =====\n";
        cout << "1. Browse Restaurants by Location\n";
        cout << "2. View My Orders\n";
        cout << "3. Give Feedback on Delivered Orders\n";
        cout << "4. Logout\nChoice: ";
        cin >> ch;

        if (ch == 1) {
            cout << "\nRestaurants near " << myLocation << ":\n";
            vector<string> nearby;
            for (auto &p : restaurants) {
                if (p.second.location == myLocation) {
                    cout << "- " << p.second.email
                         << " (" << p.second.name << ")\n";
                    nearby.push_back(p.second.email);
                }
            }
            if (nearby.empty()) {
                cout << "No restaurants found in your location.\n";
                continue;
            }

            string remail;
            cout << "Enter restaurant email to see menu: ";
            cin >> remail;
            if (restaurants.find(remail) == restaurants.end()) {
                cout << "Invalid restaurant.\n";
                continue;
            }

            Restaurant &r = restaurants[remail];

            if (r.menu.empty()) {
                cout << "This restaurant has no menu items yet.\n";
                continue;
            }

            cout << "\n--- MENU of " << remail << " ---\n";
            for (auto &m : r.menu) {
                cout << m.id << ". " << m.name << " - " << m.price << endl;
            }

            int itemId, qty;
            cout << "Enter Item ID: ";
            cin >> itemId;
            cout << "Quantity: ";
            cin >> qty;

            bool foundItem = false;
            double price = 0;
            for (auto &m : r.menu) {
                if (m.id == itemId) {
                    foundItem = true;
                    price = m.price;
                    break;
                }
            }
            if (!foundItem) {
                cout << "Invalid item.\n";
                continue;
            }

            Order o;
            o.id = allOrders.size() + 1;
            o.customerPhone = phone;
            o.customerLocation = myLocation;
            o.restaurantEmail = remail;
            o.itemId = itemId;
            o.quantity = qty;
            o.status = "Pending";
            o.riderPhone = "N/A";

            o.itemAmount = price * qty;

            Restaurant &rest = restaurants[remail];

            auto isInsideDhaka = [&](const string &loc) {
                return loc == "Uttara" || loc == "Dhanmondi" ||
                       loc == "Gulshan" || loc == "Mirpur" ||
                       loc == "Banani";
            };

            bool customerInside = isInsideDhaka(myLocation);
            bool restaurantInside = isInsideDhaka(rest.location);

            if (customerInside && restaurantInside) {
                o.deliveryCharge = 80;   
            } else {
                o.deliveryCharge = 100;  
            }

            o.totalAmount = o.itemAmount + o.deliveryCharge;

            o.paid = false;
            o.paymentMethod = "None";

            allOrders.push_back(o);
            orderIndex[o.id] = (int)allOrders.size() - 1;
            orderQueue.push(o.id);
            saveOrdersToFile();

            cout << "✅ Order placed. Status: Pending (waiting restaurant accept)\n";
            cout << "Food Amount: " << o.itemAmount
                 << " | Delivery: " << o.deliveryCharge
                 << " | Total: " << o.totalAmount << "\n";
        }
        else if (ch == 2) {
            cout << "\n--- Your Orders ---\n";
            for (auto &o : allOrders) {
                if (o.customerPhone == phone) {
                    cout << "Order " << o.id
                         << " | Rest: " << o.restaurantEmail
                         << " | Status: " << o.status
                         << " | Rider: " << o.riderPhone
                         << " | Total: " << o.totalAmount
                         << " | Paid: " << (o.paid ? "Yes" : "No")
                         << "\n";
                }
            }
        }
        else if (ch == 3) {
            cout << "\n--- Delivered Orders (for feedback) ---\n";
            for (auto &o : allOrders) {
                if (o.customerPhone == phone && o.status == "Delivered") {
                    cout << "Order " << o.id
                         << " | Restaurant: " << o.restaurantEmail
                         << " | Total: " << o.totalAmount << "\n";
                }
            }
            int oid;
            cout << "Enter Order ID to give feedback (0 to cancel): ";
            cin >> oid;
            if (oid == 0) continue;
            if (orderIndex.find(oid) == orderIndex.end()) {
                cout << "Invalid order.\n";
                continue;
            }
            Order &o = allOrders[orderIndex[oid]];
            if (o.customerPhone != phone || o.status != "Delivered") {
                cout << "You can give feedback only for your delivered orders.\n";
                continue;
            }
            giveFeedback(o);
        }
        else if (ch == 4) {
            break;
        }
    }
}

// ----------- RESTAURANT PANEL (menu from restaurant) -------------

void SystemCore::restaurantPanel(const string &email) {
    while (true) {
        int ch;
        cout << "\n===== RESTAURANT PANEL (" << email << ") =====\n";
        cout << "1. Manage Menu (Add/View/Delete Items)\n";
        cout << "2. View Pending Orders\n";
        cout << "3. Update Order (Accept / Reject)\n";
        cout << "4. View All My Orders\n";
        cout << "5. View Feedback\n";
        cout << "6. Logout\nChoice: ";
        cin >> ch;

        Restaurant &rest = restaurants[email];

        if (ch == 1) {
            int mc;
            cout << "\n--- MENU MANAGEMENT ---\n";
            cout << "1. View Menu\n";
            cout << "2. Add Menu Item\n";
            cout << "3. Delete Menu Item\n";
            cout << "4. Back\nChoice: ";
            cin >> mc;

            if (mc == 1) {
                cout << "\n--- MENU of " << rest.email << " ---\n";
                if (rest.menu.empty()) {
                    cout << "No items yet.\n";
                } else {
                    for (auto &m : rest.menu) {
                        cout << m.id << ". " << m.name << " - " << m.price << "\n";
                    }
                }
            }
            else if (mc == 2) {
                MenuItem m;
                m.id = rest.menu.size() + 1;

                cout << "Item name: ";
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                getline(cin, m.name);

                cout << "Price: ";
                while (true) {
                    cin >> m.price;
                    if (cin.fail()) {
                        cout << "Invalid price! Please enter numeric value: ";
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    } else {
                        break;
                    }
                }

                rest.menu.push_back(m);
                saveRestaurantsToFile();
                cout << "✅ Item added to menu.\n";
            }
            else if (mc == 3) {
                if (rest.menu.empty()) {
                    cout << "No items to delete.\n";
                } else {
                    cout << "\nCurrent Menu:\n";
                    for (auto &m : rest.menu) {
                        cout << m.id << ". " << m.name << " - " << m.price << "\n";
                    }
                    int delId;
                    cout << "Enter Item ID to delete: ";
                    cin >> delId;

                    bool found = false;
                    for (size_t i = 0; i < rest.menu.size(); i++) {
                        if (rest.menu[i].id == delId) {
                            rest.menu.erase(rest.menu.begin() + i);
                            found = true;
                            break;
                        }
                    }

                    if (!found) {
                        cout << "Invalid Item ID.\n";
                    } else {
                        for (size_t i = 0; i < rest.menu.size(); i++) {
                            rest.menu[i].id = (int)i + 1;
                        }
                        saveRestaurantsToFile();
                        cout << "✅ Item deleted from menu.\n";
                    }
                }
            }
            else if (mc == 4) {
                // back
            }
        }
        else if (ch == 2) {
            cout << "\n--- Pending Orders ---\n";
            for (auto &o : allOrders) {
                if (o.restaurantEmail == email && o.status == "Pending") {
                    cout << "Order " << o.id
                         << " | Cust: " << o.customerPhone
                         << " | Qty: " << o.quantity
                         << " | Amount: " << o.totalAmount << endl;
                }
            }
        }
        else if (ch == 3) {
            int id;
            cout << "Enter Order ID: ";
            cin >> id;
            bool found = false;
            for (auto &o : allOrders) {
                if (o.id == id && o.restaurantEmail == email) {
                    found = true;
                    cout << "1. Accept\n2. Reject\nChoice: ";
                    int opt; cin >> opt;
                    if (opt == 1) {
                        o.status = "Accepted";
                        cout << "Order accepted.\n";

                        if (!assignRiderToOrder(o.id)) {
                            cout << "No rider could be assigned right now (order will stay as Accepted).\n";
                        } else {
                            cout << "Rider auto-assigned immediately after acceptance.\n";
                        }
                    } else {
                        o.status = "Rejected";
                        cout << "Order rejected.\n";
                    }
                    saveOrdersToFile();
                    saveRidersToFile();
                    break;
                }
            }
            if (!found) cout << "Order not found.\n";
        }
        else if (ch == 4) {
            cout << "\n--- All My Orders ---\n";
            for (auto &o : allOrders) {
                if (o.restaurantEmail == email) {
                    cout << "Order " << o.id
                         << " | Cust: " << o.customerPhone
                         << " | Status: " << o.status
                         << " | Rider: " << o.riderPhone
                         << " | Total: " << o.totalAmount
                         << " | Paid: " << (o.paid ? "Yes" : "No") << endl;
                }
            }
        }
        else if (ch == 5) {
            viewRestaurantFeedback(email);
        }
        else if (ch == 6) {
            break;
        }
    }
}

// ----------- RIDER PANEL (no assign, only delivery) -------------

void SystemCore::riderPanel(const string &phone) {
    string riderLoc;
    cout << "Enter your current location (Uttara/Dhanmondi/Gulshan/Mirpur/Banani or Others): ";
    cin >> riderLoc;

    if (riderInfo.find(phone) == riderInfo.end()) {
        riderInfo[phone] = Rider(phone, riderLoc, 0);
    } else {
        riderInfo[phone].location = riderLoc;
    }
    saveRidersToFile();

    while (true) {
        int ch;
        cout << "\n===== RIDER PANEL (" << phone << ") =====\n";
        cout << "1. View My Assigned Orders\n";
        cout << "2. Update Order Status (Delivered + Payment + Receipt)\n";
        cout << "3. View My Delivery History\n";
        cout << "4. Logout\nChoice: ";
        cin >> ch;

        if (ch == 1) {
            cout << "\n--- Your Assigned Orders ---\n";
            bool any = false;
            for (auto &o : allOrders) {
                if (o.riderPhone == phone && (o.status == "On the way" || o.status == "Accepted")) {
                    any = true;
                    cout << "Order " << o.id
                         << " | Rest: " << o.restaurantEmail
                         << " | Cust: " << o.customerPhone
                         << " | Status: " << o.status
                         << " | Total: " << o.totalAmount << "\n";
                }
            }
            if (!any) cout << "No assigned orders.\n";
        }
        else if (ch == 2) {
            int id;
            cout << "Enter Order ID: ";
            cin >> id;
            bool found = false;
            for (auto &o : allOrders) {
                if (o.id == id && o.riderPhone == phone) {
                    found = true;
                    cout << "Current Status: " << o.status << endl;
                    cout << "1. Mark Delivered (with Payment + Receipt)\n2. Back\nChoice: ";
                    int opt; cin >> opt;
                    if (opt == 1) {
                        if (!o.paid) {
                            Payment p = processPayment(o.id, o.totalAmount);
                            if (p.success) {
                                o.paid = true;
                                o.paymentMethod = p.method;
                            }
                        } else {
                            cout << "Payment already done.\n";
                        }

                        o.status = "Delivered";
                        cout << "Order delivered.\n";

                        riderDeliveredOrders[phone].push_back(o.id);

                        if (riderInfo[phone].activeOrders > 0)
                            riderInfo[phone].activeOrders--;

                        printReceipt(o);

                        saveOrdersToFile();
                        saveRidersToFile();
                        saveRiderHistoryToFile();
                    }
                    break;
                }
            }
            if (!found) cout << "No such order assigned to you.\n";
        }
        else if (ch == 3) {
            cout << "\n--- Your Delivery History ---\n";
            if (riderDeliveredOrders.find(phone) == riderDeliveredOrders.end() ||
                riderDeliveredOrders[phone].empty()) {
                cout << "No delivered orders yet.\n";
            } else {
                for (int oid : riderDeliveredOrders[phone]) {
                    Order &o = allOrders[orderIndex[oid]];
                    cout << "Order " << o.id
                         << " | Restaurant: " << o.restaurantEmail
                         << " | Customer: " << o.customerPhone
                         << " | Total: " << o.totalAmount << "\n";
                }
            }
        }
        else if (ch == 4) {
            break;
        }
    }
}

// ================= MAIN =================

int main() {
    srand(time(0));

    loadRestaurantsFromFile();
    loadOrdersFromFile();
    loadFeedbacksFromFile();
    loadRidersFromFile();
    loadRiderHistoryFromFile();

    SystemCore sys;

    while (true) {
        int role, choice;
        cout << "\nSelect Role:\n";
        cout << "1.Admin\n2.Restaurant\n3.Rider\n4.Customer\n5.Exit\nChoice: ";
        cin >> role;

        if (role == 1) {
            cout << "1.Signup 2.Login\n";
            cin >> choice;
            if (choice == 1) adminSignup();
            else if (choice == 2) {
                string email;
                if (adminLogin(email)) {
                    sys.adminPanel(email);
                }
            }
        }
        else if (role == 2) {
            cout << "1.Signup 2.Login\n";
            cin >> choice;
            if (choice == 1) restaurantSignup();
            else if (choice == 2) {
                string email;
                if (restaurantLogin(email)) {
                    if (restaurants.find(email) == restaurants.end()) {
                        cout << "This restaurant is not added by admin yet.\n";
                    } else {
                        sys.restaurantPanel(email);
                    }
                }
            }
        }
        else if (role == 3) {
            string phone;
            if (riderLogin(phone)) {
                sys.riderPanel(phone);
            }
        }
        else if (role == 4) {
            string phone;
            if (customerLogin(phone)) {
                sys.customerPanel(phone);
            }
        }
        else if (role == 5) {
            break;
        }
    }

    return 0;
}