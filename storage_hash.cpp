#include "storage_hash.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <locale>

static std::unordered_map<std::string, Profile> g_profiles;

static inline std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
    if (start == s.size()) return "";
    size_t end = s.size() - 1;
    while (end > start && std::isspace(static_cast<unsigned char>(s[end]))) --end;
    return s.substr(start, end - start + 1);
}
static inline std::string toLower(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (unsigned char c : s) out.push_back(static_cast<char>(std::tolower(c)));
    return out;
}
static inline std::string normalizeKey(const std::string& s) {
    return toLower(trim(s));
}

StorageHash::StorageHash() {}
StorageHash::~StorageHash() {}
bool StorageHash::loadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "profiles.dat not found — starting fresh.\n";
        g_profiles.clear();
        return true;
    }

    g_profiles.clear();
    std::string line;
    bool firstLine = true;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        if (firstLine && line.size() >= 3 && (unsigned char)line[0] == 0xEF && (unsigned char)line[1] == 0xBB && (unsigned char)line[2] == 0xBF) {
            line = line.substr(3);
        }
        firstLine = false;
        std::string tmp = line;
        std::istringstream htmp(tmp);
        std::string firstToken;
        if (std::getline(htmp, firstToken, ',')) {
            std::string ftLower = toLower(trim(firstToken));
            if (ftLower == "name" || ftLower == "username") continue;
        }
        std::istringstream iss(line);
        std::string namePart;
        if (!std::getline(iss, namePart, ',')) continue;
        std::string scorePart;
        if (!std::getline(iss, scorePart)) {
            size_t commaPos = line.find(',');
            if (commaPos == std::string::npos) continue;
            scorePart = line.substr(commaPos + 1);
        }

        std::string nameTrim = trim(namePart);
        if (nameTrim.empty()) continue;
        int score = 0;
        try {
            std::string scTrim = trim(scorePart);
            if (!scTrim.empty()) score = std::stoi(scTrim);
            else score = 0;
        } catch (...) {
            score = 0;
        }
        std::string key = normalizeKey(nameTrim);
        g_profiles[key] = { nameTrim, score };
    }

    file.close();
    std::cout << "[Storage] Loaded " << g_profiles.size() << " profiles.\n";
    return true;
}
bool StorageHash::saveToFile(const std::string& path) {
    std::ofstream file(path);
    if (!file.is_open()) {
        std::cout << "Failed to save to " << path << "\n";
        return false;
    }
    file << "name,score\n";
    for (const auto& kv : g_profiles) {
        file << kv.second.name << "," << kv.second.score << "\n";
    }
    file.close();
    std::cout << "[Storage] Saved " << g_profiles.size() << " profiles.\n";
    return true;
}
std::optional<Profile> StorageHash::find(const std::string& name) const {
    std::string key = normalizeKey(name);
    auto it = g_profiles.find(key);
    if (it == g_profiles.end()) return std::nullopt;
    return it->second;
}
void StorageHash::insertOrUpdate(const Profile& p) {
    std::string trimmed = trim(p.name);
    std::string key = normalizeKey(trimmed);
    Profile stored = p;
    stored.name = trimmed; 
    g_profiles[key] = stored;
}
int StorageHash::getScore(const std::string& name) const {
    std::string key = normalizeKey(name);
    auto it = g_profiles.find(key);
    if (it == g_profiles.end()) return 0;
    return it->second.score;
}
std::vector<Profile> StorageHash::getAllProfiles() const {
    std::vector<Profile> out;
    out.reserve(g_profiles.size());
    for (const auto &kv : g_profiles) out.push_back(kv.second);
    return out;
}
int StorageHash::getRank(const std::string& name) const {
    auto all = getAllProfiles();
    if (all.empty()) return 1;
    std::sort(all.begin(), all.end(), [](const Profile& a, const Profile& b){
        if (a.score != b.score) return a.score > b.score;
        std::string an = toLower(a.name);
        std::string bn = toLower(b.name);
        return an < bn;
    });
    std::string key = normalizeKey(name);
    for (size_t i = 0; i < all.size(); ++i) {
        if (normalizeKey(all[i].name) == key) {
            return static_cast<int>(i) + 1;
        }    
    }
    return static_cast<int>(all.size()) + 1;
}
