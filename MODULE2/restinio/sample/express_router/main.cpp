#include <fmt/format.h>
#include <iostream>
#include <json_dto/pub.hpp>
#include <restinio/all.hpp>
#include <restinio/websocket/websocket.hpp>

namespace rr   = restinio::router;
using router_t = rr::express_router_t<>;

namespace rws       = restinio::websocket::basic;
using ws_registry_t = std::map<std::uint64_t, rws::ws_handle_t>;

using traits_t =
    restinio::traits_t<restinio::asio_timer_manager_t,
                       restinio::single_threaded_ostream_logger_t, router_t>;

struct weatherstation_t
{
  weatherstation_t() = default;

  weatherstation_t(std::string ID, std::string Date, std::string Time,
                   std::string Name, std::string Lat, std::string Lon,
                   std::string Temperature, std::string Humidity)
      : m_ID{std::move(ID)},
        m_Date{std::move(Date)},
        m_Time{std::move(Time)},
        m_Name{std::move(Name)},
        m_Lat{std::move(Lat)},
        m_Lon{std::move(Lon)},
        m_Temperature{std::move(Temperature)},
        m_Humidity{std::move(Humidity)}
  {
  }

  template <typename JSON_IO> void json_io(JSON_IO &io)
  {
    io &json_dto::mandatory("ID", m_ID) & json_dto::mandatory("Date", m_Date) &
        json_dto::mandatory("Time", m_Time) &
        json_dto::mandatory("Name", m_Name) &
        json_dto::mandatory("Lat", m_Lat) & json_dto::mandatory("Lon", m_Lon) &
        json_dto::mandatory("Temperature", m_Temperature) &
        json_dto::mandatory("Humidity", m_Humidity);
  }

  std::string m_ID;
  std::string m_Date;
  std::string m_Time;
  std::string m_Name;
  std::string m_Lat;
  std::string m_Lon;
  std::string m_Temperature;
  std::string m_Humidity;
};

using weather_collection_t = std::vector<weatherstation_t>;

class weather_handler_t
{
public:
  explicit weather_handler_t(weather_collection_t &weathers)
      : m_weather(weathers)
  {
  }

  weather_handler_t(const weather_handler_t &) = delete;
  weather_handler_t(weather_handler_t &&)      = delete;

  auto on_weather_list(const restinio::request_handle_t &req,
                       rr::route_params_t) const
  {
    auto resp = init_resp(req->create_response());

    resp.set_body(json_dto::to_json(m_weather));

    return resp.done();
  }

  auto on_date_get(const restinio::request_handle_t &req,
                   rr::route_params_t                params)
  {
    short      answer = 0;
    const auto Date =
        restinio::utils::unescape_percent_encoding(params["Date"]);

    auto resp = init_resp(req->create_response());

    for (std::size_t i = 0; i < m_weather.size(); ++i)
    {
      const auto &b = m_weather[i];
      if (Date == b.m_Date)
      {
        resp.append_body(json_dto::to_json(b));
        answer = 1;
      }
    }

    // No weather with that date
    if (answer == 0) resp.set_body("No weather with that date " + Date + "\n");

    return resp.done();
  }

  auto on_weather_latest(const restinio::request_handle_t &req,
                         rr::route_params_t                params)
  {
    auto resp = init_resp(req->create_response());
    try
    {
      resp.set_body("3 latest entries:\n");

      for (std::size_t i = 0; i < m_weather.size(); ++i)
      {
        const auto &b = m_weather[i];
        if (i >= m_weather.size() - 3)
        {
          resp.append_body(json_dto::to_json(b));
        }
      }
    }
    catch (const std::exception &)
    {
      mark_as_bad_request(resp);
    }

    return resp.done();
  }

  auto on_post_data(const restinio::request_handle_t &req, rr::route_params_t)
  {
    auto resp = init_resp(req->create_response());

    try
    {
      m_weather.emplace_back(
          json_dto::from_json<weatherstation_t>(req->body()));
    }
    catch (const std::exception & /*ex*/)
    {
      mark_as_bad_request(resp);
    }

    return resp.done();
  }

  auto on_weather_update(const restinio::request_handle_t &req,
                         rr::route_params_t                params)
  {
    const auto ID = restinio::cast_to<std::uint32_t>(params["ID"]);

    auto resp = init_resp(req->create_response());

    try
    {
      auto b = json_dto::from_json<weatherstation_t>(req->body());
      /*
      if (0 != ID && ID <= m_weather.size())
      {
        m_weather[ID - 1] = b;
      }
      */
      short answer = 0;
      for (std::size_t i = 0; i < m_weather.size(); i++)
      {
        const auto &c = m_weather[i];
        if (ID == restinio::cast_to<std::uint32_t>(c.m_ID))
        {
          m_weather[i] = b;
          answer = 1;
        }
      }

      if (answer == 0)
      {
        mark_as_bad_request(resp);
        resp.set_body("No weather with #" + std::to_string(ID) + "\n");
      }
    }
    catch (const std::exception & /*ex*/)
    {
      mark_as_bad_request(resp);
    }

    return resp.done();
  }

  auto on_live_update(const restinio::request_handle_t &req,
                      rr::route_params_t params) // Websocket handler
  {
    if (restinio::http_connection_header_t::upgrade ==
        req->header().connection())
    {
      auto wsh = rws::upgrade<traits_t>(
          *req, rws::activation_t::immediate,
          [this](auto wsh, auto m)
          {
            if (rws::opcode_t::text_frame == m->opcode() ||
                rws::opcode_t::binary_frame == m->opcode() ||
                rws::opcode_t::continuation_frame == m->opcode())
            {
              wsh->send_message(*m);
            }
            else if (rws::opcode_t::ping_frame == m->opcode())
            {
              auto resp = *m;
              resp.set_opcode(rws::opcode_t::pong_frame);
              wsh->send_message(resp);
            }
            else if (rws::opcode_t::connection_close_frame == m->opcode())
            {
              m_registry.erase(wsh->connection_id());
            }
          });
      m_registry.emplace(wsh->connection_id(), wsh);
      init_resp(req->create_response()).done();
      return restinio::request_accepted();
    }
    return restinio::request_rejected();
  }

  auto on_weather_delete(const restinio::request_handle_t &req,
                         rr::route_params_t                params)
  {
    auto resp = init_resp(req->create_response());

    const auto ID = restinio::cast_to<std::uint32_t>(params["ID"]);

    try
    {
      for (std::size_t i = 0; i < m_weather.size(); i++)
      {
        const auto &c = m_weather[i];
        if (ID == restinio::cast_to<std::uint32_t>(c.m_ID))
        {
          m_weather.erase(m_weather.begin() + i);
        }
      }
    }
    catch (const std::exception & /*ex*/)
    {
      mark_as_bad_request(resp);
    }

    return resp.done();
  }

  auto options(restinio::request_handle_t req, restinio::router::route_params_t)
  {
    const auto methods = "OPTIONS, GET, POST, PATCH, DELETE, PUT";
    auto       resp    = init_resp(req->create_response());
    resp.append_header(restinio::http_field::access_control_allow_methods,
                       methods);
    resp.append_header(restinio::http_field::access_control_allow_headers,
                       "content-type");
    resp.append_header(restinio::http_field::access_control_max_age, "86400");
    return resp.done();
  }


private:
  weather_collection_t &m_weather;
  ws_registry_t         m_registry;

  template <typename RESP> static RESP init_resp(RESP resp)
  {
    resp.append_header("Server", "RESTinio sample server /v.0.6")
        .append_header_date_field()
        .append_header("Content-Type", "text/plain; charset=utf-8")
        .append_header(restinio::http_field::access_control_allow_origin, "*");

    return resp;
  }

  template <typename RESP> static void mark_as_bad_request(RESP &resp)
  {
    resp.header().status_line(restinio::status_bad_request());
  };
};
auto server_handler(weather_collection_t &weather_collection)
{
  auto router = std::make_unique<router_t>();
  auto handler =
      std::make_shared<weather_handler_t>(std::ref(weather_collection));

  auto by = [&](auto method)
  {
    using namespace std::placeholders;
    return std::bind(method, handler, _1, _2);
  };

  auto method_not_allowed = [](const auto &req, auto)
  {
    return req->create_response(restinio::status_method_not_allowed())
        .connection_close()
        .done();
  };

  // Handlers for '/' path.
  router->http_get("/", by(&weather_handler_t::on_weather_list));
  router->http_post("/", by(&weather_handler_t::on_post_data));
  router->add_handler(restinio::http_method_options(), "/",
                      by(&weather_handler_t::options));

  // Handler for'/three' path
  router->http_get("/three", by(&weather_handler_t::on_weather_latest));
  router->add_handler(restinio::http_method_options(), "/three",
                      by(&weather_handler_t::options));

  //
  router->http_get("/chat", by(&weather_handler_t::on_live_update));

  // Disable all other methods for '/'.
  router->add_handler(
      restinio::router::none_of_methods(restinio::http_method_get(),
                                        restinio::http_method_post(),
                                        restinio::http_method_options()),
      "/", method_not_allowed);

  // Handler for '/date/:date' path.
  router->http_get("/Date/:Date", by(&weather_handler_t::on_date_get));
  router->add_handler(restinio::http_method_options(), "/Date/:Date",
                      by(&weather_handler_t::options));

  // Handler for 'R"(/:id(\d+))' path.
  router->http_put(R"(/:ID(\d+))", by(&weather_handler_t::on_weather_update));
  router->add_handler(restinio::http_method_options(), R"(/:ID(\d+))",
                      by(&weather_handler_t::options));

  // Handler for delete by ID
  router->http_delete(R"(/:ID(\d+))",
                      by(&weather_handler_t::on_weather_delete));

  return router;
}

int main()
{
  using namespace std::chrono;

  try
  {
    using traits_t =
        restinio::traits_t<restinio::asio_timer_manager_t,
                           restinio::single_threaded_ostream_logger_t,
                           router_t>;

    weather_collection_t weather_collection{
        {"1", "20211105", "12:15", "Aarhus N", "13.692", "19.438", "13.1",
         "70%"},
        {"2", "20211105", "12:30", "Aarhus N", "13.692", "19.438", "13.2",
         "71%"},
        {"3", "20211105", "12:45", "Aarhus N", "13.692", "19.438", "13.3",
         "72%"},
        {"4", "20211105", "13:00", "Aarhus N", "13.692", "19.438", "13.4",
         "73%"},
        {"5", "20211105", "13:15", "Aarhus N", "13.692", "19.438", "13.5",
         "74%"}};

    restinio::run(restinio::on_this_thread<traits_t>()
                      .address("localhost")
                      .request_handler(server_handler(weather_collection))
                      .read_next_http_message_timelimit(10s)
                      .write_http_response_timelimit(1s)
                      .handle_request_timeout(1s));
  }
  catch (const std::exception &ex)
  {
    std::cerr << "Error: " << ex.what() << std::endl;
    return 1;
  }

  return 0;
}