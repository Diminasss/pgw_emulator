#include "pgw_sessions.h"
#include <gtest/gtest.h>

TEST(SessionManagerTest, CreateSession) {
    SessionManager sm(60); // таймаут 60 секунд
    EXPECT_EQ(sm.create_session("12345", "127.0.0.1", 1234), true);
    EXPECT_TRUE(sm.session_exists("12345"));
    EXPECT_EQ(sm.get_active_sessions_count(), 1);
}

TEST(SessionManagerTest, DuplicateSession) {
    SessionManager sm(60);
    EXPECT_TRUE(sm.create_session("12345", "127.0.0.1", 1234));
    // Повторная попытка создания такой же сессии должна вернуть false
    EXPECT_FALSE(sm.create_session("12345", "127.0.0.1", 1234));
    EXPECT_EQ(sm.get_active_sessions_count(), 1);
}

TEST(SessionManagerTest, RemoveSessionsBatch) {
    SessionManager sm(60);
    // Создаем 3 разные сессии
    sm.create_session("A", "127.0.0.1", 1000);
    sm.create_session("B", "127.0.0.1", 1001);
    sm.create_session("C", "127.0.0.1", 1002);
    EXPECT_EQ(sm.get_active_sessions_count(), 3);
    // Удаляем до 2 сессий батчем
    size_t removed = sm.remove_sessions_batch(2);
    EXPECT_EQ(removed, 2);
    EXPECT_EQ(sm.get_active_sessions_count(), 1);
}

TEST(SessionManagerTest, DeleteSession) {
    SessionManager sm(60);
    sm.create_session("X", "127.0.0.1", 5000);
    EXPECT_TRUE(sm.session_exists("X"));
    sm.delete_session("X");
    EXPECT_FALSE(sm.session_exists("X"));
    EXPECT_EQ(sm.get_active_sessions_count(), 0);
}

