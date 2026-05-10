#include "database.h"
#include <iostream>

void Database::init(const std::string& connection_string) {
    conn_str = connection_string;
    try {
        conn = std::make_unique<pqxx::connection>(conn_str);
        if (!conn->is_open()) {
            std::cerr << "Can't open database" << std::endl;
            return;
        }

        pqxx::work W(*conn);
        
        W.exec(R"(
            CREATE TABLE IF NOT EXISTS users (
                user_id BIGINT PRIMARY KEY,
                locale VARCHAR(10) DEFAULT 'en',
                bio VARCHAR(2000) DEFAULT ''
            );
        )");

        W.exec(R"(
            CREATE TABLE IF NOT EXISTS user_favorites (
                id SERIAL PRIMARY KEY,
                user_id BIGINT REFERENCES users(user_id) ON DELETE CASCADE,
                media_id BIGINT,
                type VARCHAR(10),
                rating INT DEFAULT 0
            );
            CREATE UNIQUE INDEX IF NOT EXISTS fav_unique ON user_favorites (user_id, media_id);
        )");

        W.exec(R"(
            CREATE TABLE IF NOT EXISTS guild_settings (
                guild_id BIGINT PRIMARY KEY,
                allow_nsfw BOOLEAN DEFAULT true,
                theme_color BIGINT DEFAULT 15189194
            );
        )");

        // Add columns if they don't exist (for upgrading from old schema)
        try { W.exec("ALTER TABLE users ADD COLUMN bio VARCHAR(2000) DEFAULT '';"); } catch(...) {}
        try { W.exec("ALTER TABLE user_favorites ADD COLUMN rating INT DEFAULT 0;"); } catch(...) {}
        try { W.exec("ALTER TABLE guild_settings ADD COLUMN theme_color BIGINT DEFAULT 15189194;"); } catch(...) {}

        W.commit();
        std::cout << "Database initialized successfully.\n";
    } catch (const std::exception &e) {
        std::cerr << "Database initialization error: " << e.what() << std::endl;
    }
}

std::string Database::get_user_locale(dpp::snowflake user_id) {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (!conn || !conn->is_open()) return "en";
    try {
        pqxx::work W(*conn);
        pqxx::result R = W.exec_params("SELECT locale FROM users WHERE user_id = $1", (uint64_t)user_id);
        if (R.empty()) return "en";
        return R[0][0].as<std::string>();
    } catch (...) { return "en"; }
}

void Database::set_user_locale(dpp::snowflake user_id, const std::string& locale) {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (!conn || !conn->is_open()) return;
    try {
        pqxx::work W(*conn);
        W.exec_params(
            "INSERT INTO users (user_id, locale) VALUES ($1, $2) "
            "ON CONFLICT (user_id) DO UPDATE SET locale = $2", 
            (uint64_t)user_id, locale
        );
        W.commit();
    } catch (...) {}
}

std::string Database::get_user_bio(dpp::snowflake user_id) {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (!conn || !conn->is_open()) return "";
    try {
        pqxx::work W(*conn);
        pqxx::result R = W.exec_params("SELECT bio FROM users WHERE user_id = $1", (uint64_t)user_id);
        if (R.empty() || R[0][0].is_null()) return "";
        return R[0][0].as<std::string>();
    } catch (...) { return ""; }
}

void Database::set_user_bio(dpp::snowflake user_id, const std::string& bio) {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (!conn || !conn->is_open()) return;
    try {
        pqxx::work W(*conn);
        W.exec_params(
            "INSERT INTO users (user_id, bio) VALUES ($1, $2) "
            "ON CONFLICT (user_id) DO UPDATE SET bio = $2", 
            (uint64_t)user_id, bio
        );
        W.commit();
    } catch (...) {}
}

bool Database::get_guild_nsfw(dpp::snowflake guild_id) {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (!conn || !conn->is_open()) return true;
    try {
        pqxx::work W(*conn);
        pqxx::result R = W.exec_params("SELECT allow_nsfw FROM guild_settings WHERE guild_id = $1", (uint64_t)guild_id);
        if (R.empty()) return true; // Default true
        return R[0][0].as<bool>();
    } catch (...) { return true; }
}

void Database::set_guild_nsfw(dpp::snowflake guild_id, bool allow) {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (!conn || !conn->is_open()) return;
    try {
        pqxx::work W(*conn);
        W.exec_params(
            "INSERT INTO guild_settings (guild_id, allow_nsfw) VALUES ($1, $2) "
            "ON CONFLICT (guild_id) DO UPDATE SET allow_nsfw = $2", 
            (uint64_t)guild_id, allow
        );
        W.commit();
    } catch (...) {}
}

uint32_t Database::get_guild_theme(dpp::snowflake guild_id) {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (!conn || !conn->is_open()) return 15189194;
    try {
        pqxx::work W(*conn);
        pqxx::result R = W.exec_params("SELECT theme_color FROM guild_settings WHERE guild_id = $1", (uint64_t)guild_id);
        if (R.empty() || R[0][0].is_null()) return 15189194;
        return R[0][0].as<uint32_t>();
    } catch (...) { return 15189194; }
}

void Database::set_guild_theme(dpp::snowflake guild_id, uint32_t color) {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (!conn || !conn->is_open()) return;
    try {
        pqxx::work W(*conn);
        W.exec_params(
            "INSERT INTO guild_settings (guild_id, theme_color) VALUES ($1, $2) "
            "ON CONFLICT (guild_id) DO UPDATE SET theme_color = $2", 
            (uint64_t)guild_id, (uint64_t)color
        );
        W.commit();
    } catch (...) {}
}

void Database::delete_guild(dpp::snowflake guild_id) {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (!conn || !conn->is_open()) return;
    try {
        pqxx::work W(*conn);
        W.exec_params("DELETE FROM guild_settings WHERE guild_id = $1", (uint64_t)guild_id);
        W.commit();
    } catch (...) {}
}

bool Database::add_favorite(dpp::snowflake user_id, uint64_t media_id, const std::string& type, int rating) {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (!conn || !conn->is_open()) return false;
    try {
        pqxx::work W(*conn);
        W.exec_params("INSERT INTO users (user_id) VALUES ($1) ON CONFLICT DO NOTHING", (uint64_t)user_id);
        W.exec_params("INSERT INTO user_favorites (user_id, media_id, type, rating) VALUES ($1, $2, $3, $4) "
                      "ON CONFLICT (user_id, media_id) DO UPDATE SET rating = $4", 
            (uint64_t)user_id, media_id, type, rating);
        W.commit();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "DB Error add_favorite: " << e.what() << std::endl;
        return false;
    }
}

std::vector<UserFavorite> Database::get_favorites(dpp::snowflake user_id) {
    std::lock_guard<std::mutex> lock(db_mutex);
    std::vector<UserFavorite> favs;
    if (!conn || !conn->is_open()) return favs;
    try {
        pqxx::work W(*conn);
        pqxx::result R = W.exec_params("SELECT media_id, type, rating FROM user_favorites WHERE user_id = $1", (uint64_t)user_id);
        for (auto row : R) {
            favs.push_back({row[0].as<uint64_t>(), row[1].as<std::string>(), row[2].is_null() ? 0 : row[2].as<int>()});
        }
    } catch (const std::exception& e) {
        std::cerr << "DB Error get_favorites: " << e.what() << std::endl;
    }
    return favs;
}

void Database::delete_user_data(dpp::snowflake user_id) {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (!conn || !conn->is_open()) return;
    try {
        pqxx::work W(*conn);
        W.exec_params("DELETE FROM users WHERE user_id = $1", (uint64_t)user_id);
        W.commit();
    } catch (...) {}
}
