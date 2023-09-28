#ifndef __GM_LIB__
#define __GM_LIB__

#include <condition_variable>
#include <mutex>
#include <queue>
#include <chrono>
#include <memory>
#include <thread>
#include <atomic>
#include <deque>
#include <iostream>
#include <fstream>
#include <string>
#include <regex>

#include "easywsclient/easywsclient.hpp"
#include "json/json.hpp"

#define SCALE 50
#define BUFF_SIZE (2 * SCALE)

// namespace thread safe queue
namespace tsq
{
/**
 * @brief Interface class for a thread safe queue
 */
template <typename T>
class IThreadSafeQueue
{
   public:
    /**
     * @brief Pure virtual method for puting an element into the queue
     *
     * @param element The element to be inserted into the queue.
     */
    virtual void put(T &&element) = 0;

    /**
     * @brief Pure virtual method for puting an element into the queue with priority
     *
     * @param element The element to be inserted into the queue.
     */
    virtual void put_prioritized(T &&element) = 0;

    /**
     * @brief Pure virtual method for puting an element into the queue
     *
     * @param element A constant reference to the element to be inserted into the queue.
     */
    virtual void put(T const &element) = 0;

    /**
     * @brief Pure virtual method for popping an element from the queue in a "wait indefinitely
     * until new element is available" fashion
     *
     * @return Shared pointer to the element popped.
     */
    virtual std::shared_ptr<T> wait_and_pop() = 0;

    /**
     * @brief Pure virtual method for popping an element from the queue in a "wait for a timeout
     * until new element is available" fashion
     *
     * @param timeout Timeout in milliseconds to wait for new element
     * @note In the event of a timeout, a nullptr will be returned
     */
    virtual std::shared_ptr<T> wait_and_pop_for(const std::chrono::milliseconds &timeout) = 0;

    /**
     * @brief Pure virtual method for popping an element from the queue in a "wait indefinitely
     * until new element is available" fashion.
     *
     * @param element Reference to the popped element
     *
     * @return boolean that is true in case of a sucessful pop nad false in case of empty queue
     */
    virtual bool wait_and_pop(T &element) = 0;

    /**
     * @brief Pure virtual method for checking whether the queue is empty
     *
     * @return boolean that is true in case of the queue being empty
     */
    virtual bool empty() = 0;

    /**
     * @brief Pure virtual method for re-creating the underlying data structure (throwing away all
     * current elements)
     */
    virtual void reset() = 0;

    /**
     * @brief Pure virtual method for clearing the underlying data structure
     */
    virtual void clear() = 0;
};

/**
 * @brief Implements the interface defined in class IThreadSafeQueue. In this
 * case, the underlying data structure is a std::dequeue<T>
 * @see IThreadSafeQueue<T>
 */
template <typename T>
class ThreadSafeQueue : public IThreadSafeQueue<T>
{
   public:
    /**
     * @brief Construct a new Thread Safe Queue object
     *
     */
    ThreadSafeQueue()
    {
    }

    /**
     * @brief Overrides the method void put(T &&element) of the interface class
     */
    virtual void put(T &&element) override

    {
        {
            std::scoped_lock<std::mutex> lock(m_mutex);
            m_queue.push_back(std::forward<T>(element));
        }
        m_cv.notify_all();
    }

    /**
     * @brief Overrides the method void put_prioritized(T &&element) of the interface class
     */
    virtual void put_prioritized(T &&element) override

    {
        {
            std::scoped_lock<std::mutex> lock(m_mutex);
            m_queue.push_front(std::forward<T>(element));
        }
        m_cv.notify_all();
    }

    /**
     * @brief Overrides the method void put(T const &element) of the interface class
     */
    virtual void put(T const &element) override

    {
        {
            std::scoped_lock<std::mutex> lock(m_mutex);
            m_queue.push_back(element);
        }
        m_cv.notify_all();
    }

    /**
     * @brief Overrides the method std::shared_ptr<T> wait_and_pop() of the interface class
     */
    virtual std::shared_ptr<T> wait_and_pop() override

    {
        std::shared_ptr<T>           result;
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [&]() { return !m_queue.empty(); });
        result = std::make_shared<T>(std::move(m_queue.front()));
        m_queue.pop_front();
        lock.unlock();
        return result;
    }

    /**
     * @brief Overrides the method std::shared_ptr<T> wait_and_pop_for(const
     * std::chrono::milliseconds &timeout) of the interface class
     */
    virtual std::shared_ptr<T> wait_and_pop_for(const std::chrono::milliseconds &timeout) override

    {
        std::shared_ptr<T>           result;
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_cv.wait_for(lock, timeout, [&]() { return !m_queue.empty(); }))
        {
            result = std::make_shared<T>(std::move(m_queue.front()));
            m_queue.pop_front();
            lock.unlock();
        }
        return result;
    }

    /**
     * @brief Overrides the method bool wait_and_pop(T &element) of the interface class
     */
    virtual bool wait_and_pop(T &element) override

    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [&]() { return !m_queue.empty(); });
        element = m_queue.front();
        m_queue.pop_front();
        lock.unlock();
        return true;
    }

    /**
     * @brief Overrides the method bool empty() of the interface class
     */
    virtual bool empty() override

    {
        bool result;
        {
            std::scoped_lock<std::mutex> lock(m_mutex);
            result = m_queue.empty();
        }
        return result;
    }

    /**
     * @brief Overrides the method void reset() of the interface class
     */
    virtual void reset() override

    {
        m_queue = std::deque<T>{};
    }

    /**
     * @brief Overrides the method void clear() of the interface class
     */
    virtual void clear() override

    {
        m_queue.clear();
    }

   private:
    std::deque<T>           m_queue{};
    std::condition_variable m_cv;
    std::mutex              m_mutex;
};
}  // namespace tsq

// namespace websocket
namespace ws
{
/**
 * @brief Implementation of a websocket. Receives information from the webserver in it's own thread.
 * Queues all the received data and allow other threads to consume it in a producer-consumer manner,
 * where this class would be the producer of data.
 *
 * @note easywsclient isn't thread safe by itself, so this is a thread-safe wrapper
 */
class WebSocket
{
   public:
    /**
     * @brief Constructs a new WebSocket object.
     *
     * This constructor initializes the WebSocket object based on the given url
     * @param url The websocket url
     */
    WebSocket(std::string url) : m_ewsc{easywsclient::WebSocket::from_url(url)}
    {
    }

    /**
     * @brief Destroys the WebSocket object.
     *
     * This destructor closes the connection and stops the thread used by the WebSocket object.
     */
    ~WebSocket()
    {
        close();
    }

    /**
     *  @brief Sends a string over the WebSocket connection.
     *  This method enqueues the given string to the outgoing queue
     *  for transmission over the WebSocket connection.
     *  @param s The string to be sent.
     */
    void send(std::string const &s);
    /**
     *  @brief Receives a string from the WebSocket connection.
     *  This method waits for a string to be available in the incoming queue
     *  and retrieves it by reference.
     *  @param s A reference to store the received string.
     *  @return True if a string was successfully received, false otherwise.
     */
    bool recv(std::string &s);
    /**
     *  @brief Starts the WebSocket connection.
     *  This method sets the running flag to true and starts a separate thread
     *  to run the WebSocket connection in the background.
     */
    void start();

    /**
     * @brief Closes the WebSocket connection.
     *
     * This method closes the WebSocket connection by setting the running flag to false.
     * It also puts a predefined closing message in the incoming queue to signal the closure.
     * If the WebSocket is running in a separate thread, it will join that thread to ensure
     * proper termination.
     */
    void close();

   private:
    void run();

   private:
    std::shared_ptr<easywsclient::WebSocket> m_ewsc;
    tsq::ThreadSafeQueue<std::string>        m_outgoing_q;
    tsq::ThreadSafeQueue<std::string>        m_incoming_q;
    std::thread                              m_thread;
    bool                                     m_running{false};
};

}  // namespace ws

// namespace actuator
namespace act
{
/**
 * @brief Consumes information comming from a WebSocket. Saves data to a file and draw position of
 * the actuator to the screen. In a producer-consumer manner, this class defines a thread that would
 * be the consumer of data.
 */
class Actuator
{
   public:
    /**
     * @brief Constructs a new Actuator object.
     *
     * This constructor initializes the Actuator object based on the given url and file name
     * @param url The websocket url
     * @param file_name A reference to the file name to save the data
     */
    Actuator(const std::string &url, const std::string &file_name)
        : m_ws{std::make_shared<ws::WebSocket>(url)}, m_drawing_buffer(BUFF_SIZE, '_')
    {
        m_file.open(file_name, std::ios_base::app);
    }

    /**
     * @brief Destroys the Actuator object.
     *
     * This destructor calls the 'stop' method and destroys the Actuator object
     */
    ~Actuator()
    {
        stop();
        m_file.close();
    }

    /**
     *  @brief Starts the Actuator thread.
     *  This method sets the running flag to true and starts a separate thread
     *  to run the Actuator that consumes the websocket data
     */
    void start();

    /**
     *  @brief Stops the Actuator thread.
     *  This method closes the WebSocket, sets the running flag to false. If the Actuator is
     * running in a separate thread, it will join that thread to ensure proper termination.
     */
    void stop();

   private:
    void run();
    void draw(float &val);

   private:
    std::shared_ptr<ws::WebSocket> m_ws;
    std::string                    m_drawing_buffer;
    std::ofstream                  m_file;
    std::thread                    m_thread;
    bool                           m_running{false};
};
}  // namespace act

#endif