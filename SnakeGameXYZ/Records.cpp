#include "Records.h"
#include "GameData.h"
#include <fstream>

namespace SnakeGame {
    void loadRecords() {
        records.clear();
        std::ifstream file("records.dat");
        if (file.is_open()) {
            Record record;
            while (file >> record.name >> record.score) {
                records.push_back(record);
            }
            file.close();
        }

        while (records.size() < 10) {
            records.push_back({ "XYZ", 0 });
        }

        std::sort(records.begin(), records.end(), [](const Record& a, const Record& b) {
            return a.score > b.score;
            });
    }

    void saveRecords() {
        std::ofstream file("records.dat");
        if (file.is_open()) {
            for (const auto& record : records) {
                file << record.name << " " << record.score << " " << "\n";
            }
            file.close();
        }
    }

    void addRecord(const std::string& name, int score) {
        records.push_back({ name, score });
        std::sort(records.begin(), records.end(), [](const Record& a, const Record& b) {
            return a.score > b.score;
            });

        if (records.size() > 10) {
            records.pop_back();
        }

        saveRecords();
    }
}