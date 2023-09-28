#include <gtest/gtest.h>
#include <thread>
#include <vector>

#include "gm_lib.hpp"

// Fixture definition
class ThreadSafeQueueFixture : public ::testing::Test
{
   protected:
    ThreadSafeQueueFixture() : m_queue{std::make_shared<tsq::ThreadSafeQueue<std::string>>()}
    {
        // You can do set-up work for each test here.
    }

    ~ThreadSafeQueueFixture()
    {
        // You can do clean-up work that doesn't throw exceptions here.
        m_queue->clear();
    }

    testing::AssertionResult assertStdStrings(std::string ref)
    {
        if (ref == m_test_string1 || ref == m_test_string2)
        {
            return testing::AssertionSuccess();
        }
        else
        {
            /* clang-format off */
            return testing::AssertionFailure()
                   << "-> The popped value:\n\""
                   << ref << "\"\n"
                   << "-> Does not match any of the possible expected values, which are:\n\""
                   << m_test_string1 << "\"\n"
                   << "-> And this:\n\""
                   << m_test_string2 << "\"\n";
            /* clang-format on */
        }
    }

    static constexpr const char* m_test_string1 =
        "Deep into that darkness peering;Long I stood there, wondering, "
        "fearing;Doubting, dreaming dreams no mortals;";
    static constexpr const char* m_test_string2 =
        "Ever dared to dream before;But the silence was unbroken;And the "
        "stillness gave no token;";
    std::shared_ptr<tsq::IThreadSafeQueue<std::string>> m_queue;
};

TEST_F(ThreadSafeQueueFixture, TestPoppingWholeString)
{
    m_queue->put(std::move(m_test_string1));
    m_queue->put(std::move(m_test_string2));

    auto runner = [this](std::shared_ptr<tsq::IThreadSafeQueue<std::string>> q)
    {
        std::shared_ptr<std::string> popped_element;
        popped_element = q->wait_and_pop_for(std::chrono::milliseconds{100});
        if (popped_element)
            ASSERT_TRUE(assertStdStrings(*popped_element));
    };

    std::vector<std::thread> threads;
    threads.emplace_back(runner, m_queue);
    threads.emplace_back(runner, m_queue);
    threads.emplace_back(runner, m_queue);
    threads.emplace_back(runner, m_queue);
    for (auto& thread : threads)
        thread.join();
}