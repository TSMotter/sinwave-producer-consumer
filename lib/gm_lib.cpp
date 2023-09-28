#include "gm_lib.hpp"

namespace ws
{
void WebSocket::send(std::string const &s)
{
    m_outgoing_q.put(std::string{s});
}

bool WebSocket::recv(std::string &s)
{
    return m_incoming_q.wait_and_pop(s);
}

void WebSocket::start()
{
    m_running = true;
    m_thread  = std::thread(&WebSocket::run, this);
}

void WebSocket::close()
{
    if (!m_running)
    {
        return;
    }

    m_running = false;

    m_incoming_q.put(std::string{
        "[{\"channel\":\"signaling_websocket_close\",\"frequency\":0,\"value\":0,\"time\":0}]"});

    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

void WebSocket::run()
{
    while (m_running)
    {
        if (m_ewsc->getReadyState() == easywsclient::WebSocket::CLOSED)
        {
            std::cout << "Websocket closed connection" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            break;
        }
        /* Could use a negative timeout as a "block until new message arrives"
        but decided to use a positive timeout instead so that system does not hang for
        ever if websocket stops pusblishing new data */
        m_ewsc->poll(10);
        m_ewsc->dispatch([&](const std::string &message)
                         { m_incoming_q.put(std::string{message}); });
    }
    m_ewsc->close();
    m_ewsc->poll();
}
}  // namespace ws

namespace act
{
void Actuator::run()
{
    std::string raw_sample;
    while (m_running)
    {
        if (m_ws->recv(raw_sample))
        {
            try
            {
                nlohmann::json sample_as_json = nlohmann::json::parse(raw_sample);
                for (auto &object : sample_as_json)
                {
                    // Ignore sample if doesn't pass basic requirement
                    if (object["value"] == nullptr)
                    {
                        continue;
                    }

                    // Draw on the console
                    float value = object["value"];
                    draw(value);

                    // Save each json 'object' (content within 1 pair of curly brackets)
                    m_file << object << std::endl;
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error while processing one sample - " << e.what() << std::endl;
                continue;
            }
        }
    }
}

void Actuator::start()
{
    m_running = true;

    // start consumer
    m_thread = std::thread(&Actuator::run, this);

    // start producer
    m_ws->start();
}

void Actuator::stop()
{
    if (!m_running)
    {
        return;
    }

    // Stop producer
    m_running = false;
    m_ws->close();

    // Stop consumer
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

void Actuator::draw(float &val)
{
    int pos = static_cast<int>((val * SCALE) + SCALE);
    std::fill(m_drawing_buffer.begin(), m_drawing_buffer.end(), '_');
    m_drawing_buffer[pos] = 'O';
    std::cout << "\r" << m_drawing_buffer << std::flush;
    // std::cout << m_drawing_buffer << std::endl;
}

}  // namespace act