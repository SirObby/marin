#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <pqxx/pqxx>
#include <dpp/dpp.h>

struct UserFavorite {
    uint64_t media_id;
    std::string type; // "ANIME" or "MANGA"
    int rating;
};

class Database {
private:
    std::unique_ptr<pqxx::connection> conn;
    std::mutex db_mutex;
    std::string conn_str;

public:
    static Database& get() {
        static Database instance;
        return instance;
    }

    void init(const std::string& connection_string);
    
    // User Settings
    std::string get_user_locale(dpp::snowflake user_id);
    void set_user_locale(dpp::snowflake user_id, const std::string& locale);
    std::string get_user_bio(dpp::snowflake user_id);
    void set_user_bio(dpp::snowflake user_id, const std::string& bio);
    
    // Guild Settings
    bool get_guild_nsfw(dpp::snowflake guild_id);
    void set_guild_nsfw(dpp::snowflake guild_id, bool allow);
    uint32_t get_guild_theme(dpp::snowflake guild_id);
    void set_guild_theme(dpp::snowflake guild_id, uint32_t color);
    void delete_guild(dpp::snowflake guild_id);

    // Favorites
    bool add_favorite(dpp::snowflake user_id, uint64_t media_id, const std::string& type, int rating = 0);
    std::vector<UserFavorite> get_favorites(dpp::snowflake user_id);
    
    // GDPR
    void delete_user_data(dpp::snowflake user_id);
};
