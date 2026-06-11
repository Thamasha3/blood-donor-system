#include "httplib.h"
#include "avl_tree.h"
#include "doubly_linked_list.h"
#include "priority_heap.h"
#include <iostream>
#include <sstream>

using namespace std;

// Quick helper utility function to extract fields from basic URL-encoded form lines
string getFormValue(const string& body, const string& key) {
    size_t pos = body.find(key + "=");
    if (pos == string::npos) return "";
    pos += key.length() + 1;
    size_t end_pos = body.find("&", pos);
    string val = body.substr(pos, end_pos - pos);

    // 1. Decode standard URL-encoded plus symbols back to spaces
    size_t space_pos;
    while ((space_pos = val.find("+")) != string::npos) {
        val.replace(space_pos, 1, " ");
    }

    // 2. Decode explicit "%20" hex strings back to spaces
    size_t pct_space_pos;
    while ((pct_space_pos = val.find("%20")) != string::npos) {
        val.replace(pct_space_pos, 3, " ");
    }

    // 3. Decode explicit "%2B" hex strings back to actual "+" signs
    size_t pct_plus_pos;
    while ((pct_plus_pos = val.find("%2B")) != string::npos) {
        val.replace(pct_plus_pos, 3, "+");
    }

    // 4. Case-insensitive handle for lowercase "%2b" just in case
    size_t pct_plus_l_pos;
    while ((pct_plus_l_pos = val.find("%2b")) != string::npos) {
        val.replace(pct_plus_l_pos, 3, "+");
    }

    return val;
}

int main() {
    httplib::Server svr;
    AVLTree registry;
    PriorityHeap triage;

    // Seed sample structural data on startup
    registry.insert({"D101", "Amal Perera", "A+", "Matara", "2026-02-10", true});
    registry.insert({"D102", "Gothami Silva", "O-", "Galle", "2026-01-15", true});
    registry.insert({"D103", "Janith Nimesha", "B+", "Colombo", "2026-05-20", false});
    triage.insert("REQ-801", "O-", 3);

    // Helper CORS middleware rule setup to bypass browser security blocks
    svr.set_to_options([](const httplib::Request&, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
    });

    // 🌐 HTTP GET endpoint: Stream current memory data maps out to web browser UI layout
    svr.Get("/api/data", [&](const httplib::Request&, httplib::Response& res) {
        stringstream json;
        json << "{\n  \"donors\": [\n";
        vector<Donor> activeDonors = registry.filterDonors("ALL", "ALL");
        for (size_t i = 0; i < activeDonors.size(); ++i) {
            json << "    {\n";
            json << "      \"id\": \"" << activeDonors[i].id << "\",\n";
            json << "      \"name\": \"" << activeDonors[i].name << "\",\n";
            json << "      \"blood\": \"" << activeDonors[i].bloodGroup << "\",\n";
            json << "      \"district\": \"" << activeDonors[i].district << "\"\n";
            json << "    }" << (i + 1 < activeDonors.size() ? ",\n" : "\n");
        }
        json << "  ],\n  \"emergencies\": [\n";
        vector<EmergencyRequest> reqs = triage.getRawHeap();
        for (size_t i = 0; i < reqs.size(); ++i) {
            string label = (reqs[i].urgencyScore == 3) ? "CRITICAL" : (reqs[i].urgencyScore == 2 ? "URGENT" : "NORMAL");
            json << "    {\n";
            json << "      \"id\": \"" << reqs[i].requestId << "\",\n";
            json << "      \"blood\": \"" << reqs[i].bloodGroupNeeded << "\",\n";
            json << "      \"urgency\": \"" << label << "\"\n";
            json << "    }" << (i + 1 < reqs.size() ? ",\n" : "\n");
        }
        json << "  ]\n}";

        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(json.str(), "application/json");
    });

    // 🌐 HTTP POST endpoint: Intercept web form values and execute AVL tree insertions
    svr.Post("/api/registerDonor", [&](const httplib::Request& req, httplib::Response& res) {
        string body = req.body;
        Donor d;
        d.id = getFormValue(body, "id");
        d.name = getFormValue(body, "name");
        d.bloodGroup = getFormValue(body, "bloodGroup");
        d.district = getFormValue(body, "district");
        d.isAvailable = true;

        if(!d.id.empty() && !d.name.empty()) {
            registry.insert(d);
            cout << "\n[📡 HTTP API] Inserted dynamic node into AVL Tree: " << d.id << " - " << d.name << endl;
            res.set_content("{\"status\":\"success\"}", "application/json");
        } else {
            res.set_content("{\"status\":\"error\",\"message\":\"Empty input allocation fields.\"}", "application/json");
        }
        res.set_header("Access-Control-Allow-Origin", "*");
    });

    // 🌐 HTTP POST endpoint: Intercept emergency request variables and push into Max-Heap
    svr.Post("/api/createRequest", [&](const httplib::Request& req, httplib::Response& res) {
        string body = req.body;
        string rId = getFormValue(body, "id");
        string rHospital = getFormValue(body, "hospital");
        string bGrp = getFormValue(body, "blood");
        string urgStr = getFormValue(body, "urgency");
        int score = (urgStr.find("3") != string::npos) ? 3 : ((urgStr.find("2") != string::npos) ? 2 : 1);

        if(!rId.empty()) {
            triage.insert(rId, bGrp, score);
            cout << "\n[📡 HTTP API] Pushed emergency node onto Max-Heap: " << rId;
            if(!rHospital.empty()) {
                cout << " from " << rHospital;
            }
            cout << " (Urgency Tier " << score << ")" << endl;
            res.set_content("{\"status\":\"success\"}", "application/json");
        } else {
            res.set_content("{\"status\":\"error\"}", "application/json");
        }
        res.set_header("Access-Control-Allow-Origin", "*");
    });

    // 🌐 HTTP POST endpoint: Intercept deletion command and execute true AVL tree node purging
    svr.Post("/api/purgeDonor", [&](const httplib::Request& req, httplib::Response& res) {
        string body = req.body;
        string targetId = getFormValue(body, "id");

        if (!targetId.empty()) {
            // Invokes the native rebalancing removal loop inside your custom library classes
            registry.remove(targetId);

            cout << "\n[📡 HTTP API] Structurally purged node from AVL Tree: " << targetId << endl;
            res.set_content("{\"status\":\"success\"}", "application/json");
        } else {
            res.set_content("{\"status\":\"error\",\"message\":\"Target ID reference missing.\"}", "application/json");
        }
        res.set_header("Access-Control-Allow-Origin", "*");
    });

    cout << "========================================================\n";
    cout << "  \u001b[31m\u25cf FULL-STACK NETWORK SERVER LIVE ENGINE ONLINE \u25cf\u001b[0m   \n";
    cout << "  Listening for API calls live on: http://localhost:8080\n";
    cout << "========================================================\n" << endl;

    svr.listen("0.0.0.0", 8080);
    return 0;
}
