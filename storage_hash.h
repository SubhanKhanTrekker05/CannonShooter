#pragma once
#include <string>
#include <optional>
#include <vector>

struct Profile {
    std::string name;
    int score = 0;
};

class StorageHash {
public:
    StorageHash();
    ~StorageHash();
    bool loadFromFile(const std::string& path);
    bool saveToFile(const std::string& path);
    std::optional<Profile> find(const std::string& name) const;
    void insertOrUpdate(const Profile& p);
    int getScore(const std::string& name) const;
    int getRank(const std::string& name) const;
    std::vector<Profile> getAllProfiles() const;
};
